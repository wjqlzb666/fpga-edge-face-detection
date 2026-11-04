# 检测到人脸就触发:截图 + 记录日志 + 打印提示。认识的人=放行,不认识=警报。
# 用法: !python3 trigger.py 60        (运行 60 秒,按停止键也能中断)
import sys, os, time, csv
import cv2
import face_id as F

HOLD = 3          # 连续 3 帧都看到脸才触发,防止偶尔误检
COOLDOWN = 5.0    # 同一类事件触发后 5 秒内不重复触发
OUTDIR = "events"

def on_trigger(kind, name, dist, frame):
    """触发动作都写在这里。以后接 LED/继电器/蜂鸣器,只改这个函数。"""
    os.makedirs(OUTDIR, exist_ok=True)
    ts = time.strftime("%Y%m%d_%H%M%S")
    path = "%s/%s_%s_%s.jpg" % (OUTDIR, ts, kind, name)
    cv2.imwrite(path, frame)
    new = not os.path.exists("events.csv")
    with open("events.csv", "a", newline="") as f:
        w = csv.writer(f)
        if new: w.writerow(["时间", "事件", "姓名", "距离", "截图"])
        w.writerow([time.strftime("%Y-%m-%d %H:%M:%S"), kind, name, "%.0f" % dist, path])
    tip = "放行" if kind == "ACCESS_OK" else "警报:陌生人"
    print("*** 触发 [%s] %s 距离%.0f -> %s  (%s)" % (kind, name, dist, tip, path))

def main():
    secs = float(sys.argv[1]) if len(sys.argv) > 1 else 60
    rec, names = F.load_model()
    det = F.make_detector()
    cap = F.open_source("cam")
    t_end = time.time() + secs
    streak = {"ACCESS_OK": 0, "ALERT": 0}
    last_fire = {"ACCESS_OK": 0, "ALERT": 0}
    print("开始监视 %d 秒,有人脸就触发..." % secs)
    try:
        while time.time() < t_end:
            fr = F.read_frame(cap)
            if fr is None: break
            gray = cv2.cvtColor(fr, cv2.COLOR_BGR2GRAY)
            faces, _ = det.detect(gray, suppress=True)
            res = F.identify(rec, names, gray, faces, F.UNKNOWN_THR, det.last_w)
            real = [r for r in res if r[2] < F.NONFACE_THR]   # 忽略背景误检框
            kind = None
            if real:
                best = min(real, key=lambda r: r[2])
                kind = "ACCESS_OK" if best[3] else "ALERT"
            for k in streak:
                streak[k] = streak[k] + 1 if k == kind else 0
            if kind and streak[kind] >= HOLD and time.time() - last_fire[kind] > COOLDOWN:
                for (b, nm, d, ok, w) in real:
                    F.draw(fr, b, "%s %.0f" % (nm, d), ok)
                on_trigger(kind, best[1], best[2], fr)
                last_fire[kind] = time.time(); streak[kind] = 0
    except KeyboardInterrupt:
        pass
    cap.release()
    print("结束。记录在 events.csv,截图在 events/ 文件夹")

if __name__ == "__main__":
    main()
