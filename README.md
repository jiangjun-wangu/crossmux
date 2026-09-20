# CrossMux · 微雪 ESP32-S3-ePaper-3.97 中文移植

本仓库是 [0x1abin/crossmux](https://github.com/0x1abin/crossmux) 的移植分支，目标是把 CrossMux 移植到 **微雪 ESP32-S3-ePaper-3.97**（800×480、4 阶灰度墨水屏）上，打造一台中文阅读器。

上游英文 README 保留在 [README.upstream.md](./README.upstream.md)。

## 硬件

| 项 | 参数 |
|---|---|
| 主控 | ESP32-S3R8（双核 Xtensa LX7，240MHz） |
| 内存 | 512KB SRAM + 8MB PSRAM |
| Flash | 16MB（app 分区 6.4MB） |
| 屏幕 | 3.97 英寸，800×480，4 阶灰度，SSD1677 |
| 触摸 | 无，拨轮 + 侧边按键 |
| SD 卡 | 4-bit SDMMC，FAT32 |
| 传感器 | QMI8658 IMU、SHTC3 温湿度、PCF85063 RTC |
| 音频 | ES8311 + NS4150B + 麦克风 |
| 电源 | AXP2101 PMIC，支持 3.7V MX1.25 锂电池 |

## 快速开始

### 环境要求

- Linux / WSL2
- PlatformIO Core 6.2.0
- Python 3.14（项目自带 .venv）

### 编译 + 启动模拟器（默认流程）

cd ~/crossmux
source .venv/bin/activate
platformio run -e simulator -t run_simulator

### 编译真机固件

cd ~/crossmux
source .venv/bin/activate
platformio run -e waveshare_epaper_397

固件输出：.pio/build/waveshare_epaper_397/firmware.bin

### 一键脚本

| 脚本 | 用途 |
|---|---|
| build_simulator.sh | 只编译 + 启动模拟器 |
| build_firmware.sh | 只编译真机固件 |
| build_all.sh | 先真机后模拟器 |

### 烧录真机

platformio run -e waveshare_epaper_397 -t upload

> 注意：当前 Flash 占用 99.1%，剩余空间仅约 59 KB。烧录前请先完成模拟器验证。

## 本移植的主要改动

### 1. 内置字体替换为 MiSans（GB2312 全集）

- 内置中文字体从思源黑体替换为 **MiSans**
- 字符集：**GB2312 全集 6763 字**（一级 3755 + 二级 3008）
- 六个字号：8 / 10 / 12 / 14 / 16 / 18pt
- 编译进固件的只有 8 / 10 / 12pt（all.h 注册），14/16/18pt 保留源文件供 SD 字体脚本引用
- 字体生成脚本：lib/EpdFont/scripts/build-cn-builtin-fonts.sh

### 2. SD 卡阅读字体替换为 MiSans（全量 CJK）

- fs_/.fonts/MiSans/ 存 6 个 .cpfont 文件（8~18pt），**全量 CJK**（约 20992 字）
- 生成命令：

  python3 lib/EpdFont/scripts/fontconvert_sdcard.py --name MiSans --intervals cjk --sizes 8,10,12,14,16,18 --style regular lib/EpdFont/builtinFonts/source/NotoSansSC/NotoSansSC-Regular.otf --output-dir fs_/.fonts/MiSans/

### 3. 阅读 / UI 字体分工

| 场景 | 字体 | 来源 |
|---|---|---|
| 阅读正文 | MiSans 全量 | SD 卡 |
| 菜单 / 设置 / 汉字钟 / 状态栏 | MiSans GB2312 | 内置 Flash |

- **SD 优先模式**：sdFontFamilyName = "MiSans"
- **preload 关闭**：sdFontFlashPreload = 0
  - 模拟器不支持（HalOtaSlot::inactive() 恒空）
  - 真机 18pt 超限（cpfont 6.57 MB > 上限 6.55 MB）

### 4. UI 主题只保留 INX

- CLASSIC + INX 保留
- 删除 Lyra3CoversTheme / LyraCarouselTheme / RoundedRaffTheme 的实现与源文件
- LyraTheme 保留（InxTheme 继承自它）
- 设置菜单里「UI 主题」固定为 INX，用 valueGetter/valueSetter 拦截防止索引覆盖

### 5. 删除游戏与玩具 App

保留 6 个 App：文件传输、OPDS 浏览器、微信读书、AirPage、阅读统计、汉字钟

删除 11 个游戏目录：2048、数独、五子棋、推箱子、中国象棋、扫雷、像素切换、丑丑头像、伙伴、电子木鱼、计算器

同时清理：

- ActivityManager 里 11 个死 #include
- 11 个游戏图标文件（src/components/icons/）
- chinese_chess_16.h 字体（仅象棋使用）
- UIIcon 枚举里的游戏项
- LyraTheme.cpp / inx_apps.h 的游戏 case 和位图

### 6. i18n 精简到中英双语

- gen_i18n.py 用 CROSSPOINT_KEEP_LANGS=EN,ZH_CN 过滤，编译时只保留中英
- 从 YAML 源文件删除 196 条游戏字符串 + 3 条孤儿 key
- chinese.yaml / english.yaml 各 **903 条**键，完全对齐

### 7. 汉字钟（ZenHomeFace）

- 72pt 汉字字体（12 汉字 + 0-9 数字），文件 zen72.h
- 时间表达：
  - 小时：1-10 点「一」~「十」+「时」；11-19 点「十」「一」~「十」「九」；20 点「二」「十」；21-24 点「二」「一」~「二」「四」
  - 分钟：0「时」；1-9「零」「X」；10「一」「十」；11-19「十」「X」；20「二」「十」；21-29「二」「X」；30/40/50「三/四/五」「十」；31-59 对应十位 + 个位
- 布局：
  - 小字块占屏幕高度 11%，拆成两行（星期+时段 / 日期+时分）
  - 小字块严格垂直居中
  - 小时区上提 bigAscender / 3，上下留白再收窄 bigAscender / 9

### 8. 传统日历精简

- 删除 ChineseAlmanac 里的宜忌池（kYiPool / kJiPool）
- 删除 ChineseCalendarFace 里的 formatClash() / drawYiJiBox() / 宜忌卡片
- hero 数字改用 zen72 字体渲染

### 9. 键盘布局只保留英文 QWERTY

- KeyboardLayoutSet.h 的 ALL[] 只留 QwertyEn
- 设置里只显示 English，不可取消勾选

### 10. 主页无封面占位图

- InxRecentActivity::drawBookCover 无封面时改为**白底 + 居中 32×32 书本图标**
- 移除了「上半白 + 下半黑」的旧绘制

### 11. 其他

- 删除 33 个未使用的 notosans_* / notoserif_* 字体文件（不参与编译）
- 符号名 COMPLETE_CHINESE_NOTO_SANS_FAMILY 改为 COMPLETE_CHINESE_MISANS_FAMILY
- 默认布局：最近 / 书库走 List，Apps 走 Icons

## Flash / RAM 状态

| 项 | 值 |
|---|---|
| Flash | 6,494,063 / 6,553,600（99.1%） |
| 剩余 | 59,537 字节 |
| RAM | 30.3%（99,380 / 327,680） |
| IRAM | 100%（已满，不做 IRAM 优化） |

> Flash 剩余不足 60 KB。任何代码改动前请先评估体积。

## 设计说明：SD 字体不启用 preload

preload 机制会把选中的 SD 字体拷贝到未激活的 OTA 槽，加速阅读翻页。
本移植关闭该机制，原因：

- 模拟器环境下 HalOtaSlot::inactive() 返回空，preload 直接拒绝
- 真机 18pt cpfont（6.57 MB）超 6.55 MB 上限
- 真机 12/14/16pt 理论可行，但会占用 OTA 槽，导致后续 OTA 升级失效
- 本机 Flash 已用 99.1%，OTA 升级本就不可行，preload 的收益（e-ink 翻页
  瓶颈在屏幕刷新）也有限

当前策略：sdFontFlashPreload = 0，阅读走 SD 直读。

## 已知问题

1. **真机未烧录实测**
   - 当前所有改动仅经模拟器验证

## 关键文件路径

| 用途 | 路径 |
|---|---|
| 汉字钟 Face | src/activities/apps/standby/ZenHomeFace.{h,cpp} |
| 汉字钟 Activity | src/activities/apps/ZenClockActivity.{h,cpp} |
| 传统日历 | src/activities/apps/standby/ChineseCalendarFace.cpp |
| Apps 菜单 | src/activities/apps/AppsMenuActivity.cpp |
| ActivityManager | src/activities/ActivityManager.{h,cpp} |
| 字体 ID | src/fontIds.h |
| 内置字体 | lib/EpdFont/builtinFonts/misans_cjk_*.h、zen72.h |
| 字体生成脚本 | lib/EpdFont/scripts/build-cn-builtin-fonts.sh |
| i18n 生成 | scripts/gen_i18n.py |
| UI 主题 | src/components/UITheme.cpp、src/components/themes/ |
| INX 图标 | src/components/icons/inx_apps.h |
| 主页缩略图 | src/activities/home/InxRecentActivity.cpp |
| 键盘布局表 | src/activities/util/KeyboardLayoutSet.h |
| SD 阅读字体 | fs_/.fonts/MiSans/（需自行生成，不随仓库分发） |
| 模拟器书籍 | fs_/books/ |

## 相关文档

- [上游 README（英文）](./README.upstream.md)
- [用户指南](./USER_GUIDE.md)
- [上游仓库](https://github.com/0x1abin/crossmux)

## 许可与来源

本仓库基于 [0x1abin/crossmux](https://github.com/0x1abin/crossmux)，后者是 [CrossPoint Reader](https://github.com/crosspoint-reader/crosspoint-reader) 的社区分支。许可条款以原项目为准。

### MiSans 字体

本移植使用小米 **MiSans** 字体作为内置中文字体。MiSans 字体采用《MiSans 字体知识产权许可协议》，允许免费商用，但要求在软件中特别注明使用了 MiSans 字体。本项目已在 README 中注明，符合许可条件。

更多信息请访问 [MiSans 字体常见问题](https://hyperos.mi.com/font/faq)。
