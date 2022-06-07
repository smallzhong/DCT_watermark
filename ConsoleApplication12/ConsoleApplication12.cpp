#include "head.hpp"

using namespace cv;

void init()
{
	cv::utils::logging::setLogLevel(utils::logging::LOG_LEVEL_SILENT);
}

enum ARNOLD_TYPE
{
	YC_ARNOLD_NORMAL,
	YC_ARNOLD_REVERSE,
	YC_ARNOLD_CUSTOM
};

/// <summary>
/// 
/// 
/// </summary>
/// <param name="img">传入的mat</param>
/// <param name="times">置换次数</param>
/// <param name="arnold_type">置换类型，正常、逆置换、自定义</param>
/// <param name="a">自定义时矩阵参数</param>
/// <param name="b">自定义时矩阵参数</param>
/// <param name="c">自定义时矩阵参数</param>
/// <param name="d">自定义时矩阵参数</param>
/// <returns>完成arnold置换后的图片</returns>
Mat Arnold(Mat& img, ARNOLD_TYPE arnold_type = YC_ARNOLD_NORMAL, int times = 1,
	int a = 1, int b = 1, int c = 1, int d = 2)
{
	int img_type = img.type();
	if (img_type != CV_8UC1 && img_type != CV_8UC3)
	{
		EXIT_ERROR("传入的图片类型错误！");
	}

	Mat src = img.clone();
	if (times == 0) return src; // times=0直接返回

	switch (arnold_type)
	{
	case YC_ARNOLD_NORMAL:
	{
		if (a != 1 || b != 1 || c != 1 || d != 2)
			EXIT_ERROR("使用默认矩阵时不可修改abcd值！");
	}
	break;
	case YC_ARNOLD_REVERSE:
	{
		a = 2, b = -1, c = -1, d = 1;
	}
	break;
	case YC_ARNOLD_CUSTOM:
	{
		// TODO:可以考虑修改为支持自定义矩阵维数
	}
	break;
	default:
		EXIT_ERROR("arnold_type错误！");
		break;
	}


	int row = src.rows, col = src.cols;
	int N = row;
	Mat dest(row, col, img_type);
	if (row != col) EXIT_ERROR("row != col!");

	while (times--)
	{
		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < col; j++)
			{
				int x1 = (a * i + b * j + N) % N;
				int y1 = (c * i + d * j + N) % N;

				switch (img_type)
				{
				case CV_8UC1:
				{
					dest.at<uchar>(x1, y1) = src.at<uchar>(i, j);
				}
				break;
				case CV_8UC3:
				{
					dest.at<Vec3b>(x1, y1) = src.at<Vec3b>(i, j);
				}
				break;
				default:
				{
					EXIT_ERROR("不应发生");
				}
				break;
				}
			}
		}
		dest.copyTo(src);
	}

	return dest;
}

void test1()
{
	Mat src = imread("lena512.png");
	printf("%d %d\n", src.type(), CV_8UC3);
	Mat dest = Arnold(src, YC_ARNOLD_NORMAL, 20);

	//imshow("src", src);
	//imshow("dest", Arnold(dest, YC_ARNOLD_REVERSE));

	imwrite("out.png", dest);
}

void test2()
{

	Mat src = imread("out.png");
	resize(src, src, { 512, 512 });
	src = Arnold(src, YC_ARNOLD_REVERSE, 20);
	imshow("src", src);

	waitKey(0);
}

void test3()
{
	get_bin_image("lena512.png");
}

Mat get_bin_image(string path)
{
	Mat src = imread(path, 0);
	int row = src.rows, col = src.cols;

	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
		{
			uchar t = src.at<uchar>(i, j);
			if (t > 255 / 2) src.at<uchar>(i, j) = UCHAR_MAX;
			else src.at<uchar>(i, j) = 0;
		}
		puts("");
	}

	imshow("src", src);
	waitKey(0);

	return src;
}

int main()
{
	init();

	//test1();
	//test2();
	test3();

	return 0;
}


