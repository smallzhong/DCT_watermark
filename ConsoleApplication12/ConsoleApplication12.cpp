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
		//puts("");
	}

	//imshow("src", src);
	//waitKey(0);

	return src;
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

int g_ct = 0;


Mat read_bin_icon_and_resize(string path, int row, int col)
{
	Mat src = get_bin_image(path);
	resize(src, src, { row, col });

	return src;
}

/// <summary>
/// 
/// </summary>
/// <param name="src">原始图像</param>
/// <param name="seed">混沌种子</param>
/// <param name=""></param>
/// <returns></returns>
Mat chaos_xor(Mat& src, uint seed)
{
	int row = src.rows, col = src.cols;
	srand(seed);
	Mat dest(row, col, src.type());
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
		{
			// TODO:
			dest.at<uchar>(i, j) = src.at<uchar>(i, j) ^ (rand() % 255);
		}
	}

	return dest;
}

Mat get_icon_from_file_and_encrypt(string path, uint seed, int row, int col,
	int times = 17, int a = 1, int b = 1, int c = 1, int d = 2)
{
	Mat src = read_bin_icon_and_resize(path, row, col);
	src = Arnold(src, YC_ARNOLD_CUSTOM, times, a, b, c, d);
	src = chaos_xor(src, seed);

	return src;
}

Mat decrypt_icon(Mat& src, uint seed, int row = -1, int col = -1, int times = 17,
	int a = 2, int b = -1, int c = -1, int d = 1)
{
	Mat dest = src.clone();
	if (~row || ~col)
	{
		resize(dest, dest, { row, col });
	}
	dest = chaos_xor(dest, seed);
	dest = Arnold(dest, YC_ARNOLD_CUSTOM, times, a, b, c, d);

	return dest;
}

void test5()
{
	Mat src = get_icon_from_file_and_encrypt("lena512.png", 'zyc', 512, 512);
	imshow("src", src);
	imwrite("666.png", src);
	Mat dest = decrypt_icon(src, 'zyc');
	imshow("dest", dest);

	waitKey(0);
}

void test6()
{
	Mat src = imread("666.png", 0);
	src = decrypt_icon(src, 'zyc', 512, 512);
	imshow("dest", src);
	waitKey(0);
}

void test4()
{
	Mat src = imread("lena512.png");
	int row = src.rows, col = src.cols;

	imshow("src", src);
	printf("src.type() = %d\n", src.type());
	printf("CV_32F = %d CV_32FC1 = %d CV_32FC3 = %d\n", CV_32F, CV_32FC1, CV_32FC3);
	vector<Mat> channels;
	split(src, channels);
	printf("channels.size() = %d\n", channels.size());
	for (auto& cur_img : channels)
	{
		static int ct = 0;
		Mat dest;
		cur_img.convertTo(cur_img, CV_32FC1, 1. / 255.);
		dct(cur_img, dest, 0);

		int s = 2;
		// 全图dct变换，8*8
		for (int i = 0; i < row; i += s)
		{
			for (int j = 0; j < col; j += s)
			{
				Mat t_mat(s, s, cur_img.type());

				for (int x = 0; x < s; x++)
				{
					for (int y = 0; y < s; y++)
					{
						t_mat.at<float>(x, y) = cur_img.at<float>(i + x, j + y);
					}
				}

				dct(t_mat, t_mat);

				float a = t_mat.at<float>(s - 1, 0);
				float b = t_mat.at<float>(0, s - 1);
				if (a < b)
				{
					swap(t_mat.at<float>(0, s - 1), t_mat.at<float>(s - 1, 0));
					g_ct++;
				}
				for (int x = 0; x < s; x++)
				{
					for (int y = 0; y < s; y++)
					{
						cur_img.at<float>(i + x, j + y) = t_mat.at<float>(x, y);

					}
				}
			}
		}

		// 全图逆dct变换，8*8
		for (int i = 0; i < row; i += s)
		{
			for (int j = 0; j < col; j += s)
			{
				Mat t_mat(s, s, cur_img.type());

				for (int x = 0; x < s; x++)
				{
					for (int y = 0; y < s; y++)
					{
						t_mat.at<float>(x, y) = cur_img.at<float>(i + x, j + y);
					}
				}

				idct(t_mat, t_mat);
				for (int x = 0; x < s; x++)
				{
					for (int y = 0; y < s; y++)
					{
						cur_img.at<float>(i + x, j + y) = t_mat.at<float>(x, y);
					}
				}
			}
		}
	}

	Mat dest;
	merge(channels, dest);
	imshow("dest", dest);

	printf("g_ct = %d\n", g_ct);
	waitKey(0);
}

void test7(string path)
{
	// TODO:以后可以考虑封装一个可变长bitset的类，每次1.5倍增加大小，重载[]符号
	bitset<100000> res;
	Mat src = imread(path);
	imwrite("1.png", src);
	int row = src.rows, col = src.cols;

	vector<Mat> channels;
	split(src, channels);
	for (auto& cur_img : channels)
	{
		static int ct = 0;
		cur_img.convertTo(cur_img, CV_32FC1, 1. / 255.);
		imshow(to_string(ct++), cur_img);

		int s = 8;
		// 全图DCT变换，置换高频
		//for (int i = 0; i < row; i += s)
		//{
		//	for (int j = 0; j < col; j += s)
		//	{
		//		Mat t_mat(s, s, cur_img.type());

		//		for (int x = 0; x < s; x++)
		//		{
		//			for (int y = 0; y < s; y++)
		//			{
		//				t_mat.at<float>(x, y) = cur_img.at<float>(i + x, j + y);
		//			}
		//		}

		//		dct(t_mat, t_mat);

		//		float a = t_mat.at<float>(s - 1, 0);
		//		float b = t_mat.at<float>(0, s - 1);
		//		if (a < b)
		//		{
		//			swap(t_mat.at<float>(0, s - 1), t_mat.at<float>(s - 1, 0));
		//			g_ct++;
		//		}
		//		for (int x = 0; x < s; x++)
		//		{
		//			for (int y = 0; y < s; y++)
		//			{
		//				cur_img.at<float>(i + x, j + y) = t_mat.at<float>(x, y);

		//			}
		//		}
		//	}
		//}

		//// 全图IDCT变换，重新填入
		//for (int i = 0; i < row; i += s)
		//{
		//	for (int j = 0; j < col; j += s)
		//	{
		//		Mat t_mat(s, s, cur_img.type());

		//		for (int x = 0; x < s; x++)
		//		{
		//			for (int y = 0; y < s; y++)
		//			{
		//				t_mat.at<float>(x, y) = cur_img.at<float>(i + x, j + y);
		//			}
		//		}

		//		idct(t_mat, t_mat);
		//		for (int x = 0; x < s; x++)
		//		{
		//			for (int y = 0; y < s; y++)
		//			{
		//				cur_img.at<float>(i + x, j + y) = t_mat.at<float>(x, y);
		//			}
		//		}
		//	}
		//}
	}


	Mat dest;
	merge(channels, dest);
	imshow("dest", dest);

	// 防止保存时为全黑
	normalize(dest, dest, 0, 255, NORM_MINMAX, CV_8U);
	imwrite("2.png", dest);

	printf("g_ct = %d\n", g_ct);
	waitKey(0);
}

int main()
{
	init();

	//test1();
	//test2();
	//test3();
	//test4();
	//test5();
	//test6();
	test7("lena512.png");

	return 0;
}


