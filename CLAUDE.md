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
python3 lib/EpdFont/scripts/fontconvert_sdcard.py --name MiSans --intervals cjk --sizes 18,20,24 --style regular lib/EpdFont/builtinFonts/source/NotoSansSC/NotoSansSC-Regular.otf --output-dir /tmp/misans/

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

## 当前状态（2026-09-22）

### 硬件

ESP32-S3R8 / 512KB SRAM + 8MB PSRAM / 16MB Flash（app 6.4MB）/ 3.97" 800x480 4 阶灰度 / 无触摸

### 编译

- Flash：6,494,867 / 6,553,600（99.1%）
- 剩余：58,733 字节
- RAM：30.3%
- IRAM：100%（不做 IRAM 优化）
- Flash 剩余不足 60 KB，改动前评估体积

### 字体

| 场景 | 字体 | 位置 |
|---|---|---|
| 内置 UI | MiSans GB2312 6763 字，8/10/12pt | misans_cjk_*.h |
| SD 阅读 | MiSans 全量 CJK，18/20/24pt | 真机 SD /fonts/MiSans/ |
| 汉字钟 | zen72.h 72pt 12 汉字 | zen72.h |
| 状态栏数字 | ubuntu_10/12 | ubuntu_*.h |
| 小字号 | notosans_8_regular | notosans_8_regular.h |

### 关键设置

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
| 内置字体 | lib/EpdFont/builtinFonts/misans_cjk_*.h、zen72.h |
| 字体生成 | lib/EpdFont/scripts/build-cn-builtin-fonts.sh |
| SD 字体生成 | lib/EpdFont/scripts/fontconvert_sdcard.py |
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

## 设计取舍（非 bug）

- SD 字体 preload 不可用：模拟器 HalOtaSlot::inactive() 恒空；真机 18/20/24pt 超 6.55 MB 上限。走 SD 直读。
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
- 长 heredoc 容易截断，用 cat >> 分小段追加
