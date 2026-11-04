"""FPGA+ARM 混合人脸检测 vs 纯软件 OpenCV:耗时 + 检出人脸对比

用法(板子 Jupyter,当前目录需有 haar.bit/.hwh、hybrid.py、haar_rest.c):
    !python3 bench_hybrid.py haar.bit p1_c.jpg p2_c.jpg p3_c.jpg p4_c.jpg
"""
import sys, os, time
import numpy as np, cv2
import hybrid as Hy

W, H = 640, 480


def find_cascade():
    c = []
    try:
        c.append(os.path.join(cv2.data.haarcascades, "haarcascade_frontalface_default.xml"))
    except Exception:
        pass
    c += ["haarcascade_frontalface_default.xml",
          "/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml",
          "/usr/share/opencv/haarcascades/haarcascade_frontalface_default.xml"]
    for p in c:
        if os.path.exists(p):
            return p
    raise SystemExit("找不到 haarcascade_frontalface_default.xml")


class PLHaar:
    def __init__(self, bit):
        from pynq import Overlay, allocate
        self.ol = Overlay(bit)
        self.dma = self.ol.axi_dma_0
        self.inb = allocate(shape=(H * W,), dtype=np.uint8)
        self.outb = allocate(shape=(H * W,), dtype=np.uint8)

    def run(self, gray):
        self.inb[:] = gray.reshape(-1)
        self.dma.recvchannel.transfer(self.outb)
        self.dma.sendchannel.transfer(self.inb)
        self.dma.sendchannel.wait()
        self.dma.recvchannel.wait()
        return np.array(self.outb).reshape(H, W)


def iou(a, b):
    ax, ay, aw, ah = a; bx, by, bw, bh = b
    ix = max(0, min(ax + aw, bx + bw) - max(ax, bx)); iy = max(0, min(ay + ah, by + bh) - max(ay, by))
    inter = ix * iy
    return inter / float(aw * ah + bw * bh - inter + 1e-9)


def main():
    if len(sys.argv) < 3:
        print(__doc__); return
    pl = PLHaar(sys.argv[1])
    xml = find_cascade()
    here = os.path.dirname(os.path.abspath(__file__)) if "__file__" in globals() else "."
    hy = Hy.Hybrid(xml, here, pl.run)
    sw = cv2.CascadeClassifier(xml)
    print("级联文件:", xml)
    print("%-8s | %-26s | %-34s" % ("图片", "纯软件 OpenCV", "FPGA+ARM 混合"))
    tot_sw = tot_hy = 0.0
    for path in sys.argv[2:]:
        img = cv2.imread(path)
        img = cv2.resize(img, (W, H)) if img.shape[:2] != (H, W) else img
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        # 纯软件(全图 640x480,最小 30 像素,与混合方案相同条件)
        ts = []
        for _ in range(2):
            t0 = time.perf_counter(); fs = sw.detectMultiScale(gray, 1.1, 5, minSize=(30, 30)); ts.append(time.perf_counter() - t0)
        t_sw = min(ts) * 1000
        # 混合
        th = []
        for _ in range(3):
            t0 = time.perf_counter(); fh, _ = hy.detect(gray); th.append(time.perf_counter() - t0)
        t_hy = min(th) * 1000
        t = hy.t
        match = sum(1 for a in fh if any(iou(a, tuple(b)) > 0.4 for b in fs))
        print("%-8s | %4d ms, 检出 %d 张脸        | %4d ms, 检出 %d 张脸, 与 OpenCV 重合 %d" %
              (os.path.basename(path), t_sw, len(fs), t_hy, len(fh), match))
        print("         混合耗时明细(最后一轮): 缩放 %.0f + FPGA %.0f(含DMA,%d 次) + 取候选 %.0f + ARM 后半段 %.0f + 合并 %.0f ms; 尺度 %d, 候选窗口 %d" %
              (t['resize'] * 1000, t['pl'] * 1000, t['canvases'], t['cand'] * 1000, t['rest'] * 1000, t['group'] * 1000, t['scales'], t['cands']))
        # 精度诊断:不合并时,OpenCV 与混合方案各自找出多少原始候选框,重合多少
        raw_sw = set(tuple(int(v) for v in r) for r in sw.detectMultiScale(gray, 1.1, 0, minSize=(30, 30)))
        raw_hy = set(hy.raw_boxes(gray))
        print("         原始候选框: OpenCV %d 个, 混合 %d 个, 完全相同 %d 个, 仅OpenCV有 %d, 仅混合有 %d" %
              (len(raw_sw), len(raw_hy), len(raw_sw & raw_hy), len(raw_sw - raw_hy), len(raw_hy - raw_sw)))
        vis = img.copy()
        for (x, y, w, h) in fs:
            cv2.rectangle(vis, (x - 2, y - 2), (x + w + 2, y + h + 2), (0, 0, 255), 1)   # OpenCV: 红
        for (x, y, w, h) in fh:
            cv2.rectangle(vis, (x, y), (x + w, y + h), (0, 255, 0), 2)                   # 混合: 绿
        cv2.imwrite("out_haar_" + os.path.splitext(os.path.basename(path))[0] + ".png", vis)
        tot_sw += t_sw; tot_hy += t_hy
    print("合计: 纯软件 %.0f ms, 混合 %.0f ms, 加速比 %.2fx" % (tot_sw, tot_hy, tot_sw / tot_hy))


main()
