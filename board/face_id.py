"""人脸录入 + 识别(检测用 FPGA 加速的 Haar,识别用 OpenCV 自带 LBPH,在 ARM 上运行)

用法(板子 Jupyter 里,当前目录需有 haar.bit/.hwh、hybrid.py、haar_rest.c):
  录入:  !python3 face_id.py enroll 张三 20          # 对着摄像头,保存 20 张脸(默认 20)
  训练:  !python3 face_id.py train                   # 录入完成后自动训练,也可手动重训
  识别:  !python3 face_id.py recognize cam 40        # 摄像头识别 40 帧,保存 id_out.avi / id_frame.png
  边认边学: !python3 face_id.py recognize cam 40 --learn   # 很有把握时把当前的脸加入该人的照片库,结束后自动重训
  站位:  !python3 face_id.py check                   # 检查摄像头位置/距离是否合适,保存 check_frame.png
  图片:  !python3 face_id.py test p3_c.jpg p4_c.jpg  # 对静态图片识别,保存 id_test_*.png
  列表:  !python3 face_id.py list                    # 查看已录入的人和照片数
  清除:  !python3 face_id.py clear 张三               # 删除某人的全部照片(隐私),不写名字则清空全部

人脸照片和模型只保存在板子本地的 faces/ 目录,不会上传。只录入你自己和明确同意的人。
"""
import sys, os, time, json, shutil, glob
import numpy as np, cv2
import hybrid as Hy

W, H = 640, 480
HERE = os.path.dirname(os.path.abspath(__file__)) if "__file__" in globals() else "."
FACES = os.path.join(HERE, "faces")
MODEL = os.path.join(HERE, "lbph.yml")
LABELS = os.path.join(HERE, "labels.json")
FACE_SIZE = 100          # 识别用的脸图统一缩放到 100x100
UNKNOWN_THR = 95.0       # LBPH 距离大于这个值判为"陌生人"(越小越像,可用 --thr 调整)
LEARN_THR = 72.0         # 自动学习:距离小于这个值(非常像)才把当前这张脸加入照片库,可用 --lthr 调整
NONFACE_THR = 135.0      # 距离大于这个值的框基本不是人脸(多半是背景物体),学习时忽略它们
MAX_AUTO = 40            # 每个人最多保留多少张自动学到的照片,超过就删最旧的
BIT = "haar.bit"


def find_cascade():
    c = []
    try:
        c.append(os.path.join(cv2.data.haarcascades, "haarcascade_frontalface_default.xml"))
    except Exception:
        pass
    c += ["/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml",
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


def make_detector():
    pl = PLHaar(BIT)
    return Hy.Hybrid(find_cascade(), HERE, pl.run)


def to_face(gray, box):
    """裁出脸并规范化:略去边缘,缩放到固定大小,直方图均衡(减弱光照影响)"""
    x, y, w, h = box
    m = int(0.08 * w)
    x0, y0 = max(0, x + m), max(0, y + m)
    x1, y1 = min(gray.shape[1], x + w - m), min(gray.shape[0], y + h - m)
    f = cv2.resize(gray[y0:y1, x0:x1], (FACE_SIZE, FACE_SIZE))
    return cv2.equalizeHist(f)


def open_source(src):
    cap = cv2.VideoCapture(0 if src == "cam" else src)
    cap.set(3, W); cap.set(4, H)
    for _ in range(8):
        cap.read()
    return cap


def read_frame(cap):
    ok, fr = cap.read()
    if not ok:
        return None
    if fr.shape[:2] != (H, W):
        fr = cv2.resize(fr, (W, H))
    return fr


def list_people():
    if not os.path.isdir(FACES):
        return {}
    return {n: len(glob.glob(os.path.join(FACES, n, "*.png"))) for n in sorted(os.listdir(FACES))
            if os.path.isdir(os.path.join(FACES, n))}


def train():
    ppl = list_people()
    ppl = {n: c for n, c in ppl.items() if c > 0}
    if not ppl:
        print("还没有录入任何人"); return False
    names = sorted(ppl)
    imgs, labels = [], []
    for i, n in enumerate(names):
        for p in sorted(glob.glob(os.path.join(FACES, n, "*.png"))):
            im = cv2.imread(p, 0)
            if im is not None:
                imgs.append(cv2.resize(im, (FACE_SIZE, FACE_SIZE))); labels.append(i)
    rec = cv2.face.LBPHFaceRecognizer_create(1, 8, 8, 8)
    rec.train(imgs, np.array(labels, np.int32))
    rec.write(MODEL)
    json.dump(names, open(LABELS, "w"), ensure_ascii=False)
    print("训练完成:", ", ".join("%s(%d张)" % (n, ppl[n]) for n in names))
    return True


def load_model():
    if not (os.path.exists(MODEL) and os.path.exists(LABELS)):
        raise SystemExit("还没有训练好的模型,请先录入并训练")
    rec = cv2.face.LBPHFaceRecognizer_create(1, 8, 8, 8)
    rec.read(MODEL)
    return rec, json.load(open(LABELS))


def enroll(name, n, src="cam"):
    det = make_detector()
    cap = open_source(src)
    d = os.path.join(FACES, name); os.makedirs(d, exist_ok=True)
    have = len(glob.glob(os.path.join(d, "*.png")))
    saved = tries = 0
    print("开始录入 %s:请正对摄像头,光线充足,每次检测到脸会保存一张,过程中缓慢转动头部。" % name)
    while saved < n and tries < n * 6:
        fr = read_frame(cap)
        if fr is None:
            print("读不到摄像头画面"); break
        gray = cv2.cvtColor(fr, cv2.COLOR_BGR2GRAY)
        faces, _ = det.detect(gray, suppress=True)
        tries += 1
        if len(faces) != 1:
            print("  第 %d 次尝试:检测到 %d 张脸(需要恰好 1 张),跳过" % (tries, len(faces)))
            continue
        x, y, w, h = faces[0]
        if w < 80:
            print("  脸太小(%d 像素),请靠近一点" % w); continue
        cv2.imwrite(os.path.join(d, "%03d.png" % (have + saved)), to_face(gray, faces[0]))
        saved += 1
        print("  已保存 %d/%d" % (saved, n))
    cap.release()
    print("录入结束,共保存 %d 张。" % saved)
    if saved:
        train()


def draw(img, box, text, ok):
    x, y, w, h = box
    col = (0, 255, 0) if ok else (0, 0, 255)
    cv2.rectangle(img, (x, y), (x + w, y + h), col, 2)
    cv2.putText(img, text, (x, max(14, y - 6)), cv2.FONT_HERSHEY_SIMPLEX, 0.6, col, 2, cv2.LINE_AA)


def identify(rec, names, gray, boxes, thr, weights=None):
    """返回 [(框, 名字, 距离, 是否认识, 检测可信度)];可信度 = 合并时的邻居框数,越大越像真脸"""
    out = []
    for k, b in enumerate(boxes):
        lab, dist = rec.predict(to_face(gray, b))
        known = dist <= thr
        w = weights[k] if weights is not None and k < len(weights) else 0
        out.append((b, names[lab] if known else "unknown", dist, known, w))
    return out


def save_auto(name, face):
    d = os.path.join(FACES, name)
    cv2.imwrite(os.path.join(d, "auto_%d.png" % int(time.time() * 1000)), face)
    autos = sorted(glob.glob(os.path.join(d, "auto_*.png")), key=os.path.getmtime)
    for p in autos[:-MAX_AUTO]:
        os.remove(p)


def recognize(src, nframes, thr, learn=False, lthr=LEARN_THR):
    rec, names = load_model()
    det = make_detector()
    cap = open_source(src)
    vw = cv2.VideoWriter("id_out.avi", cv2.VideoWriter_fourcc(*"MJPG"), 3, (W, H))
    last = None; t_all = []
    streak_name, streak, added = None, 0, 0
    for i in range(nframes):
        fr = read_frame(cap)
        if fr is None: break
        gray = cv2.cvtColor(fr, cv2.COLOR_BGR2GRAY)
        t0 = time.perf_counter()
        faces, _ = det.detect(gray, suppress=True)
        res = identify(rec, names, gray, faces, thr, det.last_w)
        t_all.append((time.perf_counter() - t0) * 1000)
        note = ""
        if learn:
            # 只在画面里恰好 1 张脸、被认成某人、距离很小,并且连续 3 帧都是同一个人时才学习,避免把别人学进去
            real = [r for r in res if r[2] < NONFACE_THR]      # 忽略明显不是人脸的背景框
            if len(real) == 1 and real[0][3] and real[0][2] < lthr:
                res0 = real[0]
                nm = res0[1]
                streak = streak + 1 if nm == streak_name else 1
                streak_name = nm
                if streak >= 3:
                    save_auto(nm, to_face(gray, res0[0])); added += 1; streak = 0; note = "  [已学习]"
            else:
                streak_name, streak = None, 0
        for (b, nm, dist, ok, w) in res:
            draw(fr, b, "%s %.0f" % (nm, dist), ok)
        vw.write(fr); last = fr
        print("帧 %d: %s%s" % (i, ", ".join("%s(距离%.0f,可信度%d)" % (nm, d, w) for (_, nm, d, _, w) in res) or "没有脸", note))
    vw.release(); cap.release()
    if last is not None:
        cv2.imwrite("id_frame.png", last)
    if t_all:
        print("每帧 检测+识别 平均 %.0f ms;结果: id_out.avi, id_frame.png" % np.mean(t_all))
    if learn:
        print("本次自动学习了 %d 张新照片" % added)
        if added:
            train()


def test_images(paths, thr):
    rec, names = load_model()
    det = make_detector()
    for p in paths:
        img = cv2.imread(p)
        img = cv2.resize(img, (W, H)) if img.shape[:2] != (H, W) else img
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        faces, _ = det.detect(gray, suppress=True)
        res = identify(rec, names, gray, faces, thr, det.last_w)
        for (b, nm, dist, ok, w) in res:
            draw(img, b, "%s %.0f" % (nm, dist), ok)
        cv2.imwrite("id_test_" + os.path.splitext(os.path.basename(p))[0] + ".png", img)
        print("%s: %s" % (os.path.basename(p), ", ".join("%s(距离%.0f,可信度%d)" % (nm, d, w) for (_, nm, d, _, w) in res) or "没有脸"))


def check_position():
    det = make_detector()
    cap = open_source("cam")
    last = None; good = 0
    for i in range(5):
        fr = read_frame(cap)
        if fr is None:
            print("读不到摄像头画面"); return
        gray = cv2.cvtColor(fr, cv2.COLOR_BGR2GRAY)
        faces, _ = det.detect(gray, suppress=True)
        msgs = []
        for (x, y, w, h) in faces:
            cut = x <= 2 or y <= 2 or x + w >= W - 2 or y + h >= H - 2
            if cut:
                v = "被画面边缘截断"
            elif w < 150:
                v = "太小(太远或不是人脸)"
            elif w > 320:
                v = "太大(太近)"
            else:
                v = "合适"; good += 1
            msgs.append("框宽%d: %s" % (w, v))
            cv2.rectangle(fr, (x, y), (x + w, y + h), (0, 255, 0) if v == "合适" else (0, 0, 255), 2)
        print("第 %d 帧: %s" % (i + 1, "; ".join(msgs) or "没有检测到脸"))
        last = fr
    cap.release()
    if last is not None:
        cv2.imwrite("check_frame.png", last)
    print("结论:", "位置合适,可以录入/识别了" if good >= 3 else "位置还不合适:请后退到 60 厘米左右,摄像头放平,让整张脸(额头到下巴)完整出现在画面中央,再运行一次")


def main():
    a = sys.argv[1:]
    thr = UNKNOWN_THR; lthr = LEARN_THR; learn = False
    if "--thr" in a:
        i = a.index("--thr"); thr = float(a[i + 1]); del a[i:i + 2]
    if "--lthr" in a:
        i = a.index("--lthr"); lthr = float(a[i + 1]); del a[i:i + 2]
    if "--learn" in a:
        a.remove("--learn"); learn = True
    if not a:
        print(__doc__); return
    cmd = a[0]
    if cmd == "enroll":
        enroll(a[1], int(a[2]) if len(a) > 2 else 20)
    elif cmd == "train":
        train()
    elif cmd == "recognize":
        recognize(a[1] if len(a) > 1 else "cam", int(a[2]) if len(a) > 2 else 40, thr, learn, lthr)
    elif cmd == "test":
        test_images(a[1:], thr)
    elif cmd == "check":
        check_position()
    elif cmd == "list":
        print(list_people() or "还没有录入任何人")
    elif cmd == "clear":
        if len(a) > 1:
            shutil.rmtree(os.path.join(FACES, a[1]), ignore_errors=True); print("已删除", a[1])
            train() if list_people() else [os.remove(f) for f in (MODEL, LABELS) if os.path.exists(f)]
        else:
            shutil.rmtree(FACES, ignore_errors=True)
            for f in (MODEL, LABELS):
                if os.path.exists(f): os.remove(f)
            print("已清空全部录入数据")
    else:
        print(__doc__)


main()
