#include "head.hpp"

using namespace cv;

int ct = 0;
const int bits_size = 1e5 + 10;
const int a_x_embed = 7;
const int a_y_embed = 7;
const int b_x_embed = 6;
const int b_y_embed = 7;
const float embed_addup = .01;

#define DEFAULT_TIMES 17


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
Mat Arnold(Mat& img, ARNOLD_TYPE arnold_type = YC_ARNOLD_NORMAL, int times = DEFAULT_TIMES,
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

	return src;
}

bitset<bits_size> chaos_xor_mat(Mat src, uint seed)
{
	int row = src.rows, col = src.cols;
	srand(seed);
	bitset<bits_size> bits;
	int cur = 0;
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
		{
			int t = src.at<uchar>(i, j) ? 1 : 0;
			t ^= (rand() % 2);
			bits[cur++] = t & 1;
		}
	}

	return bits;
}

bitset<bits_size> get_bitset_from_mat(Mat src)
{
	int cur = 0;
	bitset<bits_size> bits;
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			if (src.at<uchar>(i, j)) bits[cur++] = 1;
			else bits[cur++] = 0;
		}
	}

	return bits;
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

bitset<bits_size> get_icon_from_file_and_encrypt(string path, uint seed, int row, int col,
	int times = DEFAULT_TIMES, int a = 1, int b = 1, int c = 1, int d = 2)
{
	Mat src = read_bin_icon_and_resize(path, row, col);
	src = Arnold(src, YC_ARNOLD_CUSTOM, times, a, b, c, d);

	srand(seed);
	bitset<bits_size> bits;
	int cur = 0;
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
		{
			int t = src.at<uchar>(i, j) ? 1 : 0;
			t ^= (rand() % 2);
			bits[cur++] = t & 1;
		}
	}

	return bits;
}

Mat decrypt_watermark(Mat& src, uint seed, int row = -1, int col = -1, int times = DEFAULT_TIMES,
	int a = 2, int b = -1, int c = -1, int d = 1)
{
	Mat dest = src.clone();
	if (~row || ~col)
	{
		resize(dest, dest, { row, col });
	}

	srand(seed);
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
		{
			int t = (rand() % 2) * UCHAR_MAX;
			dest.at<uchar>(i, j) ^= t;
		}
	}

	dest = Arnold(dest, YC_ARNOLD_CUSTOM, times, a, b, c, d);

	return dest;
}

Mat embed_watermark(string path, bitset<bits_size>& bits)
{
	Mat src = imread(path);
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
					t_mat.at<float>(a_x_embed, a_y_embed) += embed_addup;
					t_mat.at<float>(b_x_embed, b_y_embed) -= embed_addup;
				}
				else
				{
					t_mat.at<float>(a_x_embed, a_y_embed) -= embed_addup;
					t_mat.at<float>(b_x_embed, b_y_embed) += embed_addup;
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

Mat embed_watermark(Mat src, bitset<bits_size>& bits, int img_row = -1, int img_col = -1)
{
	if (~img_row || ~img_col)
	{
		resize(src, src, { img_row, img_col });
	}

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
					t_mat.at<float>(a_x_embed, a_y_embed) += embed_addup;
					t_mat.at<float>(b_x_embed, b_y_embed) -= embed_addup;
				}
				else
				{
					t_mat.at<float>(a_x_embed, a_y_embed) -= embed_addup;
					t_mat.at<float>(b_x_embed, b_y_embed) += embed_addup;
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

Mat extract_watermark(string path, int icon_row, int icon_col, int img_row = -1, int img_col = -1)
{
	Mat src = imread(path);
	if (~img_row || ~img_col)
	{
		resize(src, src, { img_row, img_col });
	}

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

	Mat res(icon_row, icon_col, 0);
	cur = 0;
	for (int i = 0; i < icon_row; i++)
	{
		for (int j = 0; j < icon_col; j++)
		{
			if (bits[cur++] == 1) res.at<uchar>(i, j) = UCHAR_MAX;
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
	bitset<bits_size> bits = get_icon_from_file_and_encrypt("icon.png", 'zyc', 90, 90);
	waitKey(1000);

	Mat embeded = embed_watermark("lena512.png", bits);
	//show(embeded);
	imwrite("embeded.png", embeded);
}

void test11()
{
	Mat extracted_icon = extract_watermark("embeded.png", 90, 90, 512, 512);

	extracted_icon = decrypt_watermark(extracted_icon, 'zyc', 90, 90);

	show(extracted_icon);
	waitKey(0);
}

int main()
{
	init();

	//show(imread("lena512.png"));
	//test10();
	test11();

	//waitKey(0);
	return 0;
}


