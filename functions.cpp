#include <functions.h>
#include <vector>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

struct less_than_key
{
    inline bool operator() (const cv::RotatedRect& struct1, const cv::RotatedRect& struct2)
    {
            return (struct1.center.x < struct2.center.x);
    }
};

void averageFlowerRectangleCoordinates(std::vector<cv::RotatedRect>& rectsA, std::vector<cv::RotatedRect>& rectsB, cv::Mat homography)
{
    if (rectsA.size() != rectsB.size())
    {
        std::cout << "Rectangle vectors of different lengths for different cameras. This means the number of identified flowers from each camera are different. This is BAD!" << std::endl;
        return;
    }
    std::vector<cv::RotatedRect> rectsATransformed;
    rectsATransformed.resize(rectsA.size());
    //cv::perspectiveTransform(rectsA, rectsATransformed, homography);
    /*
    for (int i = 0; i<rectsA.size(); i++)
    {
        // iterate rectangle corners
        cv::Point2f ptsA[4];
        cv::Point2f ptsB[4];
        rectsA[i].points(ptsA);
        rectsB[i].points(ptsB);

        std::vector<cv::Point2f> pointsA;
        std::vector<cv::Point2f> pointsATransformed;


        for (int j = 0; j<4; j++)
        {
            pointsA.push_back(ptsA[j]);
        }
        cv::Size2f size = rectsA[i].size();
        cv::Point2f center = rectsA[i].center();
        double angle = rects[i].


        rectsA[i] = cv::RotatedRect(pointsA[0], pointsA[1], pointsA[2]);
        cv::perspectiveTransform(pointsA, pointsATransformed, homography);
    }
*/

}

void sortMinimumDistanceInChain(std::vector<cv::RotatedRect>& rects, std::vector<int>& flowerColors)
{
    std::vector<int> index(rects.size(), 0);
    for (int i = 0; i<index.size(); i++)
    {
        index[i] = i;
    }
    for (auto v : index)
        std::cout << v << " ";
    std::cout << std::endl;
    for (int i = 0; i<rects.size(); i++)
    {
        cv::Point2f pt3 = rects[index[i]].center;
        sort(index.begin()+i+1, index.end(),
            [&](const int& a, const int& b) {
                cv::Point2f pt1 = rects[a].center;
                cv::Point2f pt2 = rects[b].center;
                cv::Point2f da = pt3-pt1;
                cv::Point2f db = pt3-pt2;
                double dist1 = da.dot(da);
                double dist2 = db.dot(db);
                return (dist1 < dist2);
            });
    }

    std::vector<cv::RotatedRect> outRects;
    std::vector<int> outFlowerColors;
    for (int i = 0; i<rects.size(); i++)
    {
        outRects.push_back(rects[index[i]]);
        outFlowerColors.push_back(flowerColors[index[i]]);
    }
    rects = outRects;
    flowerColors = outFlowerColors;
    return;
}

void matchingRectanglesByHomography(std::vector<cv::RotatedRect> rectsA, std::vector<cv::RotatedRect>& rectsB, std::vector<int>& flowerColorsB, cv::Mat homography)
{
    std::vector<cv::Point2f> centersA;
    std::vector<cv::Point2f> perspectiveCentersA;
    perspectiveCentersA.resize(centersA.size());

    for (int i = 0; i< rectsA.size(); i++)
    {
        centersA.push_back(rectsA[i].center);
    }
    cv::perspectiveTransform(centersA, perspectiveCentersA, homography);


    std::vector<int> index(rectsB.size(), 0);
    for (int i = 0; i<index.size(); i++)
    {
        index[i] = i;
    }

    int len = (rectsA.size()<rectsB.size() ? rectsA.size() : rectsB.size());

    for (int i = 0; i<len; i++)
    {
        sort(index.begin()+i, index.end(),
            [&](const int& a, const int& b)
            {
                 cv::Point2f pt1 = perspectiveCentersA[i];
                 cv::Point2f pt2 = rectsB[a].center;
                 cv::Point2f pt3 = rectsB[b].center;
                 cv::Point2f da = pt1-pt2;
                 cv::Point2f db = pt1-pt3;
                 return (da.dot(da) < db.dot(db));
            });
    }

    std::vector<cv::RotatedRect> outRects;
    std::vector<int> outFlowerColors;
    for (int i = 0; i<rectsB.size(); i++)
    {
        outRects.push_back(rectsB[index[i]]);
        outFlowerColors.push_back((flowerColorsB[index[i]]));
    }
    rectsB = outRects;
    flowerColorsB = outFlowerColors;
}

void sortFlowersAndGroupByClosenessAndColor(
        std::vector<cv::RotatedRect>& flowerRectsA,
        std::vector<int>& flowerColorsA,
        std::vector<cv::RotatedRect>& flowerRectsB,
        std::vector<int>& flowerColorsB,
        cv::Mat& homography)
{
    std::vector<int> colorsA;
    std::vector<int> colorsB;


    sortMinimumDistanceInChain(flowerRectsA, flowerColorsA);
    matchingRectanglesByHomography(flowerRectsA, flowerRectsB, flowerColorsB, homography);
    averageFlowerRectangleCoordinates(flowerRectsA, flowerRectsB, homography);
    return;
}
