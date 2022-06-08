#pragma once
#include <iostream>
#include <fstream>
#include <bitset>
#include <string>
#include <time.h>
#include <limits.h>
#include <vector>
#include <bitset>
#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>


#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;

cv::Mat get_bin_image(string path);
//extern "C" int getopt(int argc, char* const* argv, const char* optstring);

typedef union _LARGE_INTEGER {
    struct {
        unsigned int LowPart;
        int HighPart;
    } DUMMYSTRUCTNAME;
    struct {
        unsigned int LowPart;
        int HighPart;
    } u;
    long long QuadPart;
} LARGE_INTEGER;

#define EXIT_ERROR(x)                                 \
	do                                                \
	{                                                 \
		cout << "error in file " << __FILE__ << " in function " << __FUNCTION__ << " in line " << __LINE__ << endl; \
		cout << x;                                    \
		getchar();                                    \
		exit(EXIT_FAILURE);                           \
	} while (0)
