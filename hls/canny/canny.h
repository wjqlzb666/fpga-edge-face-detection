#ifndef CANNY_H
#define CANNY_H

#include <ap_int.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>

#define IMG_W 640
#define IMG_H 480

// 双阈值(与软件基线 cv2.Canny(gray, 50, 150) 相同)
#define CANNY_LOW  50
#define CANNY_HIGH 150

// 梯度方向编码
#define DIR_H   0   // 梯度水平 -> 比较左右邻居
#define DIR_V   1   // 梯度垂直 -> 比较上下邻居
#define DIR_D1  2   // 对角 "\"  -> 比较左上 / 右下
#define DIR_D2  3   // 对角 "/"  -> 比较右上 / 左下

// tan(22.5°) * 2^15,和 OpenCV 的整数算法一致
#define TG22 13573

// 8 位灰度像素的 AXI-Stream 包:data(8 位) + user(帧起始) + last(帧结束)
typedef ap_axiu<8, 1, 1, 1> pixel_t;

void canny_top(hls::stream<pixel_t> &in, hls::stream<pixel_t> &out);

#endif
