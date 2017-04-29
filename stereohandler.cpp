#include "stereohandler.h"
#include <opencv2/core/cuda.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/cudastereo.hpp>
#include <iostream>
StereoHandler::StereoHandler(std::shared_ptr<BeeTracker2d> camA, std::shared_ptr<BeeTracker2d> camB)
    : m_camA(camA), m_camB(camB)
{
    m_matcher = cv::cuda::DescriptorMatcher::createBFMatcher(camA->m_surfDetector.defaultNorm());
}

cv::Mat StereoHandler::compute()
{

    /*
    cv::Mat imgMatches;
    std::vector<cv::DMatch> matches;
    m_matcher->match(m_camA->descriptors1GPU, m_camB->descriptors1GPU, matches);
    drawMatches(m_camA->m_cpuFrame, m_camA->keypoints, m_camB->m_cpuFrame, m_camB->keypoints, matches, imgMatches);
    m_frame = imgMatches;
    //return imgMatches;
    */

    cv::Mat outputCpu;
    cv::cuda::GpuMat frameA = m_camA->m_frameUnprocessed;
    //frameA.download(outputCpu);
    //return outputCpu;
    cv::cuda::GpuMat frameB = m_camB->m_frameUnprocessed;
    cv::cuda::cvtColor(frameA, frameA, CV_BGR2GRAY);
    cv::cuda::cvtColor(frameB, frameB, CV_BGR2GRAY);
    //cv::Ptr<cv::cuda::StereoConstantSpaceBP> stereo = cv::cuda::createStereoConstantSpaceBP (16, 8, 4, 4, CV_32F);
    cv::Ptr<cv::cuda::StereoBM> stereo = cv::cuda::createStereoBM (128, 21);
    //cv::Ptr<cv::cuda::StereoBeliefPropagation> stereo = cv::cuda::createStereoBeliefPropagation(16, 5, 2, CV_32F);
    cv::cuda::GpuMat output;
    stereo->compute(frameB, frameA, output);
    cv::cuda::GpuMat output8u;
    output.convertTo(output8u, CV_8U);
    std::cout << output8u.depth() << std::endl;
    cv::cuda::cvtColor(output8u, output, CV_GRAY2BGR);
    output.download(outputCpu);
    m_frame = outputCpu;
    return outputCpu;
}


