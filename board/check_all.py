"""联合比特流上板检查:Canny(axi_dma_1)、按键/拨码/LED/RGB。Haar 另用 check_haar.py all.bit 检查。
用法: !python3 check_all.py all.bit
"""
import sys, time
import numpy as np

W, H = 640, 480

def main():
    from pynq import Overlay, allocate
    from pynq.lib import AxiGPIO
    ol = Overlay(sys.argv[1] if len(sys.argv) > 1 else "all.bit")
    print("设计里的 IP:", sorted(ol.ip_dict.keys()))

    # 1) Canny: 黑底白方块,应该在方块边缘出现白线
    dma = ol.axi_dma_1
    img = np.zeros((H, W), np.uint8); img[120:360, 200:440] = 200
    inb = allocate(shape=(H * W,), dtype=np.uint8); outb = allocate(shape=(H * W,), dtype=np.uint8)
    inb[:] = img.reshape(-1)
    dma.recvchannel.transfer(outb); dma.sendchannel.transfer(inb)
    dma.sendchannel.wait(); dma.recvchannel.wait()
    out = np.array(outb).reshape(H, W)
    ys, xs = np.nonzero(out)
    print("Canny: 输出边缘点 %d 个" % len(ys), end="")
    if len(ys):
        print(",范围 y %d~%d x %d~%d(方块在 y120~360 x200~440 附近)" % (ys.min(), ys.max(), xs.min(), xs.max()))
    else:
        print(" —— 没有边缘,不正常")
    t0 = time.perf_counter()
    for _ in range(20):
        dma.recvchannel.transfer(outb); dma.sendchannel.transfer(inb)
        dma.sendchannel.wait(); dma.recvchannel.wait()
    print("Canny 每帧(含 Python+DMA) %.2f ms" % ((time.perf_counter() - t0) * 1000 / 20))

    # 2) GPIO
    btn = AxiGPIO(ol.ip_dict["axi_gpio_btn"]).channel1
    sw = AxiGPIO(ol.ip_dict["axi_gpio_sw"]).channel1
    led = AxiGPIO(ol.ip_dict["axi_gpio_led"]).channel1
    rgb = AxiGPIO(ol.ip_dict["axi_gpio_rgb"]).channel1
    print("LED 流水灯...")
    for i in range(4):
        led.write(1 << i, 0xF); time.sleep(0.3)
    led.write(0, 0xF)
    print("RGB 灯:LD4 蓝/绿/红,再 LD5 蓝/绿/红...")
    for i in range(6):
        rgb.write(1 << i, 0x3F); time.sleep(0.4)
    rgb.write(0, 0x3F)
    print("请在 10 秒内按各个按键、拨动拨码开关,屏幕会显示读数(LED 跟随按键)")
    last = None; t_end = time.time() + 10
    while time.time() < t_end:
        cur = (btn.read(), sw.read())
        led.write(cur[0], 0xF)
        if cur != last:
            print("按键 %s  拨码 %s" % (format(cur[0], "04b"), format(cur[1], "02b"))); last = cur
        time.sleep(0.05)
    led.write(0, 0xF)
    print("结束")

main()
