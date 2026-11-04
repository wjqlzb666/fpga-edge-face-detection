# FPGA 边缘人脸检测系统(PYNQ-Z2 / Zynq-7020)

PL 端用 Vivado HLS 实现 Canny 边缘检测与 Haar 级联人脸检测前 4 级流式 IP;PS 端(PYNQ Linux)完成摄像头采集、图像金字塔、Haar 后续级判决、框合并与 LBPH 人脸识别。

## 目录
- `hls/haar/`    Haar 前 4 级 HLS 源码(haar.cpp/.h,haar_eval.h 由 gen_eval.py 生成),C 仿真 testbench,测试向量生成脚本 mkvec.py
- `hls/canny/`   Canny HLS 源码与 testbench
- `board/`       板上运行的 Python/C 代码
  - `hybrid.py`、`haar_rest.c`  混合式人脸检测(PL 前 4 级 + PS 第 5~25 级 + 框合并)
  - `face_id.py`  人脸录入、训练、识别、在线学习
  - `trigger.py`  检测到人脸后的触发脚本(截图、日志;未经完整验证,仅供参考)
  - `check_haar.py`、`check_all.py`  上板验证脚本
  - `bench_hybrid.py`、`bench_canny.py`、`demo_video.py`  性能对比与演示
- `vivado/pins.xdc`  按键/拨码开关/LED/RGB 灯引脚约束
- `test_vectors/`    Haar IP 上板验证用的输入与期望输出(640×480 灰度 raw)

## 使用(板上,Jupyter)
1. 把比特流 all.bit 与 all.hwh(或 haar.bit/haar.hwh、canny.bit/canny.hwh)放到与脚本相同的目录。
2. Haar IP 逐像素验证:`!python3 check_haar.py all.bit`(需要 test_vectors 里的 .raw)
3. 混合检测对比 OpenCV:`!python3 bench_hybrid.py haar.bit 图片1.jpg 图片2.jpg`
4. 人脸识别:`!python3 face_id.py enroll 名字 25`、`!python3 face_id.py recognize cam 20`

## 说明
- 未包含 haarcascade_frontalface_default.xml(使用 OpenCV 自带文件,由 Intel 贡献,遵循 OpenCV 附带许可声明)。
- 未包含个人人脸照片与模型文件。
- 功耗未实测;LBPH 识别不具备活体检测能力,仅为演示级。
