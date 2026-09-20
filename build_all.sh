#!/bin/bash
# 一次编译真机固件 + 模拟器
set -e

cd ~/crossmux
source .venv/bin/activate

echo "========== 1. 编译真机固件 =========="
platformio run -e waveshare_epaper_397 2>&1 | grep -E "gen_i18n|Flash:|RAM:|SUCCESS|FAILED|error" | head -20

echo ""
echo "========== 2. 编译 + 启动模拟器 =========="
# 如需走首次引导，取消下面一行注释
# rm -f fs_/.crosspoint/settings.json fs_/.crosspoint/state.json

platformio run -e simulator -t run_simulator 2>&1 | tail -25
