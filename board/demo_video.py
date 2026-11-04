"""演示:视频(或摄像头) -> PL(FPGA) Canny 边缘检测 + PS 周期性人脸检测 -> 输出对比视频

用法(板子 Jupyter 里):
    !python3 demo_video.py canny.bit demo.mp4              # 处理视频文件
    !python3 demo_video.py canny.bit demo.mp4 120          # 只处理前 120 帧
    !python3 demo_video.py canny.bit cam                   # 用 USB 摄像头(默认 100 帧)
结果:
    demo_out.avi   左边=原画面+人脸框,右边=FPGA Canny 边缘图
    demo_frame.png 第 30 帧的截图(写文档用)
"""
import sys, os, time
import numpy as np
import cv2

W, H = 640, 480
FACE_EVERY = 5          # 每 5 帧做一次人脸检测,中间沿用上一次的框
HAAR_W, HAAR_H = 320, 240   # 人脸检测用缩小后的图,速度快约 3 倍


def find_cascade():
    cands = []
    try:
        cands.append(os.path.join(cv2.data.haarcascades, "haarcascade_frontalface_default.xml"))
    except Exception:
        pass
    cands += ["/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml",
              "/usr/share/opencv/haarcascades/haarcascade_frontalface_default.xml"]
    for c in cands:
        if os.path.exists(c):
            return c
    return None


class PLCanny:
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


def to_640x480(frame):
    """保持比例:先居中裁成 4:3,再缩放到 640x480(不拉伸)"""
    h, w = frame.shape[:2]
    if w * 3 > h * 4:            # 太宽,裁左右
        nw = h * 4 // 3
        x0 = (w - nw) // 2
        frame = frame[:, x0:x0 + nw]
    elif w * 3 < h * 4:          # 太高,裁上下
        nh = w * 3 // 4
        y0 = (h - nh) // 2
        frame = frame[y0:y0 + nh, :]
    return cv2.resize(frame, (W, H), interpolation=cv2.INTER_AREA)


def detect_faces(casc, gray):
    small = cv2.resize(gray, (HAAR_W, HAAR_H))
    rects = casc.detectMultiScale(small, 1.1, 5, minSize=(50, 50))
    s = W / float(HAAR_W)
    if len(rects) > 1:
        rects = [max(rects, key=lambda r: r[2] * r[3])]
    return [(int(x * s), int(y * s), int(w * s), int(h * s)) for (x, y, w, h) in rects]


def main():
    if len(sys.argv) < 3:
        print(__doc__); return
    bit, src = sys.argv[1], sys.argv[2]
    max_frames = int(sys.argv[3]) if len(sys.argv) > 3 else (100 if src == "cam" else 10**9)

    cap = cv2.VideoCapture(0 if src == "cam" else src)
    if not cap.isOpened():
        print("打不开视频/摄像头:", src); return
    fps_src = cap.get(cv2.CAP_PROP_FPS)
    if not fps_src or fps_src < 1 or fps_src > 120:
        fps_src = 30.0

    casc = None
    cp = find_cascade()
    if cp is not None and hasattr(cv2, "CascadeClassifier"):
        casc = cv2.CascadeClassifier(cp)
        if casc.empty():
            casc = None
    if casc is None:
        print("警告:没找到人脸检测模型,本次只做边缘检测")

    pl = PLCanny(bit)
    out = cv2.VideoWriter("demo_out.avi", cv2.VideoWriter_fourcc(*"MJPG"), fps_src, (W * 2, H))

    t_pl, t_face = [], []
    boxes = []
    n = 0
    t_start = time.perf_counter()
    while n < max_frames:
        ok, frame = cap.read()
        if not ok:
            break
        frame = to_640x480(frame)
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

        t0 = time.perf_counter()
        edge = pl.run(gray)
        t_pl.append((time.perf_counter() - t0) * 1000.0)

        if casc is not None and n % FACE_EVERY == 0:
            t0 = time.perf_counter()
            boxes = detect_faces(casc, gray)
            t_face.append((time.perf_counter() - t0) * 1000.0)

        left = frame.copy()
        for (x, y, w, h) in boxes:
            cv2.rectangle(left, (x, y), (x + w, y + h), (0, 255, 0), 2)
        right = cv2.cvtColor(edge, cv2.COLOR_GRAY2BGR)
        cv2.putText(left, "camera/video + face boxes (every %d frames)" % FACE_EVERY, (8, 22),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.55, (0, 255, 255), 1, cv2.LINE_AA)
        cv2.putText(right, "FPGA Canny (PL) %.1f ms/frame" % t_pl[-1], (8, 22),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.55, (0, 255, 255), 1, cv2.LINE_AA)
        comp = np.hstack([left, right])
        out.write(comp)
        if n == 30:
            cv2.imwrite("demo_frame.png", comp)
        n += 1

    total = time.perf_counter() - t_start
    cap.release(); out.release()

    if n == 0:
        print("没有读到任何帧"); return
    if n <= 30:
        cv2.imwrite("demo_frame.png", comp)
    print("\n===== 演示统计 =====")
    print("处理帧数: %d,总耗时 %.1f s(含读视频、缩放、画框、写视频)" % (n, total))
    print("FPGA Canny 每帧(含DMA): 平均 %.2f ms,约 %.0f FPS" % (np.mean(t_pl), 1000.0 / np.mean(t_pl)))
    if t_face:
        print("人脸检测(%dx%d): 平均 %.0f ms/次,共 %d 次,每 %d 帧刷新一次人脸框"
              % (HAAR_W, HAAR_H, np.mean(t_face), len(t_face), FACE_EVERY))
    print("输出文件: demo_out.avi, demo_frame.png")


if __name__ == "__main__":
    main()
