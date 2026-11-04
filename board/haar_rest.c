/* Haar 级联后半段(第 K 级起)的 C 实现:对 FPGA 筛出来的候选窗口继续逐级判断。
   算法与 OpenCV 一致:积分图 + 方差归一化(内框 22x22)+ stump 判断 + 级阈值。 */
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* 级联参数(由 Python 从 XML 展开成平面数组传入)
   stage_n[s]   : 第 s 级 stump 数        stage_th[s]: 第 s 级阈值
   wk_feat[i]   : stump i 用的特征号       wk_thr/wk_l0/wk_l1: stump 阈值/左叶/右叶
   f_nr[f]      : 特征 f 的矩形数          f_rect[f*12 + r*4 + {0..3}] = x,y,w,h;  f_w[f*3+r] = 权重 */
int haar_rest(const unsigned char *img, int w, int h,
              const int *ys, const int *xs, int n,
              int stage_from, int nstage,
              const int *stage_n, const float *stage_th,
              const int *wk_feat, const float *wk_thr, const float *wk_l0, const float *wk_l1,
              const int *f_nr, const int *f_rect, const float *f_w,
              int *out_idx)
{
    int W1 = w + 1;
    int *I = (int *)calloc((size_t)(w + 1) * (h + 1), sizeof(int));
    double *S = (double *)calloc((size_t)(w + 1) * (h + 1), sizeof(double));
    if (!I || !S) { free(I); free(S); return -1; }
    for (int y = 0; y < h; y++) {
        int rs = 0; double rq = 0;
        for (int x = 0; x < w; x++) {
            int p = img[y * w + x];
            rs += p; rq += (double)p * p;
            I[(y + 1) * W1 + x + 1] = I[y * W1 + x + 1] + rs;
            S[(y + 1) * W1 + x + 1] = S[y * W1 + x + 1] + rq;
        }
    }
    /* 每级第一个 stump 在 wk_* 数组里的起始位置(前 stage_from 级也要跳过) */
    int *base = (int *)malloc(sizeof(int) * (nstage + 1));
    base[0] = 0;
    for (int s = 0; s < nstage; s++) base[s + 1] = base[s] + stage_n[s];

    /* 预先把每个 stump 的矩形角点换算成"相对窗口左上角的一维偏移" */
    int nw = base[nstage];
    int *offs = (int *)malloc(sizeof(int) * (size_t)nw * 12);
    float *wts = (float *)malloc(sizeof(float) * (size_t)nw * 3);
    int *nr = (int *)malloc(sizeof(int) * (size_t)nw);
    for (int j = base[stage_from]; j < nw; j++) {
        int f = wk_feat[j];
        nr[j] = f_nr[f];
        for (int r = 0; r < 3; r++) {
            const int *q = f_rect + f * 12 + r * 4;
            int rx = q[0], ry = q[1], rw = q[2], rh = q[3];
            offs[j * 12 + r * 4 + 0] = ry * W1 + rx;               /* 左上 */
            offs[j * 12 + r * 4 + 1] = ry * W1 + rx + rw;          /* 右上 */
            offs[j * 12 + r * 4 + 2] = (ry + rh) * W1 + rx;        /* 左下 */
            offs[j * 12 + r * 4 + 3] = (ry + rh) * W1 + rx + rw;   /* 右下 */
            wts[j * 3 + r] = f_w[f * 3 + r];
        }
    }

    int cnt = 0;
    for (int k = 0; k < n; k++) {
        int y0 = ys[k], x0 = xs[k];
        const int *Ip = I + y0 * W1 + x0;
        const double *Sp = S + y0 * W1 + x0;
        double sm = (double)(Ip[23 * W1 + 23] - Ip[1 * W1 + 23] - Ip[23 * W1 + 1] + Ip[1 * W1 + 1]);
        double sq = Sp[23 * W1 + 23] - Sp[1 * W1 + 23] - Sp[23 * W1 + 1] + Sp[1 * W1 + 1];
        double var = 484.0 * sq - sm * sm;
        double nf = var > 0 ? sqrt(var) : 1.0;
        float inv = (float)(1.0 / nf);
        int pass = 1;
        for (int s = stage_from; s < nstage && pass; s++) {
            float tot = 0;
            for (int j = base[s]; j < base[s + 1]; j++) {
                const int *o = offs + j * 12;
                const float *w = wts + j * 3;
                float v = w[0] * (float)(Ip[o[3]] - Ip[o[1]] - Ip[o[2]] + Ip[o[0]])
                        + w[1] * (float)(Ip[o[7]] - Ip[o[5]] - Ip[o[6]] + Ip[o[4]]);
                if (nr[j] == 3)
                    v += w[2] * (float)(Ip[o[11]] - Ip[o[9]] - Ip[o[10]] + Ip[o[8]]);
                tot += (v * inv < wk_thr[j]) ? wk_l0[j] : wk_l1[j];
            }
            if (tot < stage_th[s]) pass = 0;
        }
        if (pass) out_idx[cnt++] = k;
    }
    free(I); free(S); free(base); free(offs); free(wts); free(nr);
    return cnt;
}
