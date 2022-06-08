#include "head.hpp"

using namespace cv;

int ct = 0;
#define show(a) imshow(to_string(ct ++ ), a)
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

const int bits_size = 1e5 + 10;
const int a_x_embed = 7;
const int a_y_embed = 7;
const int b_x_embed = 6;
const int b_y_embed = 7;

Mat embed_watermark(string path, bitset<bits_size>& bits)
{
	Mat src = imread(path);
	show(src);
	int cur = 0;
	int row = src.rows, col = src.cols;

	vector<Mat> channels;
	split(src, channels);
	for (auto& cur_img : channels)
	{
		cur_img.convertTo(cur_img, CV_32FC1, 1. / 255.);

		int s = 8;
		// 全图DCT变换
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

				
				float a = t_mat.at<float>(a_x_embed, a_y_embed);
				float b = t_mat.at<float>(b_x_embed, b_y_embed);
				// a>b表示1，a<b表示0
				if (((a > b) && (bits[cur] == 1)) || ((a < b) && (bits[cur] == 0)))
				{

				}
				else
				{
					swap(t_mat.at<float>(a_x_embed, a_y_embed), t_mat.at<float>(b_x_embed, b_y_embed));
					swap(a, b);
				}

				if (a > b)
				{
					t_mat.at<float>(a_x_embed, a_y_embed) += .005;
					t_mat.at<float>(b_x_embed, b_y_embed) -= .005;
				}
				else
				{
					t_mat.at<float>(a_x_embed, a_y_embed) -= .005;
					t_mat.at<float>(b_x_embed, b_y_embed) += .005;
				}

				cur++;
				for (int x = 0; x < s; x++)
				{
					for (int y = 0; y < s; y++)
					{
						cur_img.at<float>(i + x, j + y) = t_mat.at<float>(x, y);
					}
				}
			}
		}

		// 全图IDCT变换，重新填入
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

	// 防止保存时为全黑
	normalize(dest, dest, 0, 255, NORM_MINMAX, CV_8U);

	return dest;
}

Mat extract_watermark(string path, int icon_row, int icon_col)
{
	Mat src = imread(path);
	int cur = 0;
	bitset<bits_size> bits;
	int row = src.rows, col = src.cols;

	vector<Mat> channels;
	split(src, channels);
	for (auto& cur_img : channels)
	{
		show(cur_img);
		cur_img.convertTo(cur_img, CV_32FC1, 1. / 255.);

		int s = 8;
		// 全图DCT变换，提取
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

				float a = t_mat.at<float>(a_x_embed, a_y_embed);
				float b = t_mat.at<float>(b_x_embed, b_y_embed);
				// a>b表示1，a<b表示0
				if (a > b)
				{
					bits[cur++] = 1;
				}
				else
				{
					bits[cur++] = 0;
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
	}

	Mat res(icon_row, icon_col, 0);
	cur = 0;
	for (int i = 0; i < icon_row; i++)
	{
		for (int j = 0; j < icon_col; j++)
		{
			if (bits[cur++] == 1) res.at<uchar>(i, j) = 255;
			else res.at<uchar>(i, j) = 0;
		}
	}

	return res;
}


Mat extract_watermark(Mat src, int icon_row, int icon_col)
{
	int cur = 0;
	bitset<bits_size> bits;
	int row = src.rows, col = src.cols;

	vector<Mat> channels;
	split(src, channels);
	for (auto& cur_img : channels)
	{
		cur_img.convertTo(cur_img, CV_32FC1, 1. / 255.);


		int s = 8;
		// 全图DCT变换，提取
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

				float a = t_mat.at<float>(a_x_embed, a_y_embed);
				float b = t_mat.at<float>(b_x_embed, b_y_embed);
				// a>b表示1，a<b表示0
				if (a > b)
				{
					bits[cur++] = 1;
				}
				else
				{
					bits[cur++] = 0;
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
	}

	printf("cur = %d\n", cur);
	Mat res(icon_row, icon_col, 0);
	cur = 0;
	for (int i = 0; i < icon_row; i++)
	{
		for (int j = 0; j < icon_col; j++)
		{
			if (bits[cur++] == 1) res.at<uchar>(i, j) = 255;
			else res.at<uchar>(i, j) = 0;
		}
	}

	return res;
}

void test10()
{
	Mat icon = get_bin_image("icon.png");
	bitset<bits_size> bits;
	int pos = 0;
	for (int i = 0; i < icon.rows; i++)
	{
		for (int j = 0; j < icon.cols; j++)
		{
			bits[pos++] = icon.at<uchar>(i, j) ? 1 : 0;
		}
	}

	Mat embeded = embed_watermark("lena512.png", bits);
	Mat extracted_icon = extract_watermark(embeded, icon.rows, icon.cols);
	show(icon);
	show(embeded);
	show(extracted_icon);


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
	//test7("lena512.png");
	//test8("1.png");
	//test8("2.png");
	test10();

	//waitKey(0);
	return 0;
}


