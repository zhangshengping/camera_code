/********************************************************************
 *
 *	文件名：	image.h
 *	功能  ：	计算图像中线，返回图像中线与车夹角。
 *	作者  ：	赵志明
 *	日期  ：	2013/11/20
 *	版本  ：	V1.0
 *
 *
 *  说明：本接口调用函数image_average_offset获取每幅图像偏差值。使用方式为：image_average_offset(image , threshold)
 *	其中image为图像的二维数组，threshold为图像二值化设定的阈值。
 *******************************************************************/
#ifndef _IMAGE_H_
#define _IMAGE_H_
#define image_row	120									//图像行数
#define image_col	240									//图像列数
#include "common.h"
#include "stdint.h"

/***********************************************************************
 *
 函数名		:	image_binary
 参数		:	**image:图像二维数组.threshold。	:图像二值化阈值。
 输出		:	无
 操作结果	:	图像二值化
 *
 ************************************************************************/

void image_binary(int8_t   (*image)[image_col],int8_t threshold);

/***********************************************************************
 *
 函数名		:	image_left_offset
 参数		:	image:图像二维数组行指针 n:行号
 输出		:	无
 操作结果	:	提取一行左边偏移值。
 *
 ************************************************************************/

int8_t image_left_offset(int8_t (*image)[image_col] , int8_t n);

/***********************************************************************
 *
 函数名		:	image_right_offset
 参数		:	image:图像二维数组行指针 n:行号.
 输出		:	无
 操作结果	:	提取一行右边偏移值。
 *
 ************************************************************************/

int8_t image_right_offset(int8_t (*image)[image_col] , int8_t n);


 /***********************************************************************
 *
 函数名		:	image_average_offset
 参数		:	图像二维数组。
 输出		:	每行平均偏差。
 操作结果	:	提取一行右边偏移值。
 *
 ************************************************************************/

 int8_t image_average_offset(int8_t (*image)[image_col] , int8_t threshold);

#endif
