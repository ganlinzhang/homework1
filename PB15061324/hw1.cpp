#include "SubImageMatch.h"
#define MY_FAIL -1
#define MY_DONE 1
#define MY_PI 3.14159f
//ustc_ConvertBgr2Gray
int ustc_ConvertBgr2Gray(Mat bgrImg, Mat& grayImg)
{
	int x, y, z;
	if (NULL == bgrImg.data)
	{
		printf("Image Load Failed.\n");
		return MY_FAIL;
	}
	if (bgrImg.channels() != 3)
	{
		printf("bgrImg error\n");
		return MY_FAIL;
	}
	int row = bgrImg.rows;
	int col = bgrImg.cols;
	grayImg.create(row, col, CV_8U);
	for (y = 0; y < row; y++) 
	{
		uchar *data = bgrImg.ptr<uchar>(y); 
		uchar*dst = grayImg.ptr<uchar>(y);  
		for (z = 0, x = 0; x < col; x++) 
		{
			dst[x] = (306 * data[z + 2] + 601 * data[z + 1] + 117 * data[z]) >> 10;
			z += 3;
		}
	}
	return MY_DONE;
}
//ustc_CalcGrad
int ustc_CalcGrad(Mat grayImg, Mat& gradImg_x, Mat& gradImg_y) {
	int x, y;
	int row = grayImg.rows - 1;
	int col = grayImg.cols - 1;
	int gx, gy;
	if (NULL == grayImg.data)
	{
		printf("image load fail\n");
		return MY_FAIL;
	}
	if (grayImg.channels() != 1)
	{
		printf("grayImg error\n");
		return MY_FAIL;
	}
	gradImg_x.create(row + 1, col + 1, CV_32SC1); 
	gradImg_y.create(row + 1, col + 1, CV_32SC1);
	gradImg_x.setTo(0);
	gradImg_y.setTo(0);
	for (y = 1; y < row; y++)
	{
		uchar *data_1 = grayImg.ptr<uchar>(y - 1);
		uchar *data_2 = grayImg.ptr<uchar>(y);
		uchar *data_3 = grayImg.ptr<uchar>(y + 1);
		int*dst_x = gradImg_x.ptr<int>(y);
		int*dst_y = gradImg_y.ptr<int>(y);
		for (x = 1; x < col; x++)
		{
			gx = -data_1[x - 1] + data_1[x + 1] - (data_2[x - 1] << 1) + (data_2[x + 1] << 1) - data_3[x - 1] + data_3[x + 1];
			gy = data_1[x - 1] + (data_1[x] << 1) + data_1[x + 1] - data_3[x - 1] - (data_3[x] << 1) - data_3[x + 1];
			if (gx >> 31) dst_x[x] = -gx;
			else dst_x[x] = gx;
			if (gy >> 31) dst_y[x] = -gy;
			else dst_y[x] = gy;
		}
	}
	return MY_DONE;
}
//myatan2
inline float myatan2(float y, float x)
{
	float angle = atan2(y, x); 
	angle = angle * 180.0f / MY_PI;
	if (angle < 0) angle += 360;
	return angle;
}
//ustc_CalcAngleMag
int ustc_CalcAngleMag(Mat gradImg_x, Mat gradImg_y, Mat&angleImg, Mat& magImg)
{
	if (NULL == gradImg_x.data || NULL == gradImg_y.data)
	{
		printf("image load fail\n");
		return MY_FAIL;
	}
	int col = gradImg_x.cols;
	int row = gradImg_x.rows;
	int i, j;
	int rowedge = row - 1;
	int coledge = col - 1;
	angleImg.create(row, col, CV_32FC1);
	angleImg.setTo(0);
	magImg.create(row, col, CV_32FC1);
	magImg.setTo(0);
	for (i = 1; i < rowedge; i++)
	{
		int* xData = gradImg_x.ptr<int>(i);
		int* yData = gradImg_y.ptr<int>(i);
		float* angleData = angleImg.ptr<float>(i);
		float*magData = magImg.ptr<float>(i);
		for (j = 1; j < coledge; j++)
		{
			int grad_x = xData[j];
			int grad_y = yData[j];
			float angle = myatan2(grad_y, grad_x);
			angleData[j] = angle;
			grad_x = grad_x*grad_x;
			grad_y = grad_y*grad_y;
			magData[j] = sqrt(grad_x + grad_y);
		}
	}
	return MY_DONE;
}
//ustc_Threshold
int ustc_Threshold(Mat grayImg, Mat& binaryImg, int th) {
	int x, y;
	int row = grayImg.rows;
	int col = grayImg.cols;
	int edge = col / 4;
	if (NULL == grayImg.data)
	{
		printf("Image Load Failed\n");
		return MY_FAIL;
	}
	if (grayImg.channels() != 1)
	{
		printf("grayImg error\n");
		return MY_FAIL;
	}
	binaryImg.create(row, col, CV_8U);
	for (y = 0; y < row; y++)
	{
		uchar*data = grayImg.ptr<uchar>(y);
		uchar*dst = binaryImg.ptr<uchar>(y);
		for (x = 0; x < edge; )
		{
			dst[x] = 255 * ((th - data[x]) >> 31);
			dst[x + 1] = 255 * ((th - data[x + 1]) >> 31);
			dst[x + 2] = 255 * ((th - data[x + 2]) >> 31);
			dst[x + 3] = 255 * ((th - data[x + 3]) >> 31);
			x += 4;
		}
		for (x = edge; x < col; x++)
		{
			dst[x] = 255 * ((th - data[x]) >> 31);
		}
	}
	return MY_DONE;
}
//ustc_CalcHist
int ustc_CalcHist(Mat grayImg, int* hist, int hist_len) {
	int x, y;
	if (NULL == grayImg.data || NULL == hist)
	{
		printf("Image Load Failed\n");
		return MY_FAIL;
	}
	if (grayImg.channels() != 1)
	{
		printf("grayImg error\n");
		return MY_FAIL;
	}
	int row = grayImg.rows;
	int col = grayImg.cols;
	int edge = hist_len / 4;
	for (x = 0; x < edge; x += 4)
	{
		hist[x] = 0;
		hist[x + 1] = 0;
		hist[x + 2] = 0;
		hist[x + 3] = 0;
	}
	for (x = edge; x < hist_len; x++)
	{
		hist[x] = 0;
	}
	edge = col / 4;
	edge = 4 * edge;
	for (y = 0; y < row; y++)
	{
		uchar*data = grayImg.ptr<uchar>(y);
		for (x = 0; x < edge; x += 4)
		{
			hist[data[x]]++;
			hist[data[x + 1]]++;
			hist[data[x + 2]]++;
			hist[data[x + 3]]++;
		}
		for (x = edge; x < col; x++)
		{
			hist[data[x]]++;
		}
	}
	return MY_DONE;
}
//ustc_SubImgMatch_gray
int ustc_SubImgMatch_gray(Mat grayImg, Mat subImg, int* x, int* y)
{
	int i, j;
	int a, b;
	int diff = 0;
	int temp1, temp2;
	int srcRow = grayImg.rows;
	int srcCol = grayImg.cols;
	int subRow = subImg.rows;
	int subCol = subImg.cols;
	int rowEdge = srcRow - subRow + 1;
	int colEdge = srcCol - subCol + 1;
	int min = INT_MAX;
	int min_x, min_y;
	if (NULL == grayImg.data || NULL == subImg.data)
	{
		printf("image load fail\n");
		return MY_FAIL;
	}
	if (grayImg.channels() != 1 || subImg.channels() != 1)
	{
		printf("grayImg error\n");
		return MY_FAIL;
	}
	if (srcCol < subCol || srcRow < subRow)
	{
		printf("Imgsize error\n");
		return MY_FAIL;
	}
	Mat grayImg_INT;
	grayImg.convertTo(grayImg_INT, CV_32SC1);
	Mat subImg_INT;
	subImg.convertTo(subImg_INT, CV_32SC1);
	int divide = subCol / 8;
	__m256i c;
	for (i = 0; i < rowEdge; i++) 
	{
		for (j = 0; j < colEdge; j++)
		{
			c = _mm256_sub_epi32(c, c);
			Mat imageROI = grayImg_INT(Rect(j, i, subCol, subRow));
			for (a = 0; a < subRow; a++)
			{
				int *srcData = imageROI.ptr<int>(a); 
				int *samData = subImg_INT.ptr<int>(a); 
				for (b = 0; b < divide; b++)
				{
					__m256i l1 = _mm256_loadu_si256((__m256i*)(&srcData[b << 3]));
					__m256i l2 = _mm256_loadu_si256((__m256i*)(&samData[b << 3]));
					c = _mm256_add_epi32(c, _mm256_abs_epi32(_mm256_sub_epi32(l1, l2)));
				}
				temp1 = c.m256i_i32[0] + c.m256i_i32[1] + c.m256i_i32[2] + c.m256i_i32[3];
				temp2 = c.m256i_i32[4] + c.m256i_i32[5] + c.m256i_i32[6] + c.m256i_i32[7];
				diff = temp1 + temp2;
				for (b = divide * 8; b < subCol; b++)
				{
					diff += abs(srcData[b] - samData[b]);
				}
			}
			if (min > diff)
			{
				min = diff;
				min_x = j;
				min_y = i;
			}
		}
	}
	*x = min_x;
	*y = min_y;
	return MY_DONE;
}
//ustc_SubImgMatch_bgr
int ustc_SubImgMatch_bgr(Mat colorImg, Mat subImg, int* x, int* y)
{
	int i, j;
	int a, b;
	int diff;
	int min = INT_MAX, min_x, min_y;
	int srcRow = colorImg.rows;
	int srcCol = colorImg.cols;
	int samRow = subImg.rows;
	int samCol = subImg.cols;
	int rowEdge = srcRow - samRow + 1;
	int colEdge = srcCol - samCol + 1;
	int channel = colorImg.channels();
	int width = samCol*channel;
	int divide = width / 8;
	int temp1, temp2;
	if (NULL == colorImg.data || NULL == subImg.data)
	{
		printf("image load fail\n");
		return MY_FAIL;
	}
	if (colorImg.channels() != 3 || subImg.channels() != 3)
	{
		printf("colorImg error\n");
		return MY_FAIL;
	}
	if (srcCol < samCol || srcRow < samRow)
	{
		printf("Imgsize error\n");
		return MY_FAIL;
	}
	Mat colorImg_INT;
	colorImg.convertTo(colorImg_INT, CV_32SC3);
	Mat subImg_INT;
	subImg.convertTo(subImg_INT, CV_32SC3);
	__m256i c;
	for (i = 0; i < rowEdge; i++) 
	{
		for (j = 0; j < colEdge; j++)
		{
			c = _mm256_sub_epi32(c, c);
			Mat imageROI = colorImg_INT(Rect(j, i, samCol, samRow));
			for (a = 0; a < samRow; a++)
			{
				int *srcData = imageROI.ptr<int>(a);
				int *samData = subImg_INT.ptr<int>(a); 
				for (b = 0; b < divide; b++)
				{
					__m256i l1 = _mm256_loadu_si256((__m256i*)(&srcData[b << 3]));
					__m256i l2 = _mm256_loadu_si256((__m256i*)(&samData[b << 3]));

					c = _mm256_add_epi32(c, _mm256_abs_epi32(_mm256_sub_epi32(l1, l2)));
				}
				temp1 = c.m256i_i32[0] + c.m256i_i32[1] + c.m256i_i32[2] + c.m256i_i32[3];
				temp2 = c.m256i_i32[4] + c.m256i_i32[5] + c.m256i_i32[6] + c.m256i_i32[7];
				diff = temp1 + temp2;
				for (b = divide * 8; b < width; b++)
				{
					diff += abs(srcData[b] - samData[b]);
				}
				if (min > diff)
				{
					min = diff;
					min_x = j;
					min_y = i;
				}
			}
		}
	}
	*x = min_x;
	*y = min_y;
	return MY_DONE;
}
//ustc_SubImgMatch_corr
int ustc_SubImgMatch_corr(Mat grayImg, Mat subImg, int* x, int* y) 
{
	int srcRow = grayImg.rows;
	int srcCol = grayImg.cols;
	int subRow = subImg.rows;
	int subCol = subImg.cols;
	int rowEdge = srcRow - subRow + 1;
	int colEdge = srcCol - subCol + 1;
	int a, b, i, j;
	int temp1, temp2;
	int divide = subCol / 8;
	float s1, s2, s3;
	if (NULL == grayImg.data || NULL == subImg.data)
	{
		printf("image load fail\n");
		return MY_FAIL;
	}
	if (grayImg.channels() != 1 || subImg.channels() != 1)
	{
		printf("grayImg error\n");
		return MY_FAIL;
	}
	if (srcCol < subCol || srcRow < subRow)
	{
		printf("Imgsize error\n");
		return MY_FAIL;
	}
	Mat grayImg_FLT;
	grayImg.convertTo(grayImg_FLT, CV_32FC1);
	Mat subImg_FLT;
	subImg.convertTo(subImg_FLT, CV_32FC1);
	Mat searchImg(srcRow, srcCol, CV_32FC1);
	searchImg.setTo(FLT_MAX);
	__m256 sum1, sum2, sum3;
	for (i = 0; i < rowEdge; i++)
	{
		float *searchData = searchImg.ptr<float>(i);
		for (j = 0; j < colEdge; j++)
		{
			sum1 = _mm256_sub_ps(sum1, sum1);
			sum2 = _mm256_sub_ps(sum2, sum2);
			sum3 = _mm256_sub_ps(sum3, sum3);
			Mat imageROI = grayImg_FLT(Rect(j, i, subCol, subRow));
			for (a = 0; a < subRow; a++)
			{

				float *srcData = imageROI.ptr<float>(a);
				float *subData = subImg_FLT.ptr<float>(a);
				for (b = 0; b < divide; b++)
				{
					__m256 l1 = _mm256_loadu_ps((&srcData[b << 3]));
					__m256 l2 = _mm256_loadu_ps((&subData[b << 3]));
					sum1 = _mm256_add_ps(sum1, _mm256_mul_ps(l1, l2));
					sum2 = _mm256_add_ps(sum2, _mm256_mul_ps(l1, l1));
					sum3 = _mm256_add_ps(sum3, _mm256_mul_ps(l2, l2));
				}
				temp1 = sum1.m256_f32[0] + sum1.m256_f32[1] + sum1.m256_f32[2] + sum1.m256_f32[3];
				temp2 = sum1.m256_f32[4] + sum1.m256_f32[5] + sum1.m256_f32[6] + sum1.m256_f32[7];
				s1 = temp1 + temp2;
				temp1 = sum2.m256_f32[0] + sum2.m256_f32[1] + sum2.m256_f32[2] + sum2.m256_f32[3];
				temp2 = sum2.m256_f32[4] + sum2.m256_f32[5] + sum2.m256_f32[6] + sum2.m256_f32[7];
				s2 = temp1 + temp2;
				temp1 = sum3.m256_f32[0] + sum3.m256_f32[1] + sum3.m256_f32[2] + sum3.m256_f32[3];
				temp2 = sum3.m256_f32[4] + sum3.m256_f32[5] + sum3.m256_f32[6] + sum3.m256_f32[7];
				s3 = temp1 + temp2;
				for (b = divide * 8; b < subCol; b++)
				{
					s1 += srcData[b] * subData[b];
					s2 += srcData[b] * srcData[b];
					s3 += subData[b] * subData[b];
				}
			}
			searchData[j] = (s1 / (sqrt(s2)*sqrt(s3)));
		}
	}
	float max = 0;
	int max_x = 0, max_y = 0;
	for (i = 0; i < rowEdge; i++)
	{
		float *searchData = searchImg.ptr<float>(i);
		for (j = 0; j < colEdge; j++)
		{
			if (max <= searchData[j])
			{
				max = searchData[j];
				max_x = j;
				max_y = i;
			}
		}
	}
	*x = max_x;
	*y = max_y;
	return MY_DONE;
}
//ustc_SubImgMatch_angle
int ustc_SubImgMatch_angle(Mat grayImg, Mat subImg, int* x, int* y)
{
	int srcRow = grayImg.rows;
	int srcCol = grayImg.cols;
	int subRow = subImg.rows;
	int subCol = subImg.cols;
	int rowEdge = srcRow - subRow + 1;
	int colEdge = srcCol - subCol + 1;
	int a, b, i, j;
	int diff = 0;
	int divide = subCol / 8;
	Mat angleImg;
	Mat gradImg_x;
	Mat gradImg_y;
	Mat magImg;
	Mat angleImg_sub;
	if (NULL == grayImg.data || NULL == subImg.data)
	{
		printf("image load fail\n");
		return MY_FAIL;
	}
	if (grayImg.channels() != 1 || subImg.channels() != 1)
	{
		printf("grayImg error\n");
		return MY_FAIL;
	}
	if (srcCol < subCol || srcRow < subRow)
	{
		printf("Imgsize error\n");
		return MY_FAIL;
	}
	ustc_CalcGrad(grayImg, gradImg_x, gradImg_y);
	ustc_CalcAngleMag(gradImg_x, gradImg_y, angleImg, magImg);

	ustc_CalcGrad(subImg, gradImg_x, gradImg_y); 
	ustc_CalcAngleMag(gradImg_x, gradImg_y, angleImg_sub, magImg); 

	Mat angleImg_INT;
	Mat angleImg_sub_INT;
	angleImg.convertTo(angleImg_INT, CV_32S);
	angleImg_sub.convertTo(angleImg_sub_INT, CV_32S);
	for (i = 0; i < srcRow; i++)
	{
		int *srcData = angleImg_INT.ptr<int>(i);
		for (j = 0; j < srcCol; j++)
		{
			if (srcData[j] > 180)
				srcData[j] -= 360;
		}
	}
	for (i = 0; i < subRow; i++)
	{
		int *subData = angleImg_sub_INT.ptr<int>(i);
		for (j = 0; j < subCol; j++)
		{
			if (subData[j] > 180)
				subData[j] -= 360;
		}
	}
	int min = INT_MAX;
	int min_x, min_y;
	__m256i c;
	for (i = 0; i < rowEdge; i++) 
	{
		for (j = 0; j < colEdge; j++)
		{
			Mat imageROI = angleImg_INT(Rect(j, i, subCol, subRow)); 
			c = _mm256_sub_epi32(c, c);
			for (a = 0; a < subRow; a++)
			{
				int*srcData = imageROI.ptr<int>(a);  
				int *subData = angleImg_sub_INT.ptr<int>(a); 
				for (b = 0; b < divide; b++)
				{
					__m256i l1 = _mm256_loadu_si256((__m256i*)(&srcData[b << 3]));
					__m256i l2 = _mm256_loadu_si256((__m256i*)(&subData[b << 3]));
					c = _mm256_add_epi32(c, _mm256_abs_epi32(_mm256_sub_epi32(l1, l2)));
				}
				int temp1 = c.m256i_i32[0] + c.m256i_i32[1] + c.m256i_i32[2] + c.m256i_i32[3];
				int temp2 = c.m256i_i32[4] + c.m256i_i32[5] + c.m256i_i32[6] + c.m256i_i32[7];
				diff = temp1 + temp2;
				for (b = divide * 8; b < subCol; b++)
				{
					diff += abs(srcData[b] - subData[b]);
				}
			}
			if (min >= diff)
			{
				min = diff;
				min_x = j;
				min_y = i;
			}
		}
	}
	*x = min_x;
	*y = min_y;
	return MY_DONE;
}
//ustc_SubImgMatch_mag
int ustc_SubImgMatch_mag(Mat grayImg, Mat subImg, int* x, int* y) 
{
	int srcRow = grayImg.rows;
	int srcCol = grayImg.cols;
	int subRow = subImg.rows;
	int subCol = subImg.cols;
	int rowEdge = srcRow - subRow + 1;
	int colEdge = srcCol - subCol + 1;
	int a, b, i, j;
	int diff;
	int temp1, temp2;
	int divide = subCol / 8;
	Mat angleImg;
	Mat gradImg_x;
	Mat gradImg_y;
	Mat magImg;
	Mat magImg_sub;
	if (NULL == grayImg.data || NULL == subImg.data)
	{
		printf("image load fail\n");
		return MY_FAIL;
	}
	if (grayImg.channels() != 1 || subImg.channels() != 1)
	{
		printf("grayImg error\n");
		return MY_FAIL;
	}
	if (srcCol < subCol || srcRow < subRow)
	{
		printf("Imgsize error\n");
		return MY_FAIL;
	}
	ustc_CalcGrad(grayImg, gradImg_x, gradImg_y); 
	ustc_CalcAngleMag(gradImg_x, gradImg_y, angleImg, magImg);

	ustc_CalcGrad(subImg, gradImg_x, gradImg_y);
	ustc_CalcAngleMag(gradImg_x, gradImg_y, angleImg, magImg_sub);

	Mat magImg_INT;
	Mat magImg_sub_INT;
	magImg.convertTo(magImg_INT, CV_32S);
	magImg_sub.convertTo(magImg_sub_INT, CV_32S);
	Mat searchImg(srcRow, srcCol, CV_32SC1);
	searchImg.setTo(INT_MAX);
	__m256i c;
	for (i = 0; i < rowEdge; i++)
	{
		int *searchData = searchImg.ptr<int>(i);
		for (j = 0; j < colEdge; j++)
		{
			c = _mm256_sub_epi32(c, c);
			Mat imageROI = magImg_INT(Rect(j, i, subCol, subRow)); 
			for (a = 0; a < subRow; a++)
			{
				int *srcData = imageROI.ptr<int>(a);   
				int *subData = magImg_sub_INT.ptr<int>(a); 
				for (b = 0; b < divide; b++)
				{
					__m256i l1 = _mm256_loadu_si256((__m256i*)(&srcData[b << 3]));
					__m256i l2 = _mm256_loadu_si256((__m256i*)(&subData[b << 3]));
					c = _mm256_add_epi32(c, _mm256_abs_epi32(_mm256_sub_epi32(l1, l2)));
				}
				temp1 = c.m256i_i32[0] + c.m256i_i32[1] + c.m256i_i32[2] + c.m256i_i32[3];
				temp2 = c.m256i_i32[4] + c.m256i_i32[5] + c.m256i_i32[6] + c.m256i_i32[7];
				diff = temp1 + temp2;
				for (b = divide * 8; b < subCol; b++)
				{
					diff += abs(srcData[b] - subData[b]);
				}
			}
			searchData[j] = diff;
		}
	}

	int min = INT_MAX;
	int min_x, min_y;

	for (i = 0; i < rowEdge; i++)
	{
		int *searchData = searchImg.ptr<int>(i); 
		for (j = 0; j < colEdge; j++)
		{
			if (min >= searchData[j])
			{
				min = searchData[j];
				min_x = j;
				min_y = i;
			}
		}
	}
	*x = min_x;
	*y = min_y;
	return MY_DONE;
}
//ustc_SubImgMatch_hist
int ustc_SubImgMatch_hist(Mat grayImg, Mat subImg, int* x, int* y) 
{
	int srcHist[256];
	int subHist[256];
	int hist_len = 256;
	int srcRow = grayImg.rows;
	int srcCol = grayImg.cols;
	int subRow = subImg.rows;
	int subCol = subImg.cols;
	int rowEdge = srcRow - subRow + 1;
	int colEdge = srcCol - subCol + 1;
	int temp;
	int a, i, j;
	int temp1, temp2;
	int divide = hist_len / 8;
	int diff;
	if (NULL == grayImg.data || NULL == subImg.data)
	{
		printf("image load fail\n");
		return MY_FAIL;
	}
	if (grayImg.channels() != 1 || subImg.channels() != 1)
	{
		printf("grayImg error\n");
		return MY_FAIL;
	}
	if (srcCol < subCol || srcRow < subRow)
	{
		printf("Imgsize error\n");
		return MY_FAIL;
	}
	ustc_CalcHist(subImg, subHist, hist_len);
	Mat searchImg(srcRow, srcCol, CV_32SC1);
	searchImg.setTo(INT_MAX);
	__m256i c;
	for (i = 0; i < rowEdge; i++)
	{
		int *searchData = searchImg.ptr<int>(i);
		for (j = 0; j < colEdge; j++)
		{
			c = _mm256_sub_epi32(c, c);
			Mat imageROI = grayImg(Rect(j, i, subCol, subRow)); 
			ustc_CalcHist(imageROI, srcHist, hist_len);
			temp = 0;
			for (a = 0; a < divide; a++) 
			{
				__m256i l1 = _mm256_loadu_si256((__m256i*)(&srcHist[a << 3]));
				__m256i l2 = _mm256_loadu_si256((__m256i*)(&subHist[a << 3]));
				c = _mm256_add_epi32(c, _mm256_abs_epi32(_mm256_sub_epi32(l1, l2)));
			}
			temp1 = c.m256i_i32[0] + c.m256i_i32[1] + c.m256i_i32[2] + c.m256i_i32[3];
			temp2 = c.m256i_i32[4] + c.m256i_i32[5] + c.m256i_i32[6] + c.m256i_i32[7];
			diff = temp1 + temp2;
			for (a = divide * 8; a < subCol; a++)
			{
				diff += abs(srcHist[a] - subHist[a]);
			}
			searchData[j] = diff;
		}
	}

	int min = INT_MAX;
	int min_x, min_y;

	for (i = 0; i < rowEdge; i++)
	{
		int *searchData = searchImg.ptr<int>(i); 
		for (j = 0; j < colEdge; j++)
		{
			if (min >= searchData[j])
			{
				min = searchData[j];
				min_x = j;
				min_y = i;
			}
		}
	}
	*x = min_x;
	*y = min_y;
	return MY_DONE;
}
