#include <cstdio>
#include <cstdlib>
#include "haar.h"
// 用法: ./tb in.raw exp.raw   (in:640x480 灰度,exp:期望的 0/255 图,由 Python 定点参考模型生成)
static int run_case(const char *inf, const char *expf)
{
    static unsigned char img[IMG_W * IMG_H], exp_[IMG_W * IMG_H];
    printf("== %s\n", inf);
    FILE *f = fopen(inf, "rb"); if (!f || fread(img, 1, sizeof(img), f) != sizeof(img)) { printf("bad in: %s\n", inf); return 2; } fclose(f);
    f = fopen(expf, "rb"); if (!f || fread(exp_, 1, sizeof(exp_), f) != sizeof(exp_)) { printf("bad exp: %s\n", expf); return 2; } fclose(f);

    hls::stream<pixel_t> in, out;
    for (int rep = 0; rep < 2; rep++) {          // 连续跑两帧,检查帧间残留不影响结果
        for (int i = 0; i < IMG_W * IMG_H; i++) {
            pixel_t p; p.data = img[i]; p.keep = 1; p.strb = 1; p.user = (i == 0); p.last = (i == IMG_W * IMG_H - 1); p.id = 0; p.dest = 0;
            in.write(p);
        }
        haar_top(in, out);
        int n = 0, bad = 0, ones = 0, tl = 0;
        while (!out.empty()) {
            pixel_t o = out.read();
            if (n < IMG_W * IMG_H) { if ((int)o.data != (int)exp_[n]) bad++; if (o.data) ones++; }
            if (o.last) tl++;
            n++;
        }
        printf("frame %d: outputs=%d tlast=%d survivors=%d mismatches=%d\n", rep, n, tl, ones, bad);
        if (n != IMG_W * IMG_H || tl != 1 || bad != 0) { printf("TEST FAILED\n"); return 1; }
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc >= 3) { int r = run_case(argv[1], argv[2]); if (!r) printf("TEST PASSED\n"); return r; }
    int r = run_case("v2_in.raw", "v2_exp.raw");
    if (!r) r = run_case("noise_in.raw", "noise_exp.raw");
    if (!r) printf("TEST PASSED\n");
    return r;
}
