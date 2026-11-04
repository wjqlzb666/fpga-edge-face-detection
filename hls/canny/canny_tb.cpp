#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "canny.h"

// ---------------- 参考实现(软件 golden model,算法与硬件逐级一致) ----------------
static void ref_canny(const unsigned char *src, unsigned char *dst)
{
    static unsigned char blur[IMG_W * IMG_H];
    static int mag[IMG_W * IMG_H];
    static int dirv[IMG_W * IMG_H];
    static unsigned char cls[IMG_W * IMG_H];

    #define IDX(y, x) ((y) * IMG_W + (x))
    for (int y = 0; y < IMG_H; y++)
        for (int x = 0; x < IMG_W; x++) {
            if (y == 0 || y == IMG_H - 1 || x == 0 || x == IMG_W - 1) {
                blur[IDX(y, x)] = src[IDX(y, x)];
                continue;
            }
            #define S(dy, dx) src[IDX(y + (dy), x + (dx))]
            int sum = (S(-1,-1) + 2*S(-1,0) + S(-1,1)) + 2*(S(0,-1) + 2*S(0,0) + S(0,1)) + (S(1,-1) + 2*S(1,0) + S(1,1));
            #undef S
            blur[IDX(y, x)] = (unsigned char)((sum + 8) >> 4);
        }

    for (int y = 0; y < IMG_H; y++)
        for (int x = 0; x < IMG_W; x++) {
            mag[IDX(y, x)] = 0; dirv[IDX(y, x)] = 0;
            if (y == 0 || y == IMG_H - 1 || x == 0 || x == IMG_W - 1) continue;
            #define B(dy, dx) blur[IDX(y + (dy), x + (dx))]
            int gx = (B(-1,1) + 2*B(0,1) + B(1,1)) - (B(-1,-1) + 2*B(0,-1) + B(1,-1));
            int gy = (B(1,-1) + 2*B(1,0) + B(1,1)) - (B(-1,-1) + 2*B(-1,0) + B(-1,1));
            #undef B
            int ax = abs(gx), ay = abs(gy);
            mag[IDX(y, x)] = ax + ay;
            int tg22x = ax * TG22, ys = ay << 15, d;
            if (ys < tg22x) d = DIR_H;
            else {
                int tg67x = tg22x + (ax << 16);
                if (ys > tg67x) d = DIR_V;
                else d = ((gx ^ gy) < 0) ? DIR_D2 : DIR_D1;
            }
            dirv[IDX(y, x)] = d;
        }

    for (int y = 0; y < IMG_H; y++)
        for (int x = 0; x < IMG_W; x++) {
            cls[IDX(y, x)] = 0;
            if (y == 0 || y == IMG_H - 1 || x == 0 || x == IMG_W - 1) continue;
            int m = mag[IDX(y, x)];
            if (m <= CANNY_LOW) continue;
            bool keep;
            switch (dirv[IDX(y, x)]) {
                case DIR_H:  keep = m > mag[IDX(y, x-1)]   && m >= mag[IDX(y, x+1)];   break;
                case DIR_V:  keep = m > mag[IDX(y-1, x)]   && m >= mag[IDX(y+1, x)];   break;
                case DIR_D1: keep = m > mag[IDX(y-1, x-1)] && m > mag[IDX(y+1, x+1)];  break;
                default:     keep = m > mag[IDX(y-1, x+1)] && m > mag[IDX(y+1, x-1)];  break;
            }
            if (keep) cls[IDX(y, x)] = (m > CANNY_HIGH) ? 2 : 1;
        }

    // 6 级局部滞后(与硬件的 5 个 hyst_mid + 1 个 hyst_out 对应)
    static unsigned char cur[IMG_W * IMG_H];
    memcpy(cur, cls, sizeof(cur));
    for (int it = 0; it < 6; it++) {
        static unsigned char nxt[IMG_W * IMG_H];
        memcpy(nxt, cur, sizeof(nxt));
        for (int y = 1; y < IMG_H - 1; y++)
            for (int x = 1; x < IMG_W - 1; x++) {
                if (cur[IDX(y, x)] != 1) continue;
                bool nb = false;
                for (int dy = -1; dy <= 1; dy++)
                    for (int dx = -1; dx <= 1; dx++)
                        if ((dy || dx) && cur[IDX(y + dy, x + dx)] == 2) nb = true;
                if (nb) nxt[IDX(y, x)] = 2;
            }
        memcpy(cur, nxt, sizeof(cur));
    }
    for (int y = 0; y < IMG_H; y++)
        for (int x = 0; x < IMG_W; x++) {
            bool border = (y == 0 || y == IMG_H - 1 || x == 0 || x == IMG_W - 1);
            dst[IDX(y, x)] = (!border && cur[IDX(y, x)] == 2) ? 255 : 0;
        }
    #undef IDX
}

int main(int argc, char **argv)
{
    static unsigned char src[IMG_W * IMG_H];
    static unsigned char ref[IMG_W * IMG_H];
    static unsigned char got[IMG_W * IMG_H];

    if (argc >= 3) {
        // 额外模式:从原始灰度文件读入,结果写到文件(用于和 OpenCV 比较)
        FILE *f = fopen(argv[1], "rb");
        if (!f || fread(src, 1, IMG_W * IMG_H, f) != (size_t)(IMG_W * IMG_H)) { printf("read fail\n"); return 2; }
        fclose(f);
    } else {
        // 测试图:渐变 + 方块 + 圆 + 随机噪声
        srand(1);
        for (int y = 0; y < IMG_H; y++)
            for (int x = 0; x < IMG_W; x++) {
                int v = (x * 200) / IMG_W + 20;
                if (x > 200 && x < 400 && y > 150 && y < 330) v = 230;
                int dx = x - 520, dy = y - 120;
                if (dx * dx + dy * dy < 70 * 70) v = 40;
                v += (rand() % 15) - 7;
                if (v < 0) v = 0;
                if (v > 255) v = 255;
                src[y * IMG_W + x] = (unsigned char)v;
            }
    }

    ref_canny(src, ref);

    hls::stream<pixel_t> s_in, s_out;
    for (int i = 0; i < IMG_W * IMG_H; i++) {
        pixel_t p;
        p.data = src[i];
        p.keep = 1; p.strb = 1; p.id = 0; p.dest = 0;
        p.user = (i == 0);
        p.last = ((i % IMG_W) == IMG_W - 1);
        s_in.write(p);
    }

    canny_top(s_in, s_out);

    int n = 0, last_cnt = 0;
    while (!s_out.empty() && n < IMG_W * IMG_H) {
        pixel_t o = s_out.read();
        got[n++] = (unsigned char)o.data;
        if (o.last) last_cnt++;
    }

    int errors = 0, edges = 0;
    for (int i = 0; i < IMG_W * IMG_H; i++) {
        if (got[i] != ref[i]) errors++;
        if (ref[i]) edges++;
    }

    printf("输出像素数: %d (应为 %d), TLAST 个数: %d (应为 1)\n", n, IMG_W * IMG_H, last_cnt);
    printf("参考图中的边缘像素数: %d\n", edges);
    printf("不一致的像素数: %d\n", errors);

    if (argc >= 3) {
        FILE *f = fopen(argv[2], "wb");
        fwrite(got, 1, IMG_W * IMG_H, f);
        fclose(f);
    }

    if (n == IMG_W * IMG_H && errors == 0 && last_cnt == 1 && edges > 0) {
        printf("TEST PASSED\n");
        return 0;
    }
    printf("TEST FAILED\n");
    return 1;
}
