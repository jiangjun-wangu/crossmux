# CrossMux 微雪移植 · Claude 工作记忆

> 给 AI 助手的项目记忆。每次新对话开始时先读这份文件。

## 一句话
把 CrossMux 移植到微雪 ESP32-S3-ePaper-3.97，打造中文墨水屏阅读器。

## 工作目录
- 项目：`~/crossmux`
- 虚拟环境：`source .venv/bin/activate`
- 自己的 fork：https://github.com/jiangjun-wangu/crossmux
- 上游：https://github.com/0x1abin/crossmux
- 备份：`~/crossmux_backups/YYYYMMDD_HHMMSS[_milestone/_final]/`

## 强制约定

1. **默认模拟器优先**：除非明确说「烧录真机」或「直接编译真机」，一律先跑模拟器验证
2. **编译真机后必须打开固件文件夹**：用户用 Windows 端工具烧录，不通过 WSL
3. **脚本以 clear 开头**
4. **大改动前先备份**：`~/crossmux_backups/YYYYMMDD_HHMMSS/`
5. **eFuse 只读禁写**：eFuse / Secure Boot / Flash 加密 / 生产模式 **绝对禁止写入**
6. **每轮只问 1-3 个必要问题**
7. **先探测再动手**：改代码前先 grep / cat 确认现状
8. **长 heredoc 会截断**：多行内容用 Python 分段写入，不用 `cat << EOF`
9. **每次新功能结束必须更新 CLAUDE.md 和 README.md**


## 核心命令

### 只编译模拟器

clear
cd ~/crossmux
source .venv/bin/activate
platformio run -e simulator -t run_simulator

### 编译真机固件 + 打开文件夹（标准流程）

clear
cd ~/crossmux
source .venv/bin/activate
time platformio run -e waveshare_epaper_397 && {
  FIRMWARE_DIR=.pio/build/waveshare_epaper_397
  WIN_PATH=$(wslpath -w "$(realpath "$FIRMWARE_DIR")")
  echo === 固件文件 ===
  ls -la "$FIRMWARE_DIR"/*.bin
  echo === Windows 路径 ===
  echo "$WIN_PATH"
  echo === 打开文件夹 ===
  explorer.exe "$WIN_PATH" 2>/dev/null || true
}

### 挂载真机 SD 卡（F 盘）

sudo mkdir -p /mnt/f
sudo mount -t drvfs F: /mnt/f -o metadata,uid=1000,gid=1000,umask=022

### 生成 SD 卡字体（真机 SD 卡用）

cd ~/crossmux
source .venv/bin/activate
python3 lib/EpdFont/scripts/fontconvert_sdcard.py --name MiSans --intervals cjk --sizes 8,10,12,14,16,18,20,22,24,26,28,30,32,34,36 --style regular lib/EpdFont/builtinFonts/source/MiSans/MiSans-Regular.ttf --output-dir /tmp/misans/

### 生成内置字体（编译进固件）

cd ~/crossmux
source .venv/bin/activate
bash lib/EpdFont/scripts/build-cn-builtin-fonts.sh

### 提交代码

cd ~/crossmux
git add -A
git commit -m "描述"
git push origin main

- remote 已改为自己的 fork
- 已配 credential.helper store，只需第一次输 token
- token 需 Contents (RW) + Workflows (RW) 权限

## 硬件

ESP32-S3R8 / 512KB SRAM + 8MB PSRAM / 16MB Flash（app 6.4MB）/ 3.97" 800x480 4 阶灰度 / 无触摸

## 关键设置

- sdFontFamilyName = "MiSans"
- sdFontFlashPreload = 0（超限时静默跳过并弹专用文案）
- uiTheme = 5（INX）
- 键盘只保留英文 QWERTY
- UI 主题只保留 CLASSIC + INX
- 已删 11 个游戏 App + 3 个未用主题 + 33 个未用拉丁字体

## 关键文件路径

| 用途 | 路径 |
|---|---|
| 汉字钟 Face | src/activities/apps/standby/ZenHomeFace.{h,cpp} |
| 汉字钟 Activity | src/activities/apps/ZenClockActivity.{h,cpp} |
| 传统日历 | src/activities/apps/standby/ChineseCalendarFace.cpp |
| Apps 菜单 | src/activities/apps/AppsMenuActivity.cpp |
| 阅读字体设置 | src/activities/settings/TextSettingsActivity.cpp |
| 字体 ID | src/fontIds.h |
| 内置字体 | lib/EpdFont/builtinFonts/misans_cjk_*.h、mi72.h |
| 字体生成 | lib/EpdFont/scripts/build-cn-builtin-fonts.sh |
| SD 字体生成 | lib/EpdFont/scripts/fontconvert_sdcard.py |
| SD 字体文档 | docs/misans-sd-fonts.md |
| i18n 生成 | scripts/gen_i18n.py |
| i18n 中文 | lib/I18n/translations/chinese.yaml |
| i18n 英文 | lib/I18n/translations/english.yaml |
| UI 主题 | src/components/UITheme.cpp |
| INX 图标 | src/components/icons/inx_apps.h |
| 主页缩略图 | src/activities/home/InxRecentActivity.cpp |
| 封面加载 | src/util/BookCoverLoader.{h,cpp} |
| 键盘布局 | src/activities/util/KeyboardLayoutSet.h |
| 网络 | src/activities/network/、src/network/ |
| 设置默认值 | src/CrossPointSettings.h |
| 模拟器字体 | fs_/.fonts/MiSans/ |
| 模拟器书籍 | fs_/books/ |

## 已修复 / 已确认

1. 封面截断缓存：BookCoverLoader::isValidBmp 拒绝头声明大小 > 实际文件大小的 BMP，避免 "Failed to read crop-fill row 0"
2. preload 超限提示：TextSettingsActivity::exitAfterFinalFont 加 cpfont 大小检查，超缓存容量时静默跳过，弹 STR_FONT_PRELOAD_TOO_LARGE（文案「字体超过 Flash 剩余空间，已加载 SD 卡字体文件」）
3. SD 卡缺失不再变砖：FullScreenMessageActivity 加 loop()，任意按键触发 ESP.restart()；文案改为中英双语 STR_SD_CARD_MISSING

## 设计取舍（非 bug）

- SD 字体 preload 不可用：模拟器 HalOtaSlot::inactive() 恒空；真机 20pt 及以上超 6.55 MB 上限。走 SD 直读。
- 主页三体封面略糊：上游 JPEG 转换器 bug，87x146 尺寸失败。二级 fallback 到 226/300pt 版本正常显示。

## 安全红线

| 操作 | 读 | 写 |
|---|---|---|
| eFuse | 允许 | 绝对禁止 |
| Secure Boot | 允许 | 绝对禁止 |
| Flash 加密 | 允许 | 绝对禁止 |
| 生产模式 | 允许 | 绝对禁止 |

## 用户偏好

- 中文交流
- 脚本 clear 开头
- 编译真机后打开文件夹
- 模拟器优先
- 大改动前备份
- 每轮 1-3 个问题
- 不喜欢一次抛太多信息
- 所有操作都在终端，命令必须直接可跑
- **追加文档一律用 Python 脚本**（写 .py 文件再运行），不用 heredoc / cat >>

## 工作流规范（2026-09-23 固化）

### 提交规范（固定五步）

1. 里程碑备份 → `~/crossmux_backups/YYYYMMDD_HHMMSS_<说明>/`
2. `git add -A`
3. `git commit -m "..."`（分点说明改动）
4. `git push origin main`
5. `git log --oneline -3` + `git status --short` 确认

### 文档追加规范

**一律用 Python 脚本**（写 `.py` 文件再运行），不用 heredoc / `cat >>`。
原因：长 heredoc 在 WSL 会话中会被截断，导致内容丢失。

### 烧录规范

- **默认只烧 `firmware.bin` → `0x10000`**
- 分区表（`partitions.csv`）和 bootloader 未改，不需要重烧
- ⚠️ 项目自带烧录工具会因 `crossmux-sticky-v1` manifest 标签拒绝 app-only 烧录（设备 NVS 没这个标签）；改用 **flash_download_tool** 手动烧
- flash_download_tool 参数：ESP32-S3 / Develop / DIO / 80MHz / 16MB / 起始 `0x10000`
- 如果连 `partitions.bin` 一起烧会清空 NVS（WiFi、设置、进度）

### 字体状态（2026-09-23）

- 全系统统一**官方小米 MiSans**（MD5 验证，非改名冒充）
- 源：`lib/EpdFont/builtinFonts/source/MiSans/MiSans-{Regular,Medium,Bold}.ttf`
- 内置：`misans_latin_*` / `misans_cjk_*` / `mi72.h`
- SD：`/fonts/MiSans/MiSans_<size>.cpfont`
- ID 宏：`SANS_*` / `SERIF_*`（原 NOTOSANS/NOTOSERIF，值未变）
- 已删除：NotoSans / NotoSerif / NotoSansSC / NotoSansHebrew / NotoSansArabic / Ubuntu / OpenDyslexic

### i18n 状态（2026-09-23）

- **只剩中英双语**（`chinese.yaml` + `english.yaml`）
- 已删 32 个未用语言 YAML
- `Language` 枚举仅 `EN` / `ZH_CN`
- 脚本层硬编码：`gen_i18n.py` 设 `CROSSPOINT_KEEP_LANGS=EN,ZH_CN`

### 当前编译状态

| 项 | 值 |
|---|---|
| Flash | 6,427,659 / 6,553,600（98.1%，剩 126KB）|
| RAM | 30.3%（99,380 / 327,680）|
| IRAM | 100%（已满，不做 IRAM 优化）|
| 最新 commit | baa371b2 |

⚠️ Flash 剩余不足 130 KB，改动前评估体积。

### 编译记录规范（2026-09-23 固化）

**每次编译完成后，必须记录：**

| 字段 | 示例 |
|---|---|
| 日期时间 | 2026-09-23 00:55 |
| 编译类型 | 真机 / 模拟器 |
| 耗时 | 1m33s |
| Flash | 6,427,659 / 6,553,600（98.1%，剩 126 KB）|
| RAM | 99,380 / 327,680（30.3%）|
| IRAM | 16,384 / 16,384（100%）|
| 相对上次变化 | Flash -736 B（-0.01%）|
| 触发 commit | baa371b2 |

变化率 = (本次 - 上次) / 上次 × 100%。

**记录位置**：CLAUDE.md「编译历史」段（滚动保留最近 10 次）。

### 编译历史

| 时间 | 类型 | 耗时 | Flash | 占用率 | RAM | 变化 | commit |
|---|---|---|---|---|---|---|---|
| 2026-09-23 00:55 | 真机 | 1m33s | 6,427,659 | 98.1% | 30.3% | -736 B | baa371b2 |
| 2026-09-23 00:21 | 真机 | 1m21s | 6,428,395 | 98.1% | 30.3% | -864 B | b0e9a8d2 |
| 2026-09-22 22:42 | 真机 | 1m45s | 6,429,499 | 98.1% | 30.3% | — | 4773e9c1 |

### 翻页动画安全约束（不可绕过）

- 单次动画连续局刷 ≤ 10（供应商一致建议）
- 单次动画总时长 ≤ 15s
- 连续 10 次局刷后强制全刷
- 不使用自定义 LUT / 波形，不写 eFuse

## 翻页动画（2026-09-22 新增）

安全版翻页动画，三种阅读格式（EPUB / TXT / XTC）均已支持。

### 配置项

| 设置 | 默认值 | 说明 |
|---|---|---|
| pageTurnAnimMode | PAGE_TURN_OFF | 0=关闭 / 1=卷轴 / 2=百叶窗 |
| pageTurnAnimSpeed | ANIM_SPEED_NORMAL | 5 档 |

速度档位与条带数（条带越多越平滑、越慢）：

| 档位 | 条带数 |
|---|---|
| VERY_FAST | 1（整屏一次刷） |
| FAST | 2 |
| NORMAL | 4 |
| SLOW | 6 |
| VERY_SLOW | 8 |

设置入口：设置 -> 阅读 -> 翻页动画 / 动画速度

### 关键文件

| 用途 | 路径 |
|---|---|
| 动画引擎 | src/util/PageTurnAnimator.{h,cpp} |
| 窗口刷新 HalDisplay | lib/hal/HalDisplay.{h,cpp} |
| 窗口刷新 GfxRenderer | lib/GfxRenderer/GfxRenderer.{h,cpp} |
| 设置字段 | src/CrossPointSettings.{h,cpp} |
| 设置菜单项 | src/SettingsList.h |
| EPUB 集成 | src/activities/reader/EpubReaderActivity.cpp |
| TXT 集成 | src/activities/reader/TxtReaderActivity.cpp |
| XTC 集成 | src/activities/reader/XtcReaderActivity.cpp |

### 工作原理

底层复用 SSD1677 驱动已存在的 displayWindow 窗口局部刷新，
无需新增控制器命令、LUT、波形。每次翻页把新页按条带逐块刷到面板：

1. 新页已渲染到 frameBuffer
2. frameBufferActive（单缓冲模式下由控制器 RED RAM 持有）保留上一页
3. 逐条带调 renderer.displayWindow(0, y, W, h) 做 FAST 局刷
4. 面板逐步从旧页过渡到新页

### 安全约束（写入代码，不可绕过）

- MAX_PARTIAL_BEFORE_FULL = 8   连续局刷上限（条带数上限）
- MAX_ANIMATION_TOTAL_MS  = 15000 动画总时长上限
- CANCEL_GRACE_MS         = 300  动画开始后不响应取消的宽限期

8 像素 X 对齐由 GfxRenderer::screenRectToAlignedMemRect 自动处理；
越界/未对齐由 Ssd1677Driver::displayWindow 拒绝。
连续 8 条带后由刷新计数触发一次全刷清残影。

### 中断处理

翻页动画进行中按任意键，立即停止剩余条带并做一次 HALF 全刷清残影。
前 300ms 宽限期不响应，避免翻页键的 press-edge 误取消。

### 模拟器限制

模拟器 SDK 的 HalDisplay::displayWindow 忽略坐标、不做 present，
因此模拟器里看不到逐条带效果，只能看日志中的 [ANIM] start/done。
真机才有真实墨水屏刷新时间。


## 字体架构（2026-09-23 更新）

全系统统一 **小米 MiSans**（官方版，MD5 验证）。

### 内置字体（Flash，随固件）

| 用途 | 文件 | 来源 |
|---|---|---|
| UI 拉丁 8/10/12pt | misans_latin_*.h | MiSans-Regular/Medium/Bold.ttf |
| CJK 8/10/12pt | misans_cjk_8/10/12.h | MiSans-Regular.ttf（3500 常用字）|
| CJK 14/16/18pt | misans_cjk_14/16/18.h | MiSans-Regular.ttf（i18n 字）|
| 汉字钟 72pt | mi72.h | MiSans-Regular.ttf（12 汉字）|

### 源字体位置与校验

    lib/EpdFont/builtinFonts/source/MiSans/
      MiSans-Regular.ttf   MD5 f290c996741aa1c8775d8c28372608af
      MiSans-Medium.ttf    MD5 d4ea974a987217b683b90c764fdff31f
      MiSans-Bold.ttf      MD5 9b9b94c00eb740134af55ff456235cbe

来源：https://hyperos.mi.com/font/zh/download/ 的 MiSans.zip
版权：Copyright (c) 2020-2025 Beijing Xiaomi Mobile Software Co.,Ltd.

### SD 卡字体（运行时加载，非固件）

| 项 | 说明 |
|---|---|
| 目录 | /fonts/MiSans/ 或 /.fonts/MiSans/ |
| 文件 | MiSans_<size>.cpfont（size 8-36）|
| 生成 | fontconvert_sdcard.py --name MiSans --intervals cjk |
| 源 | source/MiSans/MiSans-Regular.ttf |

### 字体 ID 宏改名（值未变，settings.bin 兼容）

| 旧 | 新 |
|---|---|
| NOTOSANS_12/14/16/18_FONT_ID | SANS_12/14/16/18_FONT_ID |
| NOTOSERIF_12/14/16/18_FONT_ID | SERIF_12/14/16/18_FONT_ID |
| CrossPointSettings::NOTOSANS | FONT_SANS |
| CrossPointSettings::NOTOSERIF | FONT_SERIF |
| STR_NOTO_SANS / STR_NOTO_SERIF | STR_FONT_SANS / STR_FONT_SERIF |

### 已删除的字体源

- NotoSans / NotoSerif / NotoSansSC / NotoSansHebrew / NotoSansArabic
- Ubuntu / OpenDyslexic
- 备份在 ~/crossmux_backups/20260923_*_noto_source_backup/ 等

说明：上游标为 NotoSansSC 的文件经验证与 MiSans 字形完全一致
（glyph count 29758 / glyph order / 轮廓哈希全部相同），实际本就是
MiSans 重命名。现已全部换成官方 MiSans 源。

### 生成脚本

| 脚本 | 用途 |
|---|---|
| build-cn-builtin-fonts.sh | 内置 CJK 6 档（源 MiSans-Regular.ttf）|
| build-font-ids.sh | 生成 fontIds.h |
| fontconvert_sdcard.py | SD 卡 cpfont |
| build-sd-fonts.py | SD 字体批量构建（fallback MiSans）|
