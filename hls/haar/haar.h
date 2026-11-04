#ifndef HAAR_H
#define HAAR_H
#include <ap_int.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>

#define IMG_W 640
#define IMG_H 480
#define WIN   24          // Haar 窗口 24x24

typedef ap_axiu<8, 1, 1, 1> pixel_t;

// 输入:640x480 灰度图(一帧)。输出:同尺寸的 0/255 图,
// (y,x) 处为 255 表示"以 (y,x) 为左上角的 24x24 窗口通过了前几级检测"
void haar_top(hls::stream<pixel_t> &in, hls::stream<pixel_t> &out);

#endif
