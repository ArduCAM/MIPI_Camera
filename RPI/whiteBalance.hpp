#ifndef __WHITE_BALANCE_HPP__
#define __WHITE_BALANCE_HPP__
#include <opencv2/opencv.hpp>

cv::Mat dynamicThreWihteBalance(cv::Mat image);
int getWhitePointThre(cv::Mat whiteRegion);
void WhitePointMask(cv::Mat Cr, cv::Mat Cb, cv::Mat RL);
cv::Mat choiceWhitePoint(cv::Mat YCrCb, int mBlocks, int nBlocks);
#endif
