#ifndef BEETRACKER2D_H
#define BEETRACKER2D_H

#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/cudafeatures2d.hpp>
#include <opencv2/xfeatures2d/cuda.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudafilters.hpp>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDebug>

class StereoHandler;
class BeeTracker2d
{
    friend class StereoHandler;
public:
    BeeTracker2d();
    cv::VideoCapture m_cam;
    void load(std::string filename, bool flipFlag);
    void getFrame(int frameIndex, std::string mode);
    void setParameters(int window1, int window2, int minimumArea, int maximumArea);

    QString getDumpString();

    int getThreshold() const;
    void setThreshold(int threshold);

    int getMaxFrame() const;
    void setMaxFrame(int maxFrame);
    cv::Mat m_cpuFrame;
    cv::Mat m_gray;
private:
    cv::cuda::GpuMat processFrame(cv::cuda::GpuMat labFrame, std::string mode);
    cv::cuda::GpuMat colorFilter(cv::cuda::GpuMat labFrame);
    cv::cuda::GpuMat simpleColorFilter(cv::cuda::GpuMat labFrame);
    void colorFilterForFlowerDetection(cv::cuda::GpuMat labFrame);
    void drawFlowerBoxes();
    void drawFlowerBeeMatches(cv::Mat& frame, std::vector<cv::KeyPoint> keypoints);
    bool insideRectangle(cv::RotatedRect rect, cv::Point point);
    cv::Mat roiMask(cv::Mat input, int threshold);
    cv::Mat hardCodedRoiMask(cv::Mat input, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);

    cv::SimpleBlobDetector::Params m_blobParams;

    //cv::Mat m_rawFrame;
    cv::Mat m_binaryFrame;
    //cv::Mat m_smoothedFrame;
    //cv::Mat m_colorFilteredFrame;
    cv::Mat m_identifierFrame;
    //cv::Mat m_outputFrame;

    cv::cuda::GpuMat m_blueFlowerMask;
    cv::cuda::GpuMat m_yellowFlowerMask;

    cv::cuda::GpuMat m_frameUnprocessed;
    //cv::cuda::GpuMat m_frameColorfiltered;
    cv::cuda::GpuMat m_frameProcessed;

    cv::Ptr<cv::cuda::Filter> m_gaussianFilter;
    cv::Ptr<cv::cuda::Filter> m_morphologyFilter;
    cv::Ptr<cv::cuda::Filter> m_erodeFilter;

    cv::Ptr<cv::cuda::CannyEdgeDetector> m_edgeDetector;

    cv::Ptr<cv::cuda::Filter> m_gaussianFilterDOG1;
    cv::Ptr<cv::cuda::Filter> m_gaussianFilterDOG2;
    cv::Ptr<cv::cuda::Filter> m_gaussianFilterMax;
    cv::cuda::SURF_CUDA m_surfDetector;
    cv::cuda::GpuMat keypoints1GPU;
    cv::cuda::GpuMat descriptors1GPU;
    std::vector<cv::KeyPoint> keypoints;
    std::vector<float> descriptors;
    std::vector<cv::RotatedRect> m_flowerRects;
    std::vector<int>             m_flowerColors;
    std::vector<cv::Point2f> m_arenaCorners;

    bool m_flipFlag;
    int m_threshold = 125;

    int m_maxFrame;
    int m_previousFrame = -1;

};

#endif // BEETRACKER2D_H
