#include "haar.h"
#include "haar_eval.h"

// =====================================================================
//  Haar 人脸检测前 N 级(早期淘汰)——流式实现,每个时钟吃 1 个像素
//
//  1) 积分图:边收像素边累加,积分值只保留低位(模 2^18 / 2^25 回绕),
//     因为窗口内任意矩形和 < 2^18,"差值"在回绕下依然正确
//  2) 25 行积分图行缓存 + 25x25 窗口寄存器,窗口内所有角点同时可读
//  3) 窗口方差归一化:nf = sqrt(484*Σp² - (Σp)²),取 8 位小数的定点数
//  4) 前 N 级的全部 stump 并行求值,全部通过则输出 255
// =====================================================================

#define TOTAL (IMG_W * IMG_H)
#define DELAY (23 * IMG_W + 23)     // 窗口右下角像素到达时,窗口左上角相对位置

static ap_uint<26> isqrt50(ap_uint<50> rad)
{
    ap_uint<26> root = 0;
    ap_uint<52> rem = 0;
    for (int i = 24; i >= 0; i--) {
#pragma HLS UNROLL
        rem = (rem << 2) | ((rad >> (2 * i)) & 3);
        ap_uint<52> trial = (ap_uint<52>(root) << 2) | 1;
        if (rem >= trial) { rem = rem - trial; root = (root << 1) | 1; }
        else              { root = root << 1; }
    }
    return root;
}

void haar_top(hls::stream<pixel_t> &in, hls::stream<pixel_t> &out)
{
#pragma HLS INTERFACE axis port=in
#pragma HLS INTERFACE axis port=out
#pragma HLS INTERFACE ap_ctrl_none port=return

    // 积分图行缓存:lb[k][x] = 第 (y-1-k) 行的积分值
    static ap_uint<18> lb[24][IMG_W];
    // 平方积分图行缓存:lq[k][x] = 第 (y-1-k) 行(k=0..22)
    static ap_uint<25> lq[23][IMG_W];
#pragma HLS ARRAY_PARTITION variable=lb complete dim=1
#pragma HLS ARRAY_PARTITION variable=lq complete dim=1
#pragma HLS RESOURCE variable=lb core=RAM_2P_BRAM
#pragma HLS RESOURCE variable=lq core=RAM_2P_BRAM

    ap_uint<18> win[25][25];
#pragma HLS ARRAY_PARTITION variable=win complete dim=0
    // 平方积分窗口:只需要第 1 行和第 23 行(相对窗口),每行 24 列
    ap_uint<25> qwin[2][24];
#pragma HLS ARRAY_PARTITION variable=qwin complete dim=0

    for (int r = 0; r < 25; r++)
        for (int c = 0; c < 25; c++) win[r][c] = 0;
    for (int r = 0; r < 2; r++)
        for (int c = 0; c < 24; c++) qwin[r][c] = 0;

    int col = 0, irow = 0;             // 输入列、输入行
    ap_uint<18> rs = 0;                // 当前行累加(模 2^18)
    ap_uint<25> rq = 0;
    int ocol = 0, orow = 0;            // 输出(窗口左上角)位置

    for (int i = 0; i < TOTAL + DELAY; i++) {
#pragma HLS PIPELINE II=1
#pragma HLS DEPENDENCE variable=lb inter false
#pragma HLS DEPENDENCE variable=lq inter false
        ap_uint<8> pix = 0;
        if (i < TOTAL) {
            pixel_t p = in.read();
            pix = p.data;
        }
        if (col == 0) { rs = 0; rq = 0; }
        ap_uint<18> rs_n = rs + pix;
        ap_uint<25> rq_n = rq + (ap_uint<25>)(pix * pix);
        rs = rs_n; rq = rq_n;

        // 读出上面各行的积分值(读旧值)
        ap_uint<18> old[24];
#pragma HLS ARRAY_PARTITION variable=old complete
        for (int k = 0; k < 24; k++) {
#pragma HLS UNROLL
            old[k] = lb[k][col];
        }
        ap_uint<25> oldq[23];
#pragma HLS ARRAY_PARTITION variable=oldq complete
        for (int k = 0; k < 23; k++) {
#pragma HLS UNROLL
            oldq[k] = lq[k][col];
        }
        // 第 0 行没有"上一行",缓存里是上一帧的残留,必须当 0 处理
        ap_uint<18> base  = (irow == 0) ? ap_uint<18>(0) : old[0];
        ap_uint<25> baseq = (irow == 0) ? ap_uint<25>(0) : oldq[0];
        ap_uint<18> cur  = base + rs_n;              // 本行积分值 II(y,x)
        ap_uint<25> curq = baseq + rq_n;

        // 行缓存推进:lb[0] <- cur, lb[k] <- old[k-1]
        lb[0][col] = cur;
        for (int k = 1; k < 24; k++) {
#pragma HLS UNROLL
            lb[k][col] = old[k - 1];
        }
        lq[0][col] = curq;
        for (int k = 1; k < 23; k++) {
#pragma HLS UNROLL
            lq[k][col] = oldq[k - 1];
        }

        // 窗口左移一列,并装入新列:第 r 行(r=0..24)= 第 y-24+r 行
        for (int r = 0; r < 25; r++) {
#pragma HLS UNROLL
            for (int c = 0; c < 24; c++) {
#pragma HLS UNROLL
                win[r][c] = win[r][c + 1];
            }
        }
        win[24][24] = cur;
        for (int r = 0; r < 24; r++) {
#pragma HLS UNROLL
            win[r][24] = old[23 - r];
        }
        // 平方积分:窗口第 1 行 = 第 y-23 行 = oldq[22],窗口第 23 行 = 第 y-1 行 = oldq[0]
        for (int c = 0; c < 23; c++) {
#pragma HLS UNROLL
            qwin[0][c] = qwin[0][c + 1];
            qwin[1][c] = qwin[1][c + 1];
        }
        qwin[0][23] = oldq[22];
        qwin[1][23] = oldq[0];

        if (i >= DELAY) {
            bool valid = (ocol <= IMG_W - WIN) && (orow <= IMG_H - WIN);
            bool top = (orow == 0), left = (ocol == 0);

            // 内框 (1,1,22,22) 的像素和与平方和
            ap_uint<18> s = (ap_uint<18>)(win[23][23] - win[1][23] - win[23][1] + win[1][1]);
            // qwin[r][t]:t=23 对应窗口列 24(最新列),t=0 对应窗口列 1
            ap_uint<25> q = (ap_uint<25>)(qwin[1][22] - qwin[0][22] - qwin[1][0] + qwin[0][0]);
            ap_int<36> nf2 = (ap_int<36>)q * 484 - (ap_int<36>)s * (ap_int<36>)s;
            ap_uint<26> nfq;
            if (nf2 > 0) nfq = isqrt50(((ap_uint<50>)nf2) << 16);
            else         nfq = 256;

            bool pass = eval_stages(win, nfq, top, left);

            pixel_t o;
            o.data = (valid && pass) ? 255 : 0;
            o.keep = 1;
            o.strb = 1;
            o.user = (orow == 0 && ocol == 0) ? 1 : 0;
            o.last = (orow == IMG_H - 1 && ocol == IMG_W - 1) ? 1 : 0;
            o.id = 0;
            o.dest = 0;
            out.write(o);

            if (ocol == IMG_W - 1) { ocol = 0; orow++; }
            else ocol++;
        }

        if (col == IMG_W - 1) { col = 0; irow++; }
        else col++;
    }
}
