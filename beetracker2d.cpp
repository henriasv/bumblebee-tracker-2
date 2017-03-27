#include "beetracker2d.h"
#include <iostream>
BeeTracker2d::BeeTracker2d()
{
    cv::cuda::setDevice(0);
    cv::cuda::printShortCudaDeviceInfo(cv::cuda::getDevice());

    m_gaussianFilter = cv::cuda::createGaussianFilter(CV_8UC1, CV_8UC1, cv::Size(31, 31), 32);
    cv::Mat element = getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));
    m_morphologyFilter = cv::cuda::createMorphologyFilter(cv::MORPH_OPEN, CV_8UC1, element, cv::Point(-1,-1),1);
}

void BeeTracker2d::load(std::string filename, bool flipFlag)
{
    m_flipFlag = flipFlag;
    m_cam.open(filename);
}

cv::Mat BeeTracker2d::getFrame(int frameIndex, std::string mode)
{
    m_cam.set(CV_CAP_PROP_POS_FRAMES, frameIndex);
    m_cam.read(m_cpuFrame);
    if (m_flipFlag)
    {
        cv::flip(m_cpuFrame, m_cpuFrame, -1);
    }
    m_cpuFrame.copyTo(m_rawFrame);
    m_cpuFrame = roiMask(m_cpuFrame, 140);
    m_frameUnprocessed.upload(m_cpuFrame);

    cv::cuda::cvtColor(m_frameUnprocessed, m_frameUnprocessed, CV_BGR2Lab);

    m_frameColorfiltered = colorFilter(m_frameUnprocessed);
    m_frameProcessed = processFrame(m_frameColorfiltered);
    //cv::Mat frame2;
    m_frameProcessed = m_frameUnprocessed;
    cv::cuda::cvtColor(m_frameProcessed, m_frameProcessed, CV_Lab2RGB);
    //cv::cvtColor(frame1, frame2, CV_BGR2RGB);
    //m_frameProcessed.download(m_cpuFrame);


    cv::Mat retFrame;
    if (mode == "Raw")
    {
        cv::cvtColor(m_rawFrame, retFrame, CV_BGR2RGB);
    }
    else if (mode == "Binary")
    {
        cv::cvtColor(m_binaryFrame, retFrame, CV_Lab2RGB);
    }
    else if (mode == "Smoothed")
    {
        cv::cvtColor(m_smoothedFrame, retFrame, CV_Lab2RGB);
    }
    else
    {
        cv::cvtColor(m_cpuFrame, retFrame, CV_Lab2RGB);
    }
    return retFrame;
}


cv::cuda::GpuMat BeeTracker2d::processFrame(cv::cuda::GpuMat labFrame)
{
    std::vector<cv::cuda::GpuMat> channelsLab(3);
    cv::cuda::split(labFrame, channelsLab);
    cv::cuda::GpuMat smoothed;
    m_gaussianFilter->apply(channelsLab[0], smoothed);
    m_gaussianFilter->apply(smoothed, smoothed);
    cv::cuda::addWeighted(channelsLab[0], 1, smoothed, -1, 150, channelsLab[0]);
    cv::cuda::merge(channelsLab, labFrame);
    labFrame.download(m_smoothedFrame);
    cv::cuda::threshold(channelsLab[0], channelsLab[0], m_threshold, 255, cv::THRESH_BINARY_INV);
    cv::cuda::merge(channelsLab, labFrame);
    labFrame.download(m_binaryFrame);
    cv::SimpleBlobDetector::Params params;
    params.minDistBetweenBlobs = 10.0f;
    params.filterByInertia = true;
    params.filterByConvexity = false;
    params.filterByColor = false;
    params.filterByCircularity = true;
    params.filterByArea = true;
    params.minArea = 30.0;
    params.maxArea = 6000.0;
    params.minInertiaRatio = 0.1;
    params.minThreshold = 1;
    //params.minConvexity = 0.3;
    params.minCircularity = 0.1;

    cv::Ptr<cv::SimpleBlobDetector> blob_detector = cv::SimpleBlobDetector::create(params);
    std::vector<cv::KeyPoint> keypoints;

    blob_detector->detect(m_binaryFrame, keypoints);
    //øm_firstFrame.copyTo(m_cpuFrame);
    for (auto blob : keypoints)
    {
        //cv::floodFill(m_frameFinal, blob.pt, 0);
        cv::circle(m_cpuFrame, blob.pt, 5, cv::Scalar(0, 255, 0), -1);
    }

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
    cv::cuda::threshold(channelsLab[1], maskBlueA, 126, 255, cv::THRESH_BINARY_INV);
    cv::cuda::threshold(channelsLab[2], maskBlueB, 126, 255, cv::THRESH_BINARY_INV);
    cv::cuda::threshold(channelsLab[2], maskYellow, 144, 255, cv::THRESH_BINARY);
    cv::cuda::threshold(channelsLab[1], maskPurple, 135, 255, cv::THRESH_BINARY);

    cv::cuda::bitwise_and(maskBlueA, maskBlueB, maskBlue);
    cv::cuda::bitwise_or(maskBlue, maskBlueB2, maskBlue);
    cv::cuda::bitwise_or(maskBlue, maskYellow, maskFinal);
    cv::cuda::bitwise_or(maskGreen, maskFinal, maskFinal);
    cv::cuda::bitwise_or(maskFinal, maskPurple, maskFinal);

    cv::cuda::add(channelsLab[0], 255, channelsLab[0], maskFinal);

    cv::cuda::merge(channelsLab, labMat);
    return labMat;
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

int BeeTracker2d::getThreshold() const
{
    return m_threshold;
}

void BeeTracker2d::setThreshold(int threshold)
{
    m_threshold = threshold;
}
