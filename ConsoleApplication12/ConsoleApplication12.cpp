#include <iostream>
#include "head.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>


using namespace cv;

void init()
{
	cv::utils::logging::setLogLevel(utils::logging::LOG_LEVEL_SILENT);
}

int main()
{
	init();
	Mat backImg = imread("lena1.png");
	imshow("lena1.png", backImg);
	waitKey(0);
}


