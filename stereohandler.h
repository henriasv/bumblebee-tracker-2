#ifndef STEREOHANDLER_H
#define STEREOHANDLER_H

#include <beetracker2d.h>
//#include <opencv2/cudafeatures2d.hpp>
#include <memory>

class StereoHandler {
public:
    StereoHandler(std::shared_ptr<BeeTracker2d> camA, std::shared_ptr<BeeTracker2d> camB);

    std::shared_ptr<BeeTracker2d> m_camA;
    std::shared_ptr<BeeTracker2d> m_camB;

    cv::Mat compute(std::string mode);

    cv::Mat m_frame;
};

#endif // STEREOHANDLER_H
