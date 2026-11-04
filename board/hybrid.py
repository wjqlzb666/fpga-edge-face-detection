"""FPGA(前 K 级 Haar 早期淘汰) + ARM(C 语言完成后面各级) 的混合人脸检测。
FPGA 部分在板子上通过 PL 对象提供;本文件也可在电脑上用参考模型代替 FPGA 做对照测试。"""
import os, ctypes, subprocess, time
import xml.etree.ElementTree as ET
import numpy as np, cv2

W, H, WIN = 640, 480, 24
K_PL = 4                      # FPGA 完成的级数(与比特流一致)


def load_cascade(xml_path):
    c = ET.parse(xml_path).getroot()[0]
    feats = []
    for f in c.find('features'):
        rs = []
        for r in f.find('rects'):
            x, y, w, h, wt = r.text.strip().rstrip(';').split()
            rs.append((int(x), int(y), int(w), int(h), float(wt)))
        feats.append(rs)
    nf = len(feats)
    f_nr = np.zeros(nf, np.int32); f_rect = np.zeros(nf * 12, np.int32); f_w = np.zeros(nf * 3, np.float32)
    for i, rs in enumerate(feats):
        f_nr[i] = len(rs)
        for j, (x, y, w, h, wt) in enumerate(rs):
            f_rect[i * 12 + j * 4: i * 12 + j * 4 + 4] = (x, y, w, h); f_w[i * 3 + j] = wt
    stage_n, stage_th, wk_feat, wk_thr, wk_l0, wk_l1 = [], [], [], [], [], []
    for s in c.find('stages'):
        stage_th.append(float(s.find('stageThreshold').text)); n = 0
        for wc in s.find('weakClassifiers'):
            nn = wc.find('internalNodes').text.split(); lv = wc.find('leafValues').text.split()
            wk_feat.append(int(nn[2])); wk_thr.append(float(nn[3])); wk_l0.append(float(lv[0])); wk_l1.append(float(lv[1])); n += 1
        stage_n.append(n)
    return dict(stage_n=np.array(stage_n, np.int32), stage_th=np.array(stage_th, np.float32),
                wk_feat=np.array(wk_feat, np.int32), wk_thr=np.array(wk_thr, np.float32),
                wk_l0=np.array(wk_l0, np.float32), wk_l1=np.array(wk_l1, np.float32),
                f_nr=f_nr, f_rect=f_rect, f_w=f_w)


def load_lib(here):
    so = os.path.join(here, 'haar_rest.so'); c = os.path.join(here, 'haar_rest.c')
    if not os.path.exists(so):
        try:
            subprocess.check_call(['gcc', '-O3', '-ffast-math', '-mcpu=cortex-a9', '-mfpu=neon', '-shared', '-fPIC', '-o', so, c, '-lm'])
        except Exception:
            subprocess.check_call(['gcc', '-O3', '-shared', '-fPIC', '-o', so, c, '-lm'])
    lib = ctypes.CDLL(so)
    P = ctypes.c_void_p
    lib.haar_rest.argtypes = [P, ctypes.c_int, ctypes.c_int, P, P, ctypes.c_int, ctypes.c_int, ctypes.c_int,
                              P, P, P, P, P, P, P, P, P, P]
    lib.haar_rest.restype = ctypes.c_int
    return lib


def _p(a):
    return a.ctypes.data_as(ctypes.c_void_p)


class Hybrid:
    def __init__(self, xml, here, pl_run, scale=1.1, min_size=30):
        """pl_run(canvas640x480) -> 同尺寸 0/255 图(FPGA 前 K 级通过的窗口左上角)"""
        self.p = load_cascade(xml); self.lib = load_lib(here)
        self.pl_run = pl_run; self.scale = scale; self.min_size = min_size
        self.nstage = len(self.p['stage_n'])
        self.t = dict(resize=0.0, pl=0.0, cand=0.0, rest=0.0, group=0.0, cands=0, scales=0, canvases=0)

    def _scales(self, gh, gw):
        out = []; f = 1.0
        while True:
            win = int(round(WIN * f))
            if win >= self.min_size:
                sw, sh = int(round(gw / f)), int(round(gh / f))
                if sw < WIN or sh < WIN: break
                out.append((f, win, sw, sh))
            else:
                sw, sh = int(round(gw / f)), int(round(gh / f))
                if sw < WIN or sh < WIN: break
            f *= self.scale
        return out

    @staticmethod
    def _pack(sizes):
        """把多个小图装进若干张 640x480 画布(简单的空闲矩形法)。返回 [(canvas_id, x, y)]"""
        free = []          # (canvas, x, y, w, h)
        place = []; ncan = 0
        order = sorted(range(len(sizes)), key=lambda i: -sizes[i][1])
        res = {}
        for i in order:
            w, h = sizes[i]
            best = None
            for k, (c, x, y, fw, fh) in enumerate(free):
                if w <= fw and h <= fh:
                    sc = min(fw - w, fh - h)
                    if best is None or sc < best[0]: best = (sc, k)
            if best is None:
                free.append((ncan, 0, 0, W, H)); ncan += 1
                best = (0, len(free) - 1)
            c, x, y, fw, fh = free.pop(best[1])
            res[i] = (c, x, y)
            if fw - w > 0: free.append((c, x + w, y, fw - w, h))
            if fh - h > 0: free.append((c, x, y + h, fw, fh - h))
        return [res[i] for i in range(len(sizes))], ncan

    def raw_boxes(self, gray, mosaic=True, threads=2):
        from concurrent.futures import ThreadPoolExecutor
        gh, gw = gray.shape
        t = self.t
        for k in t: t[k] = 0
        sc = [s for s in self._scales(gh, gw) if s[1] >= self.min_size]
        t0 = time.perf_counter()
        interp = getattr(cv2, 'INTER_LINEAR_EXACT', cv2.INTER_LINEAR) if os.environ.get('HAAR_INTERP', 'exact') == 'exact' else cv2.INTER_LINEAR
        imgs = [cv2.resize(gray, (sw, sh), interpolation=interp) for (f, win, sw, sh) in sc]
        if mosaic:
            pos, ncan = self._pack([(sw, sh) for (f, win, sw, sh) in sc])
        else:
            pos = [(i, 0, 0) for i in range(len(sc))]; ncan = len(sc)
        canvases = [np.zeros((H, W), np.uint8) for _ in range(ncan)]
        for im, (c, x, y) in zip(imgs, pos):
            canvases[c][y:y + im.shape[0], x:x + im.shape[1]] = im
        t['resize'] = time.perf_counter() - t0
        t0 = time.perf_counter()
        maps = [self.pl_run(cv) for cv in canvases]
        t['pl'] = time.perf_counter() - t0
        t0 = time.perf_counter()
        jobs = []
        for (f, win, sw, sh), g, (c, ox, oy) in zip(sc, imgs, pos):
            # 与 OpenCV 一致:缩放系数 f>2(大脸,缩得很小的图)窗口步长为 1,否则步长为 2
            st = 1 if f > 2.0 else 2
            sub = maps[c][oy:oy + sh - WIN + 1:st, ox:ox + sw - WIN + 1:st]
            ys, xs = np.nonzero(sub)
            ys = np.ascontiguousarray(ys * st, dtype=np.int32); xs = np.ascontiguousarray(xs * st, dtype=np.int32)
            t['cands'] += len(ys)
            jobs.append((f, win, g, ys, xs))
        t['cand'] = time.perf_counter() - t0
        t0 = time.perf_counter()
        p = self.p

        def work(job):
            f, win, g, ys, xs = job
            if len(ys) == 0: return []
            sh, sw = g.shape
            out = np.zeros(len(ys), np.int32)
            n = self.lib.haar_rest(_p(g), sw, sh, _p(ys), _p(xs), len(ys), K_PL, self.nstage,
                                   _p(p['stage_n']), _p(p['stage_th']), _p(p['wk_feat']), _p(p['wk_thr']),
                                   _p(p['wk_l0']), _p(p['wk_l1']), _p(p['f_nr']), _p(p['f_rect']), _p(p['f_w']), _p(out))
            return [(int(round(int(xs[i]) * f)), int(round(int(ys[i]) * f)), win, win) for i in out[:n]]
        # 候选窗口多的大尺度先做,两个线程轮流取
        order = sorted(range(len(jobs)), key=lambda i: -len(jobs[i][3]))
        with ThreadPoolExecutor(max_workers=threads) as ex:
            res = list(ex.map(work, [jobs[i] for i in order]))
        boxes = [b for r in res for b in r]
        t['rest'] = time.perf_counter() - t0
        t['scales'] = len(sc); t['canvases'] = ncan
        return boxes

    @staticmethod
    def _iou(a, b):
        ax, ay, aw, ah = a; bx, by, bw, bh = b
        ix = max(0, min(ax + aw, bx + bw) - max(ax, bx)); iy = max(0, min(ay + ah, by + bh) - max(ay, by))
        inter = ix * iy
        return inter / float(aw * ah + bw * bh - inter + 1e-9)

    def detect(self, gray, min_neighbors=5, suppress=False):
        """返回 (人脸框列表, 原始候选框)。suppress=True 时去掉重叠/包含的重复框(识别时使用)"""
        boxes = self.raw_boxes(gray)
        t0 = time.perf_counter()
        out, wts = [], []
        if boxes:
            rects, w = cv2.groupRectangles([list(b) for b in boxes], min_neighbors, 0.2)
            out = [tuple(int(v) for v in r) for r in rects]
            wts = [int(v) for v in np.array(w).reshape(-1)] if len(w) == len(out) else [0] * len(out)
        if suppress and len(out) > 1:
            order = sorted(range(len(out)), key=lambda i: -(wts[i] * 10000 + out[i][2]))
            keep = []
            for i in order:
                x, y, bw, bh = out[i]; cx, cy = x + bw / 2.0, y + bh / 2.0
                dup = False
                for j in keep:
                    X, Y, Wj, Hj = out[j]
                    if (X <= cx <= X + Wj and Y <= cy <= Y + Hj) or self._iou(out[i], out[j]) > 0.3:
                        dup = True; break
                if not dup: keep.append(i)
            out = [out[i] for i in keep]
            wts = [wts[i] for i in keep]
        self.last_w = wts if len(wts) == len(out) else [0] * len(out)
        self.t['group'] = time.perf_counter() - t0
        return out, boxes
