"""Canny 上板测试:PL(FPGA)与软件对比(耗时 + 边缘相似度)

用法(在板子的 Jupyter 里):
    !python3 bench_canny.py canny.bit p2_c.jpg p1_c.jpg
第一个参数是 .bit 文件,后面是一张或多张图片(会转灰度并缩放到 640x480)。
"""
import sys, os, time
import numpy as np
import cv2

W, H = 640, 480
LOW, HIGH = 50, 150
N_ITER = 50      # 每轮迭代次数
N_ROUND = 3      # 测几轮,取平均耗时最小的一轮(排除偶发抖动)
MARGIN = 3       # 比较边缘时忽略边框一圈


def sw_canny_blur(gray):
    """软件基线 A:高斯平滑 + Canny(算法流程与硬件一致)"""
    return cv2.Canny(cv2.GaussianBlur(gray, (3, 3), 0), LOW, HIGH)


def sw_canny_only(gray):
    """软件基线 B:只有 Canny(之前测过的 27 ms 那个)"""
    return cv2.Canny(gray, LOW, HIGH)


class PLCanny:
    def __init__(self, bit):
        from pynq import Overlay, allocate
        self.ol = Overlay(bit)
        self.dma = self.ol.axi_dma_0
        self.inb = allocate(shape=(H * W,), dtype=np.uint8)
        self.outb = allocate(shape=(H * W,), dtype=np.uint8)

    def run(self, gray):
        self.inb[:] = gray.reshape(-1)
        self.dma.recvchannel.transfer(self.outb)   # 先开接收,再发送
        self.dma.sendchannel.transfer(self.inb)
        self.dma.sendchannel.wait()
        self.dma.recvchannel.wait()
        return np.array(self.outb).reshape(H, W)

    def close(self):
        try:
            self.inb.freebuffer(); self.outb.freebuffer()
        except Exception:
            pass


def timeit(fn, n=N_ITER, rounds=N_ROUND):
    best = None
    for _ in range(rounds):
        t0 = time.perf_counter()
        for _ in range(n):
            fn()
        ms = (time.perf_counter() - t0) * 1000.0 / n
        best = ms if best is None else min(best, ms)
    return best


def similarity(hw, sw):
    m = slice(MARGIN, -MARGIN)
    a = hw[m, m] > 0
    b = sw[m, m] > 0
    tp = int((a & b).sum())
    p = tp / max(int(a.sum()), 1)
    r = tp / max(int(b.sum()), 1)
    f1 = 2 * p * r / max(p + r, 1e-9)
    return int(a.sum()), int(b.sum()), p, r, f1


def load_gray(path):
    img = cv2.imread(path)
    if img is None:
        print("读不到图片:", path); return None
    g = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    if g.shape != (H, W):
        print("  提示:%s 尺寸是 %dx%d,已缩放到 %dx%d(可能有拉伸)" % (os.path.basename(path), g.shape[1], g.shape[0], W, H))
        g = cv2.resize(g, (W, H))
    return np.ascontiguousarray(g)


def main():
    if len(sys.argv) < 3:
        print(__doc__); return
    bit = sys.argv[1]
    paths = sys.argv[2:]
    print("OpenCV %s;Canny 用 PL(FPGA);%dx%d;阈值 %d/%d" % (cv2.__version__, W, H, LOW, HIGH))
    pl = PLCanny(bit)
    rows = []
    for p in paths:
        g = load_gray(p)
        if g is None:
            continue
        name = os.path.splitext(os.path.basename(p))[0]
        print("\n===== %s =====" % os.path.basename(p))

        hw = pl.run(g)
        sw = sw_canny_blur(g)
        n_hw, n_sw, P, R, F1 = similarity(hw, sw)
        print("边缘像素数:PL %d,软件 %d" % (n_hw, n_sw))
        print("与软件 Canny 的相似度:准确率 %.3f,找回率 %.3f,F1 %.3f" % (P, R, F1))

        t_pl = timeit(lambda: pl.run(g))
        t_a = timeit(lambda: sw_canny_blur(g))
        t_b = timeit(lambda: sw_canny_only(g))
        print("%-34s %9s %8s" % ("方式", "耗时 ms/帧", "FPS"))
        print("%-34s %9.2f %8.1f" % ("软件 高斯平滑+Canny(同流程)", t_a, 1000.0 / t_a))
        print("%-34s %9.2f %8.1f" % ("软件 只有Canny(旧基线)", t_b, 1000.0 / t_b))
        print("%-34s %9.2f %8.1f" % ("PL(FPGA)Canny,含DMA与Python", t_pl, 1000.0 / t_pl))
        print("PL 相对 软件(同流程) 加速 %.1f 倍" % (t_a / t_pl))

        cv2.imwrite("out_canny_pl_%s.png" % name, hw)
        cv2.imwrite("out_canny_sw_%s.png" % name, sw)
        rows.append((os.path.basename(p), t_a, t_b, t_pl, P, R, F1))

    pl.close()
    if rows:
        print("\n==== 汇总 ====")
        print("%-14s %9s %9s %9s %7s %7s %7s %7s" % ("图片", "SW同流程", "SW仅Canny", "PL", "加速", "准确率", "找回率", "F1"))
        for n, a, b, t, P, R, F1 in rows:
            print("%-14s %9.2f %9.2f %9.2f %6.1fx %7.3f %7.3f %7.3f" % (n, a, b, t, a / t, P, R, F1))


if __name__ == "__main__":
    main()
