#include "head.hpp"

using namespace cv;

int ct = 0;
const int bits_size = 1e5 + 10;
int a_x_embed = 3;
int a_y_embed = 3;
int b_x_embed = 3;
int b_y_embed = 2;
float embed_addup = .01;
int matrix_cols = 4;

#define DEFAULT_TIMES 17

#define show(a) imshow(to_string(ct ++ ), a)

void set_global_params(int col)
{
	int* p = (int*)(&a_x_embed);
	*p = col - 1;

	p = (int*)(&a_y_embed);
	*p = col - 1;

	p = (int*)(&b_x_embed);
	*p = col - 1;

	p = (int*)(&b_y_embed);
	*p = col - 2;

	p = (int*)(&matrix_cols);
	*p = col;

	printf("修改全局变量完成！%d %d %d %d %d\n", a_x_embed, a_y_embed, b_x_embed, b_y_embed, matrix_cols);
}
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

Mat embed_watermark(string path, bitset<bits_size>& bits, int img_row = -1, int img_col = -1)
{
	Mat src = imread(path);
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

		// 全图DCT变换
		for (int i = 0; i <= row - matrix_cols; i += matrix_cols)
		{
			for (int j = 0; j <= col - matrix_cols; j += matrix_cols)
			{
				Mat t_mat(matrix_cols, matrix_cols, cur_img.type());

				for (int x = 0; x < matrix_cols; x++)
				{
					for (int y = 0; y < matrix_cols; y++)
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
				for (int x = 0; x < matrix_cols; x++)
				{
					for (int y = 0; y < matrix_cols; y++)
					{
						cur_img.at<float>(i + x, j + y) = t_mat.at<float>(x, y);
					}
				}
			}
		}

		// 全图IDCT变换，重新填入
		for (int i = 0; i <= row - matrix_cols; i += matrix_cols)
		{
			for (int j = 0; j < col; j += matrix_cols)
			{
				Mat t_mat(matrix_cols, matrix_cols, cur_img.type());

				for (int x = 0; x < matrix_cols; x++)
				{
					for (int y = 0; y < matrix_cols; y++)
					{
						t_mat.at<float>(x, y) = cur_img.at<float>(i + x, j + y);
					}
				}

				idct(t_mat, t_mat);
				for (int x = 0; x < matrix_cols; x++)
				{
					for (int y = 0; y < matrix_cols; y++)
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

		// 全图DCT变换
		for (int i = 0; i <= row - matrix_cols; i += matrix_cols)
		{
			for (int j = 0; j <= col - matrix_cols; j += matrix_cols)
			{
				Mat t_mat(matrix_cols, matrix_cols, cur_img.type());

				for (int x = 0; x < matrix_cols; x++)
				{
					for (int y = 0; y < matrix_cols; y++)
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
				for (int x = 0; x < matrix_cols; x++)
				{
					for (int y = 0; y < matrix_cols; y++)
					{
						cur_img.at<float>(i + x, j + y) = t_mat.at<float>(x, y);
					}
				}
			}
		}

		// 全图IDCT变换，重新填入
		for (int i = 0; i <= row - matrix_cols; i += matrix_cols)
		{
			for (int j = 0; j <= col - matrix_cols; j += matrix_cols)
			{
				Mat t_mat(matrix_cols, matrix_cols, cur_img.type());

				for (int x = 0; x < matrix_cols; x++)
				{
					for (int y = 0; y < matrix_cols; y++)
					{
						t_mat.at<float>(x, y) = cur_img.at<float>(i + x, j + y);
					}
				}

				idct(t_mat, t_mat);
				for (int x = 0; x < matrix_cols; x++)
				{
					for (int y = 0; y < matrix_cols; y++)
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

		// 全图DCT变换，提取
		for (int i = 0; i <= row - matrix_cols; i += matrix_cols)
		{
			for (int j = 0; j <= col - matrix_cols; j += matrix_cols)
			{
				Mat t_mat(matrix_cols, matrix_cols, cur_img.type());

				for (int x = 0; x < matrix_cols; x++)
				{
					for (int y = 0; y < matrix_cols; y++)
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
				for (int x = 0; x < matrix_cols; x++)
				{
					for (int y = 0; y < matrix_cols; y++)
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

Mat extract_watermark(Mat src, int icon_row, int icon_col, int img_row = -1, int img_col = -1)
{
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

		// 全图DCT变换，提取
		for (int i = 0; i <= row - matrix_cols; i += matrix_cols)
		{
			for (int j = 0; j <= col - matrix_cols; j += matrix_cols)
			{
				Mat t_mat(matrix_cols, matrix_cols, cur_img.type());

				for (int x = 0; x < matrix_cols; x++)
				{
					for (int y = 0; y < matrix_cols; y++)
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
				for (int x = 0; x < matrix_cols; x++)
				{
					for (int y = 0; y < matrix_cols; y++)
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

Mat 嵌入水印(string img_path, string icon_path, uint seed)
{
	Mat src = imread(img_path);
	int max_capacity = (src.cols / 4) * (src.cols / 4) * 3; // 4*4作为最小单位，算出当前图像最多能存放多少bit

	Mat icon = imread(icon_path);
	// 如果水印长宽不等，进行调整
	if (icon.rows != icon.cols)
	{
		resize(icon, icon, { max(icon.rows, icon.cols) , max(icon.rows, icon.cols) });
	}
	int icon_bits = icon.rows * icon.rows;
	// 如果发现容量不足
	if (icon_bits > max_capacity)
	{
		int col_max = (int)(floor(sqrt(max_capacity)));
		// 缩小到最大容量
		resize(icon, icon, { col_max, col_max });
		icon_bits = col_max * col_max; // 更新icon_bit
	}

	// 自适应地确定嵌入水印时选择的方阵大小
	int t_matrix_cols = 8;
	// 返回false表示方阵设置得太大了，会放不下。true表示这个可以
	auto check = [=](int col) -> bool {
		int t_capa = (src.rows / col) * (src.cols / col) * 3;
		if (t_capa < icon_bits) return false;
		else return true;
	};

	// 如果8已经大了，那么不断减小
	if (!check(t_matrix_cols))
	{
		do
		{
			t_matrix_cols--;
		} while (!check(t_matrix_cols));
	}
	// 找到最大的矩阵宽度
	else
	{
		do
		{
			t_matrix_cols++;
		} while (check(t_matrix_cols));
		t_matrix_cols--;
	}
	set_global_params(t_matrix_cols);

	bitset<bits_size> bits = get_icon_from_file_and_encrypt(icon_path, seed, 90, 90);

	Mat embeded = embed_watermark(src, bits);
	return embeded;
}

Mat 提取水印(string path, uint seed, int param, int icon_row, int icon_col, int img_row = -1, int img_col = -1)
{
	assert(icon_col == icon_row);
	set_global_params(param);

	Mat src = imread(path);
	if (~img_row) img_row = src.rows;
	if (~img_col) img_col = src.cols;

	Mat extracted_icon = extract_watermark(src, icon_row, icon_col, img_row, img_col);
	extracted_icon = decrypt_watermark(extracted_icon, seed, icon_row, icon_col);
	return extracted_icon;
}

Mat 提取水印(Mat src, uint seed, int param, int icon_row, int icon_col, int img_row = -1, int img_col = -1)
{
	assert(icon_col == icon_row);
	set_global_params(param);

	if (~img_row) img_row = src.rows;
	if (~img_col) img_col = src.cols;

	Mat extracted_icon = extract_watermark(src, icon_row, icon_col, img_row, img_col);
	extracted_icon = decrypt_watermark(extracted_icon, seed, icon_row, icon_col);
	return extracted_icon;
}


int main()
{
	init();

	//Mat src = 嵌入水印("lena512.png", "icon.png", 'zyc');
	//imwrite("embeded.png", src);
	Mat dest = 提取水印("embeded.png", 'zyc', 9, 90, 90);
	show(dest);
	waitKey(0);

	return 0;
}


