#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <opencv2/imgproc.hpp>

void sortFlowersAndGroupByClosenessAndColor(
        std::vector<cv::RotatedRect>& flowerRectsA,
        std::vector<int>& flowerColorsA,
        std::vector<cv::RotatedRect>& flowerRectsB,
        std::vector<int>& flowerColorsB,
        cv::Mat& homography);

#endif // FUNCTIONS_H
