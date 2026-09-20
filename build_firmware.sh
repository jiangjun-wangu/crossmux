#!/bin/bash
# 只编译真机固件
set -e

cd ~/crossmux
source .venv/bin/activate

echo "========== 编译真机固件 (waveshare_epaper_397) =========="
platformio run -e waveshare_epaper_397 2>&1 | grep -E "gen_i18n|Flash:|RAM:|SUCCESS|FAILED|error" | head -20

echo ""
echo "===== 固件输出 ====="
ls -la .pio/build/waveshare_epaper_397/firmware.bin
