"""Haar 前 4 级(FPGA)上板验证:和电脑上算好的标准答案逐像素对比,并测耗时

用法(在板子的 Jupyter 里,先把 haar.bit/haar.hwh/v2_in.raw/v2_exp.raw/noise_in.raw/noise_exp.raw 都传到同一目录):
    !python3 check_haar.py haar.bit
"""
import sys, time
import numpy as np

W, H = 640, 480


class PLHaar:
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


def main():
    if len(sys.argv) < 2:
        print(__doc__); return
    pl = PLHaar(sys.argv[1])
    ok_all = True
    for name in ("v2", "noise"):
        img = np.fromfile(name + "_in.raw", np.uint8).reshape(H, W)
        exp = np.fromfile(name + "_exp.raw", np.uint8).reshape(H, W)
        out = pl.run(img)
        bad = int((out != exp).sum())
        print("%-6s 期望通过窗口 %d,板子输出 %d,不一致 %d" % (name, int((exp > 0).sum()), int((out > 0).sum()), bad))
        ok_all &= (bad == 0)
        # 再跑一次,检查连续两帧结果相同(帧间无残留)
        out2 = pl.run(img)
        same = bool((out2 == out).all())
        print("       连续两帧结果一致: %s" % same)
        ok_all &= same
    img = np.fromfile("v2_in.raw", np.uint8).reshape(H, W)
    best = None
    for _ in range(3):
        t0 = time.perf_counter()
        for _ in range(50):
            pl.run(img)
        ms = (time.perf_counter() - t0) * 1000.0 / 50
        best = ms if best is None else min(best, ms)
    print("每帧耗时(含 Python+DMA): %.2f ms" % best)
    print("结果:", "全部通过" if ok_all else "有不一致,请把以上输出发给我")


main()
