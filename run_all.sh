#!/bin/bash
set -e

cd /mnt/c/Users/CKJUN/crossmux
source .venv/bin/activate

echo "========== 1. 编译微雪真机固件 =========="
platformio run -e waveshare_epaper_397

echo ""
echo "========== 2. 打开固件输出目录 =========="
FIRMWARE_DIR="/mnt/c/Users/CKJUN/crossmux/.pio/build/waveshare_epaper_397"
WIN_PATH=$(wslpath -w "$FIRMWARE_DIR")
echo "固件目录（Windows）: $WIN_PATH"
explorer.exe "$WIN_PATH" || true

echo ""
echo "========== 3. 启动模拟器 =========="
platformio run -e simulator -t run_simulator
