#ifndef BEETRACKER2D_H
#define BEETRACKER2D_H

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
//#include <opencv2/xfeatures2d.hpp>
#include <opencv2/features2d.hpp>
//#include <opencv2/cudafeatures2d.hpp>
//#include <opencv2/xfeatures2d/cuda.hpp>
//#include <opencv2/cudaimgproc.hpp>
//#include <opencv2/cudaarithm.hpp>
//#include <opencv2/cudafilters.hpp>
#include <QDebug>
#include <QFile>
#include <QString>
#include <QTextStream>

class StereoHandler;
class BeeTracker2d {
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
    cv::UMat m_cpuFrame;
    cv::UMat m_gray;
    QString m_camName;
    void setFrameOffset(int frameOffset);

    int getFrameOffset() const;

private:
    cv::UMat processFrame(cv::UMat labFrame, std::string mode);
    cv::UMat colorFilter(cv::UMat labFrame);
    void colorFilterForFlowerDetection(cv::UMat labFrame);
    void drawFlowerBoxes();
    void drawFlowerBeeMatches(cv::UMat& frame, std::vector<cv::KeyPoint> keypoints);
    bool insideRectangle(cv::RotatedRect rect, cv::Point point);
    cv::UMat hardCodedRoiMask(cv::UMat input);

    cv::SimpleBlobDetector::Params m_blobParams;
    cv::Ptr<cv::SimpleBlobDetector> m_blob_detector;
    int m_DOGWindow1 = 11;
    int m_DOGWindow2 = 25;

    cv::UMat m_binaryFrame;
    cv::UMat m_identifierFrame;
    cv::UMat m_firstFrame;
    cv::UMat m_blueFlowerMask;
    cv::UMat m_yellowFlowerMask;

    cv::UMat m_frameUnprocessed;
    cv::UMat m_frameColorfiltered;
    cv::UMat m_frameProcessed;

    cv::Mat m_elementErode;

    std::vector<cv::KeyPoint> keypoints;
    std::vector<cv::RotatedRect> m_flowerRects;
    std::vector<int> m_flowerColors;
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
