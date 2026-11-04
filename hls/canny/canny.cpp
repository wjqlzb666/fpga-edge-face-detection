#include "canny.h"

// =====================================================================
//  流式 Canny 边缘检测(640x480, 8 位灰度, 每个时钟处理 1 个像素)
//
//  四级流水线,级与级之间用 FIFO 相连(DATAFLOW):
//    1) blur_stage : 3x3 高斯平滑  [1 2 1; 2 4 2; 1 2 1] / 16
//    2) sobel_stage: 3x3 Sobel 求梯度,幅值 |gx|+|gy|,方向量化成 4 个方向
//    3) nms_stage  : 非极大值抑制(把粗边缘变成细边缘)+ 双阈值分类
//    4) hyst_mid x5 + hyst_out: 6 级串联的局部滞后。每一级:弱边缘只要 8 邻域里有强边缘
//       就升级成强边缘;串联 6 级后,强边缘可沿弱边缘链最多"传播"6 个像素
//
//  说明:第 4 级是有限步数的局部滞后(每级只看 3x3 邻域,共 6 级),OpenCV 做的是整幅图
//  的连通域追踪,所以结果会略有不同(漏掉很长的弱边缘链),这是流式硬件实现的常见取舍。
// =====================================================================

#define TOTAL (IMG_W * IMG_H)
#define DELAY (IMG_W + 1)     // 3x3 窗口凑齐所需的延迟(像素数)

// ---------------------------------------------------------------------
// 第 1 级:高斯平滑
// ---------------------------------------------------------------------
static void blur_stage(hls::stream<pixel_t> &in, hls::stream<ap_uint<8> > &out)
{
    static unsigned char lb0[IMG_W];
    static unsigned char lb1[IMG_W];
    unsigned char win[3][3];
#pragma HLS ARRAY_PARTITION variable=win complete dim=0

    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 3; c++)
            win[r][c] = 0;

    int col = 0, ocol = 0, orow = 0;

    for (int i = 0; i < TOTAL + DELAY; i++) {
#pragma HLS PIPELINE II=1
#pragma HLS DEPENDENCE variable=lb0 inter false
#pragma HLS DEPENDENCE variable=lb1 inter false
        unsigned char pix = 0;
        if (i < TOTAL) {
            pixel_t p = in.read();
            pix = (unsigned char)p.data;
        }

        unsigned char l0 = lb0[col];
        unsigned char l1 = lb1[col];
        lb1[col] = l0;
        lb0[col] = pix;

        for (int r = 0; r < 3; r++) {
            win[r][0] = win[r][1];
            win[r][1] = win[r][2];
        }
        win[0][2] = l1;
        win[1][2] = l0;
        win[2][2] = pix;

        if (i >= DELAY) {
            int sum = (win[0][0] + 2 * win[0][1] + win[0][2])
                    + 2 * (win[1][0] + 2 * win[1][1] + win[1][2])
                    + (win[2][0] + 2 * win[2][1] + win[2][2]);
            int v = (sum + 8) >> 4;

            bool border = (orow == 0) || (orow == IMG_H - 1) ||
                          (ocol == 0) || (ocol == IMG_W - 1);
            // 边框一圈不做平滑,直接把原像素送下去
            out.write(ap_uint<8>(border ? (int)win[1][1] : v));

            if (ocol == IMG_W - 1) { ocol = 0; orow++; }
            else ocol++;
        }

        if (col == IMG_W - 1) col = 0;
        else col++;
    }
}

// ---------------------------------------------------------------------
// 第 2 级:Sobel 梯度 + 方向
//   输出 13 位:[12:11] 方向, [10:0] 幅值(最大 2040)
// ---------------------------------------------------------------------
static void sobel_stage(hls::stream<ap_uint<8> > &in, hls::stream<ap_uint<13> > &out)
{
    static unsigned char lb0[IMG_W];
    static unsigned char lb1[IMG_W];
    unsigned char win[3][3];
#pragma HLS ARRAY_PARTITION variable=win complete dim=0

    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 3; c++)
            win[r][c] = 0;

    int col = 0, ocol = 0, orow = 0;

    for (int i = 0; i < TOTAL + DELAY; i++) {
#pragma HLS PIPELINE II=1
#pragma HLS DEPENDENCE variable=lb0 inter false
#pragma HLS DEPENDENCE variable=lb1 inter false
        unsigned char pix = 0;
        if (i < TOTAL) {
            pix = (unsigned char)in.read();
        }

        unsigned char l0 = lb0[col];
        unsigned char l1 = lb1[col];
        lb1[col] = l0;
        lb0[col] = pix;

        for (int r = 0; r < 3; r++) {
            win[r][0] = win[r][1];
            win[r][1] = win[r][2];
        }
        win[0][2] = l1;
        win[1][2] = l0;
        win[2][2] = pix;

        if (i >= DELAY) {
            int gx = (win[0][2] + 2 * win[1][2] + win[2][2])
                   - (win[0][0] + 2 * win[1][0] + win[2][0]);
            int gy = (win[2][0] + 2 * win[2][1] + win[2][2])
                   - (win[0][0] + 2 * win[0][1] + win[0][2]);
            int ax = gx < 0 ? -gx : gx;
            int ay = gy < 0 ? -gy : gy;
            int mag = ax + ay;

            // 方向量化(整数算法,与 OpenCV 一致)
            int tg22x = ax * TG22;
            int ys = ay << 15;
            int dir;
            if (ys < tg22x) {
                dir = DIR_H;
            } else {
                int tg67x = tg22x + (ax << 16);
                if (ys > tg67x) dir = DIR_V;
                else dir = ((gx ^ gy) < 0) ? DIR_D2 : DIR_D1;
            }

            bool border = (orow == 0) || (orow == IMG_H - 1) ||
                          (ocol == 0) || (ocol == IMG_W - 1);
            unsigned int packed = border ? 0u : (((unsigned int)dir << 11) | (unsigned int)mag);
            out.write(ap_uint<13>(packed));

            if (ocol == IMG_W - 1) { ocol = 0; orow++; }
            else ocol++;
        }

        if (col == IMG_W - 1) col = 0;
        else col++;
    }
}

// ---------------------------------------------------------------------
// 第 3 级:非极大值抑制 + 双阈值分类
//   输出 2 位: 0 = 不是边缘, 1 = 弱边缘(候选), 2 = 强边缘
// ---------------------------------------------------------------------
static void nms_stage(hls::stream<ap_uint<13> > &in, hls::stream<ap_uint<2> > &out)
{
    static ap_uint<13> lb0[IMG_W];
    static ap_uint<13> lb1[IMG_W];
    ap_uint<13> win[3][3];
#pragma HLS ARRAY_PARTITION variable=win complete dim=0

    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 3; c++)
            win[r][c] = 0;

    int col = 0, ocol = 0, orow = 0;

    for (int i = 0; i < TOTAL + DELAY; i++) {
#pragma HLS PIPELINE II=1
#pragma HLS DEPENDENCE variable=lb0 inter false
#pragma HLS DEPENDENCE variable=lb1 inter false
        ap_uint<13> pix = 0;
        if (i < TOTAL) {
            pix = in.read();
        }

        ap_uint<13> l0 = lb0[col];
        ap_uint<13> l1 = lb1[col];
        lb1[col] = l0;
        lb0[col] = pix;

        for (int r = 0; r < 3; r++) {
            win[r][0] = win[r][1];
            win[r][1] = win[r][2];
        }
        win[0][2] = l1;
        win[1][2] = l0;
        win[2][2] = pix;

        if (i >= DELAY) {
            unsigned int c  = (unsigned int)win[1][1];
            int m   = (int)(c & 0x7FF);
            int dir = (int)(c >> 11);

            // 8 个邻居的幅值(只取低 11 位)
            int mUL = (int)((unsigned int)win[0][0] & 0x7FF);
            int mU  = (int)((unsigned int)win[0][1] & 0x7FF);
            int mUR = (int)((unsigned int)win[0][2] & 0x7FF);
            int mL  = (int)((unsigned int)win[1][0] & 0x7FF);
            int mR  = (int)((unsigned int)win[1][2] & 0x7FF);
            int mDL = (int)((unsigned int)win[2][0] & 0x7FF);
            int mD  = (int)((unsigned int)win[2][1] & 0x7FF);
            int mDR = (int)((unsigned int)win[2][2] & 0x7FF);

            bool keep = false;
            if (dir == DIR_H)       keep = (m > mL)  && (m >= mR);
            else if (dir == DIR_V)  keep = (m > mU)  && (m >= mD);
            else if (dir == DIR_D1) keep = (m > mUL) && (m > mDR);
            else                    keep = (m > mUR) && (m > mDL);

            bool border = (orow == 0) || (orow == IMG_H - 1) ||
                          (ocol == 0) || (ocol == IMG_W - 1);

            int cls = 0;
            if (!border && m > CANNY_LOW && keep)
                cls = (m > CANNY_HIGH) ? 2 : 1;
            out.write(ap_uint<2>(cls));

            if (ocol == IMG_W - 1) { ocol = 0; orow++; }
            else ocol++;
        }

        if (col == IMG_W - 1) col = 0;
        else col++;
    }
}

// ---------------------------------------------------------------------
// 第 4 级(中间级):局部滞后一步 —— 弱边缘邻域里有强边缘就升级成强边缘
//   输入输出都是 2 位类别: 0 = 非边缘, 1 = 弱, 2 = 强
//   用模板参数 ID 让每个实例有自己独立的行缓存(static)
// ---------------------------------------------------------------------
template<int ID>
static void hyst_mid(hls::stream<ap_uint<2> > &in, hls::stream<ap_uint<2> > &out)
{
    static unsigned char lb0[IMG_W];
    static unsigned char lb1[IMG_W];
    unsigned char win[3][3];
#pragma HLS ARRAY_PARTITION variable=win complete dim=0

    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 3; c++)
            win[r][c] = 0;

    int col = 0;

    for (int i = 0; i < TOTAL + DELAY; i++) {
#pragma HLS PIPELINE II=1
#pragma HLS DEPENDENCE variable=lb0 inter false
#pragma HLS DEPENDENCE variable=lb1 inter false
        unsigned char pix = 0;
        if (i < TOTAL) {
            pix = (unsigned char)in.read();
        }

        unsigned char l0 = lb0[col];
        unsigned char l1 = lb1[col];
        lb1[col] = l0;
        lb0[col] = pix;

        for (int r = 0; r < 3; r++) {
            win[r][0] = win[r][1];
            win[r][1] = win[r][2];
        }
        win[0][2] = l1;
        win[1][2] = l0;
        win[2][2] = pix;

        if (i >= DELAY) {
            unsigned char c = win[1][1];
            bool nb_strong = (win[0][0] == 2) || (win[0][1] == 2) || (win[0][2] == 2) ||
                             (win[1][0] == 2) ||                     (win[1][2] == 2) ||
                             (win[2][0] == 2) || (win[2][1] == 2) || (win[2][2] == 2);
            unsigned char r = (c == 1 && nb_strong) ? 2 : c;
            out.write(ap_uint<2>(r));
        }

        if (col == IMG_W - 1) col = 0;
        else col++;
    }
}

// ---------------------------------------------------------------------
// 第 4 级(最后一级):再做一步滞后,并输出 AXI-Stream
//   强边缘 -> 255;其余 0
// ---------------------------------------------------------------------
static void hyst_out(hls::stream<ap_uint<2> > &in, hls::stream<pixel_t> &out)
{
    static unsigned char lb0[IMG_W];
    static unsigned char lb1[IMG_W];
    unsigned char win[3][3];
#pragma HLS ARRAY_PARTITION variable=win complete dim=0

    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 3; c++)
            win[r][c] = 0;

    int col = 0, ocol = 0, orow = 0;

    for (int i = 0; i < TOTAL + DELAY; i++) {
#pragma HLS PIPELINE II=1
#pragma HLS DEPENDENCE variable=lb0 inter false
#pragma HLS DEPENDENCE variable=lb1 inter false
        unsigned char pix = 0;
        if (i < TOTAL) {
            pix = (unsigned char)in.read();
        }

        unsigned char l0 = lb0[col];
        unsigned char l1 = lb1[col];
        lb1[col] = l0;
        lb0[col] = pix;

        for (int r = 0; r < 3; r++) {
            win[r][0] = win[r][1];
            win[r][1] = win[r][2];
        }
        win[0][2] = l1;
        win[1][2] = l0;
        win[2][2] = pix;

        if (i >= DELAY) {
            unsigned char c = win[1][1];
            bool nb_strong = (win[0][0] == 2) || (win[0][1] == 2) || (win[0][2] == 2) ||
                             (win[1][0] == 2) ||                     (win[1][2] == 2) ||
                             (win[2][0] == 2) || (win[2][1] == 2) || (win[2][2] == 2);
            bool edge = (c == 2) || (c == 1 && nb_strong);

            bool border = (orow == 0) || (orow == IMG_H - 1) ||
                          (ocol == 0) || (ocol == IMG_W - 1);

            pixel_t o;
            o.data = (edge && !border) ? 255 : 0;
            o.keep = 1;
            o.strb = 1;
            o.user = (orow == 0 && ocol == 0) ? 1 : 0;
            // 只在整帧最后一个像素拉高 TLAST,配合 AXI DMA(一次传输 = 一帧)
            o.last = (orow == IMG_H - 1 && ocol == IMG_W - 1) ? 1 : 0;
            o.id = 0;
            o.dest = 0;
            out.write(o);

            if (ocol == IMG_W - 1) { ocol = 0; orow++; }
            else ocol++;
        }

        if (col == IMG_W - 1) col = 0;
        else col++;
    }
}

// ---------------------------------------------------------------------
// 顶层
// ---------------------------------------------------------------------
void canny_top(hls::stream<pixel_t> &in, hls::stream<pixel_t> &out)
{
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS DATAFLOW

    hls::stream<ap_uint<8> >  s_blur;
    hls::stream<ap_uint<13> > s_grad;
    hls::stream<ap_uint<2> >  s_cls0, s_cls1, s_cls2, s_cls3, s_cls4, s_cls5;
#pragma HLS STREAM variable=s_blur depth=4
#pragma HLS STREAM variable=s_grad depth=4
#pragma HLS STREAM variable=s_cls0 depth=4
#pragma HLS STREAM variable=s_cls1 depth=4
#pragma HLS STREAM variable=s_cls2 depth=4
#pragma HLS STREAM variable=s_cls3 depth=4
#pragma HLS STREAM variable=s_cls4 depth=4
#pragma HLS STREAM variable=s_cls5 depth=4

    blur_stage(in, s_blur);
    sobel_stage(s_blur, s_grad);
    nms_stage(s_grad, s_cls0);
    hyst_mid<0>(s_cls0, s_cls1);
    hyst_mid<1>(s_cls1, s_cls2);
    hyst_mid<2>(s_cls2, s_cls3);
    hyst_mid<3>(s_cls3, s_cls4);
    hyst_mid<4>(s_cls4, s_cls5);
    hyst_out(s_cls5, out);
}
