#!/bin/bash
# 只编译 + 启动模拟器
set -e

cd ~/crossmux
source .venv/bin/activate

echo "========== 编译 + 启动模拟器 (simulator) =========="

# 如需走首次引导，取消下面一行注释
# rm -f fs_/.crosspoint/settings.json fs_/.crosspoint/state.json

platformio run -e simulator -t run_simulator 2>&1 | tail -25
