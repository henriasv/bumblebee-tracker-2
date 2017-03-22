#ifndef BEETRACKER2D_H
#define BEETRACKER2D_H

#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudafilters.hpp>


class BeeTracker2d
{
public:
    BeeTracker2d();
    cv::VideoCapture m_cam;
    void load(std::string filename, bool flipFlag);
    cv::Mat getFrame(int frameIndex, std::string mode);

private:
    cv::cuda::GpuMat processFrame(cv::cuda::GpuMat labFrame);
    cv::cuda::GpuMat colorFilter(cv::cuda::GpuMat labFrame);
    cv::Mat roiMask(cv::Mat input, int threshold);

    cv::Mat m_cpuFrame;
    cv::Mat m_rawFrame;
    cv::Mat m_binaryFrame;
    cv::Mat m_smoothedFrame;

    cv::cuda::GpuMat m_frameUnprocessed;
    cv::cuda::GpuMat m_frameColorfiltered;
    cv::cuda::GpuMat m_frameProcessed;

    cv::Ptr<cv::cuda::Filter> m_gaussianFilter;
    cv::Ptr<cv::cuda::Filter> m_morphologyFilter;

    bool m_flipFlag;

};

#endif // BEETRACKER2D_H