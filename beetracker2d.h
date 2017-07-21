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
    BeeTracker2d(QString camName);
    cv::VideoCapture m_cam;
    void load(std::string filename, bool flipFlag);
    void getFrame(int frameIndex, std::string mode);
    void setParameters(int window1, int window2, int minimumArea, int maximumArea);

    QString getDumpString();

    int getThreshold() const;
    void setThreshold(int threshold);
    void setFlowerDetectionColorFilterParameters(std::string, int, int, int, int, int, int);
    void setColorFilteringParameters(int, int, int, int, int);
    void setRoiMaskVector(int, int, int, int, int, int, int, int);
    void setRoiMaskVector(std::vector<int>);

    int getMaxFrame() const;
    void setMaxFrame(int maxFrame);
    cv::Mat m_cpuFrame;
    cv::Mat m_gray;
    QString m_camName;
    void setFrameOffset(int frameOffset);

private:
    cv::cuda::GpuMat processFrame(cv::cuda::GpuMat labFrame, std::string mode);
    cv::cuda::GpuMat colorFilter(cv::cuda::GpuMat labFrame);
    void colorFilterForFlowerDetection(cv::cuda::GpuMat labFrame);
    void drawFlowerBoxes();
    void drawFlowerBeeMatches(cv::Mat& frame, std::vector<cv::KeyPoint> keypoints);
    bool insideRectangle(cv::RotatedRect rect, cv::Point point);
    cv::Mat roiMask(cv::Mat input, int threshold);
    cv::Mat hardCodedRoiMask(cv::Mat input);

    cv::SimpleBlobDetector::Params m_blobParams;

    //cv::Mat m_rawFrame;
    cv::Mat m_binaryFrame;
    //cv::Mat m_smoothedFrame;
    //cv::Mat m_colorFilteredFrame;
    cv::Mat m_identifierFrame;
    //cv::Mat m_outputFrame;
    cv::cuda::GpuMat m_firstFrame;
    cv::cuda::GpuMat m_blueFlowerMask;
    cv::cuda::GpuMat m_yellowFlowerMask;

    cv::cuda::GpuMat m_frameUnprocessed;
    //cv::cuda::GpuMat m_frameColorfiltered;
    cv::cuda::GpuMat m_frameProcessed;

    cv::Ptr<cv::cuda::Filter> m_gaussianFilter;
    cv::Ptr<cv::cuda::Filter> m_morphologyFilter;
    cv::Ptr<cv::cuda::Filter> m_erodeFilter;
    cv::Ptr<cv::cuda::Filter> m_flowerDilateFilter;

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
    std::vector<int> m_beesOnFlower;
    std::vector<int> m_roiMaskVector;

    bool m_flipFlag;
    int m_threshold = 125;

    int m_maxFrame;
    int m_previousFrame = -1;
    int m_frameOffset;

    // paramters for flower color filtering
    int m_FDthreshL0Yellow = 150;
    int m_FDthreshL1Yellow = 105;
    int m_FDthreshL2Yellow = 145;
    int m_FDthreshL0BlueLight = 140;
    int m_FDthreshL2BlueLight = 125;
    int m_FDthreshL2BlueDark = 122;
    std::string m_FDflowerType = "split";

    int m_CFthreshL1Yellow = 112;
    int m_CFthreshL2Yellow = 140;
    int m_CFthreshL0BlueLight = 140;
    int m_CFthreshL2BlueLight = 124;
    int m_CFthreshL2BlueDark = 121;

};

#endif // BEETRACKER2D_H
