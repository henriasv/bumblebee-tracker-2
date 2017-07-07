#include "beetracker2d.h"
#include <iostream>
#include <fstream>

BeeTracker2d::BeeTracker2d()
{
    cv::cuda::setDevice(0);
    cv::cuda::printShortCudaDeviceInfo(cv::cuda::getDevice());

    m_gaussianFilter = cv::cuda::createGaussianFilter(CV_8UC1, CV_8UC1, cv::Size(3, 3), 32);
    m_gaussianFilterMax = cv::cuda::createGaussianFilter(CV_8UC1, CV_8UC1, cv::Size(31, 31), 32);
    m_gaussianFilterDOG1 = cv::cuda::createGaussianFilter(CV_8UC1, CV_8UC1, cv::Size(15, 15), 32);
    m_gaussianFilterDOG2 = cv::cuda::createGaussianFilter(CV_8UC1, CV_8UC1, cv::Size(31, 31), 32);

    m_surfDetector = cv::cuda::SURF_CUDA(50, 4, 2, true, 100.0f, true);

    m_edgeDetector = cv::cuda::createCannyEdgeDetector(128, 200, 5);
    cv::Mat element = getStructuringElement(cv::MORPH_RECT, cv::Size(4, 4));
    //m_morphologyFilter = cv::cuda::createMorphologyFilter(cv::MORPH_OPEN, CV_8UC1, element, cv::Point(-1,-1),1);
    m_morphologyFilter = cv::cuda::createMorphologyFilter(cv::MORPH_DILATE, CV_8UC1, element, cv::Point(-1,-1),1);

    cv::Mat elementErode = getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
    m_erodeFilter = cv::cuda::createMorphologyFilter(cv::MORPH_ERODE, CV_8UC1, elementErode, cv::Point(-1,-1),1);

    m_blobParams.minDistBetweenBlobs = 1.0f;
    m_blobParams.filterByInertia = true;
    m_blobParams.filterByConvexity = false;
    m_blobParams.filterByColor = false;
    m_blobParams.filterByCircularity = false;

    m_blobParams.filterByArea = true;
    m_blobParams.minArea = 100.0f;
    m_blobParams.maxArea = 100000.0f;
    m_blobParams.minInertiaRatio = 0.05;

    //params.minThreshold = 0;
    //params.maxThreshold = 130;
    //params.minConvexity = 0.3;
    //params.minCircularity = 0.1;
}

void BeeTracker2d::load(std::string filename, bool flipFlag)
{
    std::ifstream infile(filename);
    if (!infile.is_open())
    {
        std::cout << "impossible to open video file" << std::endl;
    }
    else
    {
        infile.close();
    }
    m_flipFlag = flipFlag;
    m_cam.open(filename);
    if (!m_cam.isOpened())
    {
        std::cout << "Could not open video source" << std::endl;
    }
    setMaxFrame(m_cam.get(CV_CAP_PROP_FRAME_COUNT)-2);
}

struct less_than_key
{
    inline bool operator() (const cv::RotatedRect& struct1, const cv::RotatedRect& struct2)
    {
            return (struct1.center.x < struct2.center.x);
    }
};


void BeeTracker2d::getFrame(int frameIndex, std::string mode)
{
    if (m_previousFrame+1 != frameIndex)
    {
        m_cam.set(CV_CAP_PROP_POS_FRAMES, frameIndex);
    }

    bool ret = m_cam.read(m_cpuFrame);
    std::cout << "Frame reported by video stream" << m_cam.get(CV_CAP_PROP_POS_FRAMES) << std::endl;

    while (!ret)
    {
        std::cout << "Frame reported by video stream" << m_cam.get(CV_CAP_PROP_POS_FRAMES) << std::endl;
        m_cam.set(CV_CAP_PROP_POS_FRAMES, ++frameIndex);
        ret = m_cam.read(m_cpuFrame);
        if (!ret)
        {
            std::cout << "Failed to load frame on second attempt" << std::endl;

        }
    }
    m_previousFrame = frameIndex;
    int frameIndexForPrint = m_cam.get(CV_CAP_PROP_POS_FRAMES);
    std::cout << "Got frame " << frameIndexForPrint << std::endl;

    cv::transpose(m_cpuFrame, m_cpuFrame);
    if (m_flipFlag)
    {
        cv::flip(m_cpuFrame, m_cpuFrame, -1);
    }

    //m_frameUnprocessed.upload(m_cpuFrame);




    //m_cpuFrame = roiMask(m_cpuFrame, 130);
    if (m_flipFlag)
    {
        m_cpuFrame = hardCodedRoiMask(m_cpuFrame, 55, 100, 1460, 580, 1460, 2120, 65, 2595);
    }
    else
    {
        m_cpuFrame = hardCodedRoiMask(m_cpuFrame, 65, 610, 1440, 170, 1440, 2595, 65, 2150);
    }
    m_frameUnprocessed.upload(m_cpuFrame);

    if (mode == "Raw")
    {
        cv::cuda::cvtColor(m_frameUnprocessed, m_frameUnprocessed, CV_BGR2RGB);
        m_frameUnprocessed.download(m_cpuFrame);
        return;
    }

    cv::cuda::cvtColor(m_frameUnprocessed, m_frameUnprocessed, CV_BGR2Lab);
    m_frameProcessed = processFrame(m_frameUnprocessed, mode);

    if (mode == "Bounding boxes")
    {
        colorFilterForFlowerDetection(m_frameUnprocessed);
        std::vector<cv::cuda::GpuMat> cannyMats;
        cv::cuda::GpuMat cannyMatYellow;
        cv::cuda::GpuMat cannyMatBlue;
        //std::vector<cv::cuda::GpuMat> channelsLab;
        //cv::cuda::split(m_frameProcessed, channelsLab);
        m_edgeDetector->detect(m_blueFlowerMask, cannyMatBlue);
        m_edgeDetector->detect(m_yellowFlowerMask, cannyMatYellow);

        cannyMats.push_back(cannyMatBlue);
        cannyMats.push_back(cannyMatYellow);

        int counter = 0;
        m_flowerRects.clear();
        m_flowerColors.clear();
        for (auto cannyMat : cannyMats)
        {
            std::vector< std::vector< cv::Point> > contours;
            cv::Mat cpuCannyMat;
            cannyMat.download(cpuCannyMat);
            cv::findContours(cpuCannyMat, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
            for (int i = 0; i<contours.size(); ++i)
            {
                cv::RotatedRect rotatedRect;
                rotatedRect = cv::minAreaRect(contours[i]);
                if (rotatedRect.size.area() > 100)
                {
                    m_flowerRects.push_back(rotatedRect);
                    m_flowerColors.push_back(counter);
                }

            }
            // Because of camera orientation, flowers will be sorted on x coordinate. Wrong sortings will be correced using RANSAC and homography in the stereo handler
            //std::sort(m_flowerRects[counter].begin(), m_flowerRects[counter].end(), less_than_key());
            counter ++;
        }
        //drawFlowerBoxes();
        return;
    }


    if (mode == "Raw + Identifiers")
    {
        std::vector<cv::cuda::GpuMat> channelsLab;
        cv::cuda::split(m_frameProcessed, channelsLab);
        channelsLab[0].download(m_binaryFrame);
        std::cout << "Processing raw + identifiers" << std::endl;
        cv::cvtColor(m_cpuFrame, m_cpuFrame, CV_BGR2RGB);

        cv::Ptr<cv::SimpleBlobDetector> blob_detector = cv::SimpleBlobDetector::create(m_blobParams);

        blob_detector->detect(m_binaryFrame, keypoints);
        for (cv::KeyPoint blob : keypoints)
        {
            cv::circle(m_cpuFrame, blob.pt, blob.size, cv::Scalar(0, 255, 0), -1);
        }
        drawFlowerBoxes();
        drawFlowerBeeMatches(m_cpuFrame, keypoints);
        cv::cuda::split(m_frameUnprocessed, channelsLab);
        m_surfDetector.uploadKeypoints(keypoints, keypoints1GPU);
        m_surfDetector(channelsLab[0], cv::cuda::GpuMat(), keypoints1GPU, descriptors1GPU, true);
    }



    else if (mode == "Features")
    {
        cv::cuda::cvtColor(m_frameProcessed, m_frameProcessed, CV_Lab2RGB);
        m_frameProcessed.download(m_cpuFrame);
        cv::drawKeypoints(m_cpuFrame, keypoints, m_cpuFrame);
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

QString BeeTracker2d::getDumpString()
{
    QString outString;
    QTextStream out(&outString);
    out << "{ \"Frame\" : " << m_previousFrame << ",\n \"Keypoints\" : [";
    for (int i = 0; i<keypoints.size()-1; i++)
    {
        out <<" [ " << keypoints[i].pt.x << ", " << keypoints[i].pt.y << " ], \n";
    }
    out <<" [ " << keypoints[keypoints.size()-1].pt.x << ", " << keypoints[keypoints.size()-1].pt.y << " ] \n";
    out << "] \n }";
    //QDebug() << outString;
    return outString;
}


cv::cuda::GpuMat BeeTracker2d::processFrame(cv::cuda::GpuMat labFrame, std::string mode)
{

    std::vector<cv::cuda::GpuMat> channelsLab(3);
    cv::cuda::split(labFrame, channelsLab);
    for (int i = 1; i<3; i++)
    {
        m_gaussianFilter->apply(channelsLab[2], channelsLab[2]);
    }

    cv::cuda::merge(channelsLab, labFrame);

    if (mode == "Features")
    {
        m_surfDetector(channelsLab[0], cv::cuda::GpuMat(), keypoints1GPU, descriptors1GPU);
        std::cout << "FOUND " << keypoints1GPU.cols << " keypoints on first image" << std::endl;
        m_surfDetector.downloadDescriptors(descriptors1GPU, descriptors);
        m_surfDetector.downloadKeypoints(keypoints1GPU, keypoints);
        //m_surfDetector(img2, cv::cuda::GpuMat(), keypoints2GPU, descriptors2GPU);
        return labFrame;
    }


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
    //m_gaussianFilterDOG1->apply(channelsLab[2], dog2);
    //m_gaussianFilterDOG2->apply(channelsLab[2], dog1);



    cv::cuda::subtract(dog2,dog1, channelsLab[0]);
    cv::cuda::multiply(channelsLab[0], 10, channelsLab[0]);
    cv::cuda::merge(channelsLab, labFrame);

    if (mode == "DOG")
        return labFrame;

    cv::cuda::threshold(channelsLab[0], channelsLab[0], m_threshold, 255, cv::THRESH_BINARY_INV);
    m_erodeFilter->apply(channelsLab[0], channelsLab[0]);
    cv::cuda::merge(channelsLab, labFrame);
    return labFrame;
}



cv::cuda::GpuMat BeeTracker2d::colorFilter(cv::cuda::GpuMat labMat)
{
    std::vector<cv::cuda::GpuMat> channelsLab;
    cv::cuda::split(labMat, channelsLab);

    cv::cuda::GpuMat maskBlueLight, maskBlueDark;

    cv::cuda::GpuMat maskLight, maskNotGreen;


    cv::cuda::GpuMat blurred;
    m_gaussianFilterMax->apply(channelsLab[0], blurred);
    m_gaussianFilterMax->apply(blurred, blurred);
    m_gaussianFilterMax->apply(blurred, blurred);

    cv::cuda::threshold(channelsLab[0], maskLight, 140, 255, cv::THRESH_BINARY);
    cv::cuda::threshold(channelsLab[2], maskBlueLight, 124, 255, cv::THRESH_BINARY_INV);
    cv::cuda::threshold(channelsLab[2], maskBlueDark, 121, 255, cv::THRESH_BINARY_INV);
    cv::cuda::bitwise_and(maskBlueLight, maskLight, m_blueFlowerMask);
    cv::cuda::bitwise_or(maskBlueDark, m_blueFlowerMask, m_blueFlowerMask);
    cv::cuda::subtract(channelsLab[0], channelsLab[0], channelsLab[0], m_blueFlowerMask);
    cv::cuda::add(channelsLab[0], blurred, channelsLab[0], m_blueFlowerMask);


    cv::cuda::threshold(channelsLab[2], m_yellowFlowerMask, 140, 255, cv::THRESH_BINARY);
    cv::cuda::threshold(channelsLab[1], maskNotGreen, 112, 255, cv::THRESH_BINARY);
    cv::cuda::bitwise_and(m_yellowFlowerMask, maskNotGreen, m_yellowFlowerMask);
    cv::cuda::subtract(channelsLab[0], channelsLab[0], channelsLab[0], m_yellowFlowerMask);
    cv::cuda::add(channelsLab[0], blurred, channelsLab[0], m_yellowFlowerMask);

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

void BeeTracker2d::colorFilterForFlowerDetection(cv::cuda::GpuMat labMat)
{
    std::vector<cv::cuda::GpuMat> channelsLab;
    cv::cuda::split(labMat, channelsLab);

    cv::cuda::GpuMat maskBlueLight, maskBlueDark;

    cv::cuda::GpuMat maskLight, maskNotGreen;

    cv::cuda::threshold(channelsLab[0], maskLight, 140, 255, cv::THRESH_BINARY);
    cv::cuda::threshold(channelsLab[2], maskBlueLight, 124, 255, cv::THRESH_BINARY_INV);
    cv::cuda::threshold(channelsLab[2], maskBlueDark, 121, 255, cv::THRESH_BINARY_INV);
    cv::cuda::bitwise_and(maskBlueLight, maskLight, m_blueFlowerMask);
    cv::cuda::bitwise_or(maskBlueDark, m_blueFlowerMask, m_blueFlowerMask);

    cv::cuda::threshold(channelsLab[2], m_yellowFlowerMask, 140, 255, cv::THRESH_BINARY);
    cv::cuda::threshold(channelsLab[1], maskNotGreen, 113, 255, cv::THRESH_BINARY);
    cv::cuda::bitwise_and(m_yellowFlowerMask, maskNotGreen, m_yellowFlowerMask);
}


void BeeTracker2d::drawFlowerBoxes()
{
    cv::Mat frame = m_cpuFrame;
    for (int i = 0; i<m_flowerRects.size(); i++)
    {
            cv::Point2f points[4];
            m_flowerRects[i].points(points);
            std::vector< std::vector< cv::Point> > polylines;
            polylines.resize(1);
            for(int k = 0; k < 4; ++k)
            {
                polylines[0].push_back(points[k]);
            }
            if (m_flowerColors[i] == 0)
                cv::polylines(frame, polylines, true, cv::Scalar(0, 0, 255), 2);
            else if (m_flowerColors[i] == 1)
                cv::polylines(frame, polylines, true, cv::Scalar(255, 255, 0), 2);
            cv::putText(frame, std::to_string(i), m_flowerRects[i].center, 1, 5, cv::Scalar(0,0,0), 2);
    }
}

void BeeTracker2d::drawFlowerBeeMatches(cv::Mat &frame, std::vector<cv::KeyPoint> keypoints)
{
    for (int i = 0; i<m_flowerRects.size(); i++)
    {
           for (cv::KeyPoint keypoint : keypoints)
           {
               if (insideRectangle(m_flowerRects[i], keypoint.pt))
               {
                   cv::Point2f points[4];
                   m_flowerRects[i].points(points);
                   std::vector< std::vector< cv::Point> > polylines;
                   polylines.resize(1);
                   for (int k = 0; k<4; ++k)
                   {
                       polylines[0].push_back(points[k]);
                   }
                   cv::fillPoly(frame, polylines, cv::Scalar(255,0,0, 0.3));
                   std::cout << "Bee over flower" << std::endl;
               }
           }
    }
    //cv::rectangle(frame, )
}

bool BeeTracker2d::insideRectangle(cv::RotatedRect rect, cv::Point point)
{
    cv::Point2f pts[4];
    rect.points(pts);
    cv::Point A = pts[0];
    cv::Point B = pts[1];
    cv::Point D = pts[3];
    cv::Point AP = point-A;
    cv::Point AB = B-A;
    cv::Point AD = D-A;

    bool inside = (( (0 < AP.dot(AB)) && (AP.dot(AB) < AB.dot(AB)) ) && ( (0 < AP.dot(AD)) && (AP.dot(AD) < AD.dot(AD)) ));
    return inside;
}



cv::Mat BeeTracker2d::roiMask(cv::Mat input, int threshold)
{
    cv::Mat gray;
    cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    cv::threshold(gray, gray, threshold, 255, cv::THRESH_BINARY);

    std::vector<std::vector<cv::Point> > contours;
    //std::vector<cv::Vec4i> hierarchy;
    //cv::findContours( gray, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE );
    //cv::Canny(gray, cannyOutput, 110, 160, 3);
    cv::findContours( gray, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE );

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


    //cv::drawContours( input, contours, iLargest, 255, -1 );

    cv::approxPolyDP(cv::Mat(contours[iLargest]), contours[iLargest], 5, true);
    cv::Scalar color = cv::Scalar( 255,255,255);

    cv::Mat mask = cv::Mat::zeros(gray.size(), CV_8U);
    cv::drawContours( mask, contours, iLargest, 255, -1 );
    cv::Mat not_mask;
    cv::bitwise_not(mask, not_mask);
    input.setTo(140*cv::Scalar(1, 1, 1), not_mask);
    return input;
}

cv::Mat BeeTracker2d::hardCodedRoiMask(cv::Mat input, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
    cv::Mat mask = cv::Mat::zeros(input.rows, input.cols, CV_8U);
    cv::Point pts[4] = {
        cv::Point(x1, y1),
        cv::Point(x2, y2),
        cv::Point(x3, y3),
        cv::Point(x4, y4),
    };
    m_arenaCorners.clear();
    for (int i = 0; i<4; i++)
    {
        m_arenaCorners.push_back(cv::Point2f(pts[i].x, pts[i].y));
    }
    cv::fillConvexPoly(mask, pts, 4, cv::Scalar(255) );
    cv::Mat notMask;
    cv::bitwise_not(mask, notMask);
    input.setTo(140*cv::Scalar(1, 1, 1), notMask);
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
