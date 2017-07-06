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

cv::Mat StereoHandler::compute(std::string mode)
{
    if (mode == "Bounding boxes")
    {
        std::cout << "Bounding box handling in stereo handler" << std::endl;
        std::vector<cv::Point2f> pointsA;
        std::vector<cv::Point2f> pointsB;
        std::vector<cv::Point2f> newpointsA;
        std::vector<cv::Point2f> newpointsB;

        for (cv::RotatedRect rect :  m_camA->m_flowerRects[0])
        {
            pointsA.push_back(rect.center);
        }

        for (cv::RotatedRect rect :  m_camB->m_flowerRects[0])
        {
            pointsB.push_back(rect.center);
        }

        for (cv::RotatedRect rect :  m_camA->m_flowerRects[1])
        {
            pointsA.push_back(rect.center);
        }

        for (cv::RotatedRect rect :  m_camB->m_flowerRects[1])
        {
            pointsB.push_back(rect.center);
        }

        for (cv::Point2f pt : m_camA->m_arenaCorners)
        {
            pointsA.push_back(pt);
        }

        for (cv::Point2f pt : m_camB->m_arenaCorners)
        {
            pointsB.push_back(pt);
        }

        newpointsA.resize(pointsA.size());
        newpointsB.resize(pointsB.size());

        cv::Mat homography = cv::findHomography(pointsA, pointsB, CV_RANSAC);

        cv::Mat fundamentalMatrix = cv::findFundamentalMat(pointsA, pointsB, CV_FM_RANSAC, 3, 0.9);
        //std::cout << "homography: " << std::endl << homography << std::endl;
        std::cout << fundamentalMatrix << std::endl;
        std::vector<cv::Vec3f> lines1;
        std::vector<cv::Vec3f> lines2;
        cv::computeCorrespondEpilines(cv::Mat(pointsA), 1, fundamentalMatrix, lines1);
        cv::computeCorrespondEpilines(cv::Mat(pointsB), 1, fundamentalMatrix, lines2);

        for (std::vector<cv::Vec3f>::const_iterator it = lines2.begin(); it!=lines2.end(); ++it)
           {
            // Draw the line between first and last column
            cv::line(m_camA->m_cpuFrame,
                  cv::Point(0,-(*it)[2]/(*it)[1]),
                  cv::Point(m_camA->m_cpuFrame.cols,-((*it)[2]+
                                               (*it)[0]*m_camA->m_cpuFrame.cols)/(*it)[1]),
                  cv::Scalar(255,255,255));
            }

        for (std::vector<cv::Vec3f>::const_iterator it = lines1.begin(); it!=lines1.end(); ++it)
           {

               cv::line(m_camB->m_cpuFrame,
                     cv::Point(0,-(*it)[2]/(*it)[1]),
                     cv::Point(m_camB->m_cpuFrame.cols,-((*it)[2]+
                                                  (*it)[0]*m_camB->m_cpuFrame.cols)/(*it)[1]),
                     cv::Scalar(255,255,255));
               //cv::line(m_camB->m_cpuFrame,
               //         cv::Point(0,-(*it)[2]/(*it)[1]),
               //         cv::Point(m_camB->m_cpuFrame.cols,-((*it)[2]+
               //                                      (*it)[0]*m_camB->m_cpuFrame.cols)/(*it)[1]),
               //         cv::Scalar(255,255,255));
           }
        pointsA.clear();
        pointsB.clear();
        m_camA->drawFlowerBoxes();
        m_camB->drawFlowerBoxes();
        //cv::warpPerspective(m_camA->m_cpuFrame, m_camA->m_cpuFrame, homography, m_camA->m_cpuFrame.size());

    }

    /*
    cv::Mat imgMatches;
    std::vector<cv::DMatch> matches;
    m_matcher->match(m_camA->descriptors1GPU, m_camB->descriptors1GPU, matches);
    drawMatches(m_camA->m_cpuFrame, m_camA->keypoints, m_camB->m_cpuFrame, m_camB->keypoints, matches, imgMatches);
    m_frame = imgMatches;
    //return imgMatches;
    */

    cv::Mat outputCpu;

    cv::addWeighted(m_camA->m_cpuFrame, 0.5, m_camB->m_cpuFrame, 0.5, 0, outputCpu);
    m_frame = outputCpu;
    return outputCpu;
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


