#include "stereohandler.h"
#include <opencv2/calib3d.hpp>
#include <opencv2/core/cuda.hpp>
//#include <opencv2/cudastereo.hpp>
#include <functions.h>
#include <iostream>

StereoHandler::StereoHandler(std::shared_ptr<BeeTracker2d> camA, std::shared_ptr<BeeTracker2d> camB)
    : m_camA(camA)
    , m_camB(camB)
{
}

cv::Mat StereoHandler::compute(std::string mode)
{
    if (mode == "Bounding boxes") {
        std::cout << "Bounding box handling in stereo handler" << std::endl;
        std::vector<cv::Point2f> pointsA;
        std::vector<cv::Point2f> pointsB;
        std::vector<cv::Point2f> homographyPointsA;
        std::vector<cv::Point2f> homographyPointsB;

        for (cv::RotatedRect rect : m_camA->m_flowerRects) {
            homographyPointsA.push_back(rect.center);
        }

        for (cv::RotatedRect rect : m_camB->m_flowerRects) {
            homographyPointsB.push_back(rect.center);
        }

        std::cout << "Length of rectA: " << homographyPointsA.size();
        std::cout << "Length of rectB: " << homographyPointsB.size();

        // bonus points
        pointsA.push_back(cv::Point2f(980.000000, 1176.000000));
        pointsA.push_back(cv::Point2f(704.000000, 630.000000));
        pointsA.push_back(cv::Point2f(791.000000, 2152.000000));
        pointsA.push_back(cv::Point2f(716.000000, 2060.000000));
        pointsA.push_back(cv::Point2f(72.000000, 2623.000000));
        pointsA.push_back(cv::Point2f(74.000000, 112.000000));
        pointsA.push_back(cv::Point2f(178.000000, 129.000000));
        pointsA.push_back(cv::Point2f(1475.000000, 550.000000));
        pointsA.push_back(cv::Point2f(1471.000000, 2123.000000));
        pointsA.push_back(cv::Point2f(628.000000, 158.000000));
        pointsA.push_back(cv::Point2f(662.000000, 295.000000));
        pointsA.push_back(cv::Point2f(1121.000000, 448.000000));
        pointsA.push_back(cv::Point2f(94.000000, 2204.000000));
        pointsA.push_back(cv::Point2f(1135.000000, 1981.000000));
        pointsA.push_back(cv::Point2f(320.000000, 1355.000000));
        pointsA.push_back(cv::Point2f(792.000000, 2508.000000));
        pointsA.push_back(cv::Point2f(48.000000, 41.000000));
        pointsA.push_back(cv::Point2f(1373.000000, 2110.000000));
        pointsA.push_back(cv::Point2f(48.000000, 1331.000000));
        pointsA.push_back(cv::Point2f(1476.000000, 1342.000000));
        pointsB.push_back(cv::Point2f(1092.000000, 1176.000000));
        pointsB.push_back(cv::Point2f(802.000000, 664.000000));
        pointsB.push_back(cv::Point2f(725.000000, 2169.000000));
        pointsB.push_back(cv::Point2f(819.000000, 2081.000000));
        pointsB.push_back(cv::Point2f(70.000000, 2164.000000));
        pointsB.push_back(cv::Point2f(63.000000, 621.000000));
        pointsB.push_back(cv::Point2f(121.000000, 588.000000));
        pointsB.push_back(cv::Point2f(1481.000000, 112.000000));
        pointsB.push_back(cv::Point2f(1458.000000, 2621.000000));
        pointsB.push_back(cv::Point2f(640.000000, 287.000000));
        pointsB.push_back(cv::Point2f(394.000000, 500.000000));
        pointsB.push_back(cv::Point2f(848.000000, 351.000000));
        pointsB.push_back(cv::Point2f(377.000000, 1980.000000));
        pointsB.push_back(cv::Point2f(1347.000000, 2226.000000));
        pointsB.push_back(cv::Point2f(473.000000, 1384.000000));
        pointsB.push_back(cv::Point2f(634.000000, 2482.000000));
        pointsB.push_back(cv::Point2f(202.000000, 522.000000));
        pointsB.push_back(cv::Point2f(1317.000000, 2502.000000));
        pointsB.push_back(cv::Point2f(59.000000, 1380.000000));
        pointsB.push_back(cv::Point2f(1478.000000, 1384.000000));

        int len = (homographyPointsA.size() < homographyPointsB.size() ? homographyPointsA.size() : homographyPointsB.size());
        homographyPointsA.resize(len);
        homographyPointsB.resize(len);

        cv::Mat homography = cv::findHomography(homographyPointsA, homographyPointsB, CV_RANSAC);

        cv::Mat fundamentalMatrix = cv::findFundamentalMat(pointsA, pointsB, CV_FM_8POINT, 3, 0.9);
        //std::cout << "homography: " << std::endl << homography << std::endl;
        std::cout << fundamentalMatrix << std::endl;
        std::vector<cv::Vec3f> lines1;
        std::vector<cv::Vec3f> lines2;
        cv::computeCorrespondEpilines(cv::Mat(pointsA), 1, fundamentalMatrix, lines1);
        cv::computeCorrespondEpilines(cv::Mat(pointsB), 2, fundamentalMatrix, lines2);

        int counter = 0;
        for (std::vector<cv::Vec3f>::const_iterator it = lines2.begin(); it != lines2.end(); ++it) {
            // Draw the line between first and last column
            cv::line(m_camA->m_cpuFrame,
                cv::Point(0, -(*it)[2] / (*it)[1]),
                cv::Point(m_camA->m_cpuFrame.cols, -((*it)[2] + (*it)[0] * m_camA->m_cpuFrame.cols) / (*it)[1]),
                cv::Scalar(255, 255, 255));
            cv::circle(m_camA->m_cpuFrame, pointsA[counter], 5, cv::Scalar(255, 0, 0));
            counter++;
        }

        counter = 0;
        for (std::vector<cv::Vec3f>::const_iterator it = lines1.begin(); it != lines1.end(); ++it) {

            cv::line(m_camB->m_cpuFrame,
                cv::Point(0, -(*it)[2] / (*it)[1]),
                cv::Point(m_camB->m_cpuFrame.cols, -((*it)[2] + (*it)[0] * m_camB->m_cpuFrame.cols) / (*it)[1]),
                cv::Scalar(255, 255, 255));
            cv::circle(m_camB->m_cpuFrame, pointsB[counter], 5, cv::Scalar(255, 0, 0));
            counter++;
        }
        pointsA.clear();
        pointsB.clear();

        sortFlowersAndGroupByClosenessAndColor(m_camA->m_flowerRects, m_camA->m_flowerColors, m_camB->m_flowerRects, m_camB->m_flowerColors, homography);

        m_camA->drawFlowerBoxes();
        m_camB->drawFlowerBoxes();
        //cv::warpPerspective(m_camA->m_cpuFrame, m_camA->m_cpuFrame, homography, m_camA->m_cpuFrame.size());
    }

    cv::Mat outputCpu;

    cv::addWeighted(m_camA->m_cpuFrame, 0.5, m_camB->m_cpuFrame, 0.5, 0, outputCpu);
    m_frame = outputCpu;
    return outputCpu;
}
