#include "stereohandler.h"
#include <opencv2/core/cuda.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/cudastereo.hpp>
#include <iostream>
StereoHandler::StereoHandler(std::shared_ptr<BeeTracker2d> camA, std::shared_ptr<BeeTracker2d> camB)
    : m_camA(camA), m_camB(camB)
{

}

cv::Mat StereoHandler::compute()
{
    cv::Mat outputCpu;
    cv::cuda::GpuMat frameA = m_camA->m_frameUnprocessed;
    //frameA.download(outputCpu);
    //return outputCpu;
    cv::cuda::GpuMat frameB = m_camB->m_frameUnprocessed;
    //cv::cuda::cvtColor(frameA, frameA, CV_BGR2Lab);
    //cv::cuda::cvtColor(frameB, frameB, CV_BGR2Lab);
    cv::Ptr<cv::cuda::StereoConstantSpaceBP> stereo = cv::cuda::createStereoConstantSpaceBP (128, 8, 4, 4, CV_32F);
    cv::cuda::GpuMat output;
    stereo->compute(frameA, frameB, output);
    cv::cuda::GpuMat output8u;
    output.convertTo(output8u, CV_8U);
    std::cout << output8u.depth() << std::endl;
    cv::cuda::cvtColor(output8u, output, CV_GRAY2BGR);
    output.download(outputCpu);
    m_frame = outputCpu;
    return outputCpu;
}


