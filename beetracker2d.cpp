#include "beetracker2d.h"
#include <iostream>

BeeTracker2d::BeeTracker2d()
{
    cv::cuda::setDevice(0);
    cv::cuda::printShortCudaDeviceInfo(cv::cuda::getDevice());

    m_gaussianFilter = cv::cuda::createGaussianFilter(CV_8UC1, CV_8UC1, cv::Size(5, 5), 32);
    m_gaussianFilterDOG1 = cv::cuda::createGaussianFilter(CV_8UC1, CV_8UC1, cv::Size(15, 15), 32);
    m_gaussianFilterDOG2 = cv::cuda::createGaussianFilter(CV_8UC1, CV_8UC1, cv::Size(31, 31), 32);

    m_scharrFilter = cv::cuda::createScharrFilter(CV_8UC1, CV_8UC1, 0, 1);
    cv::Mat element = getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    //m_morphologyFilter = cv::cuda::createMorphologyFilter(cv::MORPH_OPEN, CV_8UC1, element, cv::Point(-1,-1),1);
    m_morphologyFilter = cv::cuda::createMorphologyFilter(cv::MORPH_DILATE, CV_8UC1, element, cv::Point(-1,-1),1);

    m_blobParams.minDistBetweenBlobs = 1.0f;
    m_blobParams.filterByInertia = false;
    m_blobParams.filterByConvexity = false;
    m_blobParams.filterByColor = false;
    m_blobParams.filterByCircularity = false;

    m_blobParams.filterByArea = true;
    m_blobParams.minArea = 100.0f;
    m_blobParams.maxArea = 100000.0f;
    //params.minInertiaRatio = 0.05;
    //params.minThreshold = 0;
    //params.maxThreshold = 130;
    //params.minConvexity = 0.3;
    //params.minCircularity = 0.1;
}

void BeeTracker2d::load(std::string filename, bool flipFlag)
{
    m_flipFlag = flipFlag;
    m_cam.open(filename, CV_CAP_FFMPEG);
    setMaxFrame(m_cam.get(CV_CAP_PROP_FRAME_COUNT)-2);
}

void BeeTracker2d::getFrame(int frameIndex, std::string mode)
{
    if (m_previousFrame+1 != frameIndex)
    {
        m_cam.set(CV_CAP_PROP_POS_FRAMES, frameIndex);
    }
    m_previousFrame = frameIndex;
    bool ret = m_cam.read(m_cpuFrame);


    while (!ret)
    {
        m_cam.set(CV_CAP_PROP_POS_FRAMES, ++frameIndex);
        ret = m_cam.read(m_cpuFrame);
        if (!ret)
        {
            std::cout << "Failed to load frame on second attempt" << std::endl;

        }
    }

    cv::transpose(m_cpuFrame, m_cpuFrame);
    if (m_flipFlag)
    {
        cv::flip(m_cpuFrame, m_cpuFrame, -1);
    }

    m_frameUnprocessed.upload(m_cpuFrame);


    if (mode == "Raw")
    {
        cv::cuda::cvtColor(m_frameUnprocessed, m_frameUnprocessed, CV_BGR2RGB);
        m_frameUnprocessed.download(m_cpuFrame);
        return;
    }

    if (mode == "SIFT")
    {
        cv::Mat gray;
        cv::cvtColor(m_cpuFrame, gray, CV_BGR2GRAY);
        int minHessian = 400;

        //cv::xfeatures2d::SiftFeatureDetector detector;
        //std::vector<cv::KeyPoint> keypoints;
        //detector.detect( gray, keypoints);
        //cv::xfeatures2d::SurfFeatureDetector detector(100);
        //std::vector<cv::KeyPoint> keypoints;

        //feature_detector->detect(gray, keypoints);
        //cv::drawKeypoints( m_cpuFrame, keypoints, m_cpuFrame, cv::Scalar::all(-1), cv::DrawMatchesFlags::DEFAULT );
        return;
    }

    m_cpuFrame = roiMask(m_cpuFrame, 140);
    m_frameUnprocessed.upload(m_cpuFrame);

    cv::cuda::cvtColor(m_frameUnprocessed, m_frameUnprocessed, CV_BGR2Lab);
    m_frameProcessed = processFrame(m_frameUnprocessed, mode);


    if (mode == "Raw + Identifiers")
    {
        std::vector<cv::cuda::GpuMat> channelsLab;
        cv::cuda::split(m_frameProcessed, channelsLab);
        channelsLab[0].download(m_binaryFrame);
        std::cout << "Processing raw + identifiers" << std::endl;
        cv::cvtColor(m_cpuFrame, m_cpuFrame, CV_BGR2RGB);




        cv::Ptr<cv::SimpleBlobDetector> blob_detector = cv::SimpleBlobDetector::create(m_blobParams);
        std::vector<cv::KeyPoint> keypoints;

        blob_detector->detect(m_binaryFrame, keypoints);
        for (auto blob : keypoints)
        {
            cv::circle(m_cpuFrame, blob.pt, 5, cv::Scalar(0, 255, 0), -1);
        }
    }
    else
    {
        cv::cuda::cvtColor(m_frameProcessed, m_frameProcessed, CV_Lab2RGB);
        m_frameProcessed.download(m_cpuFrame);
    }
}

void BeeTracker2d::setParameters(int window1, int window2, int minimumArea, int maximumArea)
{
    m_gaussianFilterDOG1 = cv::cuda::createGaussianFilter(CV_8UC1, CV_8UC1, cv::Size(window1, window1), 32);
    m_gaussianFilterDOG2 = cv::cuda::createGaussianFilter(CV_8UC1, CV_8UC1, cv::Size(window2, window2), 32);
    m_blobParams.minArea = minimumArea;
    m_blobParams.maxArea = maximumArea;
}


cv::cuda::GpuMat BeeTracker2d::processFrame(cv::cuda::GpuMat labFrame, std::string mode)
{

    std::vector<cv::cuda::GpuMat> channelsLab(3);
    cv::cuda::split(labFrame, channelsLab);
    for (int i = 0; i<3; i++)
    {
        m_gaussianFilter->apply(channelsLab[i], channelsLab[i]);
    }
    cv::cuda::merge(channelsLab, labFrame);
    labFrame = colorFilter(labFrame);
    //labFrame = simpleColorFilter(labFrame);
    if (mode == "ColorFiltered")
        {
            return labFrame;
        }



    cv::cuda::split(labFrame, channelsLab);
    cv::cuda::GpuMat dog1;
    cv::cuda::GpuMat dog2;

    m_gaussianFilterDOG1->apply(channelsLab[0], dog1);
    m_gaussianFilterDOG2->apply(channelsLab[0], dog2);
    cv::cuda::subtract(dog2,dog1, channelsLab[0]);
    /*
    m_gaussianFilterDOG1->apply(channelsLab[1], dog1);
    m_gaussianFilterDOG2->apply(channelsLab[1], dog2);
    cv::cuda::subtract(dog2,dog1, channelsLab[1]);
    m_scharrFilter->apply(channelsLab[0], channelsLab[0]);
    m_gaussianFilterDOG1->apply(channelsLab[2], dog1);
    m_gaussianFilterDOG2->apply(channelsLab[2], dog2);
    cv::cuda::subtract(dog2,dog1, channelsLab[2]);
    */
    cv::cuda::multiply(channelsLab[0], 10, channelsLab[0]);
    cv::cuda::merge(channelsLab, labFrame);

    if (mode == "DOG")
        return labFrame;

    cv::cuda::threshold(channelsLab[0], channelsLab[0], m_threshold, 255, cv::THRESH_BINARY_INV);
    //m_morphologyFilter->apply(channelsLab[0], channelsLab[0]);
    cv::cuda::merge(channelsLab, labFrame);
    return labFrame;
}



cv::cuda::GpuMat BeeTracker2d::colorFilter(cv::cuda::GpuMat labMat)
{
    std::vector<cv::cuda::GpuMat> channelsLab;
    cv::cuda::split(labMat, channelsLab);

    cv::cuda::GpuMat maskBlueA,maskBlueB, maskYellow, maskBlue, maskGreen, maskFinal;
    cv::cuda::GpuMat maskBlueB2;
    cv::cuda::GpuMat maskPurple;

    cv::cuda::threshold(channelsLab[2], maskBlueB2, 123, 255, cv::THRESH_BINARY_INV);
    cv::cuda::threshold(channelsLab[1], maskGreen, 120, 255, cv::THRESH_BINARY_INV);
    cv::cuda::threshold(channelsLab[1], maskBlueA, 125, 255, cv::THRESH_BINARY_INV);
    cv::cuda::threshold(channelsLab[2], maskBlueB, 125, 255, cv::THRESH_BINARY_INV);
    cv::cuda::threshold(channelsLab[2], maskYellow, 160, 255, cv::THRESH_BINARY);
    cv::cuda::threshold(channelsLab[1], maskPurple, 135, 255, cv::THRESH_BINARY);

    cv::cuda::bitwise_and(maskBlueA, maskBlueB, maskBlue);
    cv::cuda::bitwise_or(maskBlue, maskBlueB2, maskBlue);
    //cv::cuda::bitwise_or(maskBlue, maskYellow, maskFinal);
    //cv::cuda::bitwise_or(maskGreen, maskFinal, maskFinal);
    //cv::cuda::bitwise_or(maskFinal, maskPurple, maskFinal);

    m_morphologyFilter->apply(maskBlue, maskBlue);

    cv::cuda::add(channelsLab[0], 50, channelsLab[0], maskBlue);


    cv::cuda::merge(channelsLab, labMat);
    return labMat;
}

cv::cuda::GpuMat BeeTracker2d::simpleColorFilter(cv::cuda::GpuMat labFrame)
{
    std::vector<cv::cuda::GpuMat> channelsLab;
    cv::cuda::split(labFrame, channelsLab);

    //cv::cuda::subtract(channelsLab[0], 127, channelsLab[0]);
    cv::cuda::subtract(channelsLab[1], 127, channelsLab[1]);
    cv::cuda::subtract(channelsLab[2], 127, channelsLab[2]);
    cv::cuda::sqr(channelsLab[1], channelsLab[1]);
    cv::cuda::sqr(channelsLab[2], channelsLab[2]);

    cv::cuda::GpuMat sqrValue;

    cv::cuda::add(channelsLab[1], channelsLab[2], sqrValue);

    cv::cuda::GpuMat mask;

    cv::cuda::threshold(sqrValue, mask, 5, 255, cv::THRESH_BINARY);

    cv::cuda::split(labFrame, channelsLab);

    cv::cuda::add(channelsLab[0], 30, channelsLab[0], mask);

    cv::cuda::merge(channelsLab, labFrame);
    return labFrame;
}


cv::Mat BeeTracker2d::roiMask(cv::Mat input, int threshold)
{
    cv::Mat gray;
    cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    cv::threshold(gray, gray, threshold, 255, cv::THRESH_BINARY);

    std::vector<std::vector<cv::Point> > contours;
    cv::findContours( gray, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE );

    cv::Mat drawing = cv::Mat::zeros(gray.size(), CV_8UC3);

    int iLargest = 0;
    double largestContour = 0;
    int counter = 0;
    for (auto contour : contours)
    {
        double contourSize = cv::contourArea(contour);
        if (contourSize > largestContour)
        {
            largestContour = contourSize;
            iLargest = counter;
        }
        counter ++;
    }

    std::vector<cv::Point> hull;

    cv::approxPolyDP(cv::Mat(contours[iLargest]), contours[iLargest], 30, true);
    cv::Scalar color = cv::Scalar( 255,255,255);

    cv::Mat mask = cv::Mat::zeros(gray.size(), CV_8U);
    cv::drawContours( mask, contours, iLargest, 255, -1 );
    cv::Mat not_mask;
    cv::bitwise_not(mask, not_mask);
    input.setTo(140*cv::Scalar(1, 1, 1), not_mask);
    return input;
}

int BeeTracker2d::getMaxFrame() const
{
    return m_maxFrame;
}

void BeeTracker2d::setMaxFrame(int maxFrame)
{
    m_maxFrame = maxFrame;
}

int BeeTracker2d::getThreshold() const
{
    return m_threshold;
}

void BeeTracker2d::setThreshold(int threshold)
{
    m_threshold = threshold;
}
