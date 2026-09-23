# 微信读书多选批量下载 · 新会话启动文档

## 启动指令

继续 CrossMux 微信读书「多选批量下载」功能。

**开工前必读（按顺序）：**

1. ~/crossmux/docs/weread-batch-download.md（本文件 · 任务自包含文档）
2. ~/crossmux/CLAUDE.md（完整开发规范 · 强制约定/工作流/字体/动画/编译历史）

读完后：
- 按本文件「十一、新会话第一件事」执行（先跑探测命令）
- 遵守 CLAUDE.md 的强制约定（模拟器优先 / 备份 / printf 写文档 / 五步提交）
- 遵守安全红线（eFuse / Secure Boot / Flash 加密 / 生产模式 禁写）

项目路径：~/crossmux
工作环境：WSL，source .venv/bin/activate
我这次要做：【微信读书多选批量下载】

## 〇、项目概览

| 项 | 值 |
|---|---|
| 目标 | CrossMux 移植到微雪 ESP32-S3-ePaper-3.97（中文墨水屏阅读器）|
| 项目路径 | ~/crossmux（WSL 原生文件系统）|
| 自己的 fork | https://github.com/jiangjun-wangu/crossmux |
| 虚拟环境 | source .venv/bin/activate |
| 备份目录 | ~/crossmux_backups/YYYYMMDD_HHMMSS[_说明]/ |

### 硬件

- ESP32-S3R8（双核 Xtensa LX7，240MHz）
- 512KB SRAM + 8MB PSRAM
- 16MB Flash（app 分区 6.4MB）
- 3.97 英寸 800×480 4 阶灰度 SSD1677
- 无触摸，滚轮（上下拨+按下+长按）+ 侧边按键

### 编译状态（2026-09-23）

| 项 | 值 |
|---|---|
| Flash | 6,427,659 / 6,553,600（98.1%，剩 126 KB）|
| RAM | 30.3%（99,380 / 327,680）|
| IRAM | 100%（已满，不做优化）|
| 最新 commit | 0a253713 |

**Flash 剩余不足 130 KB，改动前评估体积。**

### 核心命令

只编译模拟器：

    clear
    cd ~/crossmux
    source .venv/bin/activate
    platformio run -e simulator -t run_simulator

编译真机固件 + 打开文件夹：

    clear
    cd ~/crossmux
    source .venv/bin/activate
    time platformio run -e waveshare_epaper_397 && {
      FIRMWARE_DIR=".pio/build/waveshare_epaper_397"
      WIN_PATH=$(wslpath -w "$(realpath "$FIRMWARE_DIR")")
      ls -la "$FIRMWARE_DIR"/*.bin
      echo "$WIN_PATH"
      explorer.exe "$WIN_PATH" 2>/dev/null || true
    }

提交代码：

    cd ~/crossmux
    git add -A
    git commit -m "描述"
    git push origin main

---

## 一、需求

### 交互

**书架（普通模式）**
- 滚轮上下：移动光标
- 滚轮按下：打开详情
- 滚轮长按：进入多选模式（新增）

**书架（多选模式）**
- 顶部提示：已选 N 本 + 长按开始下载
- 滚轮上下：移动光标
- 滚轮按下：勾选/取消
- 滚轮长按：开始连续下载
- 侧键 Back：退出多选

### 下载行为
- 遍历勾选的书，每本复用单本下载逻辑
- 成功 → 继续下一本
- 失败 → 跳过，继续下一本
- 全部完成 → 汇总成功 N 本 / 失败 M 本
- 下载中 Back → 停止，已完成保留

## 二、代码现状

关键文件：
- src/activities/apps/weread/webapi/WeReadActivity.cpp (2691 行)
- src/activities/apps/weread/webapi/WeReadActivity.h (178 行)
- src/activities/apps/weread/WeReadBackend.h
- src/components/icons/weread.h

核心机制：
1. 下载从详情页触发（书架确认 → openSelectedDetail → 详情页选缓存 → startBookDownload）
2. 异步 Job 模型（startJob 立即返回，loop 里 advanceJob 轮询）
3. operation_ 单例，不能并发 → 批量下载必须串行队列

关键函数：
- startBookDownload()：WiFi 已连 → startJob(Download)；否则 connectThen
- startJob()：初始化进度原子量，调 operation_.begin()
- activateSelected()：书架确认 → openSelectedDetail
- handleShelfInput()：处理滚轮短按/长按/触摸/滑动/Confirm

关键成员：
- std::atomic<int> shelfSelected_
- uint32_t shelfCount_
- WeReadStore::ShelfRecord pendingBook_
- WeReadClient::Operation operation_
- std::atomic<State> state_

## 三、待探测（新会话第一件事）

跑以下命令后开始：

    clear
    cd ~/crossmux
    echo "=== advanceJob ==="
    awk '/void WeReadActivity::advanceJob/,/^}/' src/activities/apps/weread/webapi/WeReadActivity.cpp
    echo "=== loop 主循环 ==="
    sed -n '1852,1960p' src/activities/apps/weread/webapi/WeReadActivity.cpp
    echo "=== Operation 接口 ==="
    grep -n "class Operation|struct Event|enum.*ProgressStage|enum.*Kind" src/activities/apps/weread/WeReadBackend.h
    echo "=== ShelfRecord ==="
    grep -n "struct ShelfRecord" -A 20 src/activities/apps/weread/WeReadBackend.h

探测目的：
1. advanceJob 里下载完成做什么 → 一本完成切入点
2. loop 如何驱动状态机 → 队列插入位置
3. Operation 生命周期 → 能否串行复用
4. ShelfRecord 字段 → 勾选状态存哪

## 四、实现方案

阶段 1：探测（30 min）

阶段 2：多选状态（1-2 h）
- 加成员：bool shelfMultiSelect_；std::vector<uint32_t> shelfCheckedBookIds_
- 长按 Confirm 进入多选
- 多选下短按 Confirm = 切换勾选
- 多选下长按 Confirm = 开始批量下载
- Back = 退出多选

阶段 3：批量队列（2-3 h）
- 加成员：std::vector<ShelfRecord> downloadQueue_；size_t queueIndex_；成功/失败计数
- startBatchDownload()：填队列，调 startNextBatchItem()
- startNextBatchItem()：取下一本，调 startJob(Download)
- advanceJob 完成钩子：成功++ → 下一本；失败++ → 下一本；队列空 → 汇总

阶段 4：UI（1-2 h）
- drawShelfGrid 多选态显示复选框
- 顶部提示
- State::Downloading 显示批量进度

阶段 5：i18n + 测试（1-2 h）

关键技术点：
1. 串行队列（operation_ 单例）
2. WiFi 保持（wifiSessionActive_）
3. 中断：Back 清队列，保留已完成
4. 状态恢复：倾向保持 Downloading，不每次刷书架

## 五、强制约定

> 完整规范见 ~/crossmux/CLAUDE.md 的「工作流规范（2026-09-23 固化）」段落。


- 模拟器优先
- 编译真机后打开文件夹
- 脚本 clear 开头
- 大改动前备份 ~/crossmux_backups/YYYYMMDD_HHMMSS/
- eFuse / Secure Boot / Flash 加密 / 生产模式禁写
- 每轮 1-3 问题
- 先探测再动手
- 文档写入用 printf 逐行追加（禁止 heredoc，见 CLAUDE.md「文档追加/新建规范」）
- 提交五步：备份 → add → commit → push → log 确认

## 六、编译状态（2026-09-23）

| 项 | 值 |
|---|---|
| Flash | 6,427,659 / 6,553,600（98.1%，剩 126 KB）|
| RAM | 30.3% |
| IRAM | 100% |
| commit | 0a253713 |

Flash 剩 <130 KB，改动前评估体积。

## 七、用户偏好

- 中文交流
- 脚本 clear 开头
- 编译真机后打开文件夹
- 模拟器优先
- 每轮 1-3 问题
- 不喜一次抛太多
- 命令必须直接可跑
- 文档写入用 printf 逐行追加，禁止 heredoc

---

文档版本：2026-09-23

## 九、工作流规范（继承自 CLAUDE.md）

### 提交规范（固定五步）

1. 里程碑备份 → ~/crossmux_backups/YYYYMMDD_HHMMSS_说明/
2. git add -A
3. git commit -m "..."（分点说明改动）
4. git push origin main
5. git log --oneline -3 + git status --short 确认

### 文档写入规范

**用 printf 逐行追加，禁止 heredoc。**

原因：heredoc 在 WSL 会被截断，终端卡在 > 或内容丢失。

标准写法：

    # 新建（每次一小段）
    printf '%s
' '第一行' '第二行' > /tmp/doc.md

    # 追加
    printf '%s
' '追加内容' >> /tmp/doc.md

    # 复制到目标
    cp /tmp/doc.md docs/xxx.md

    # 验证
    wc -l docs/xxx.md

禁止：
- cat > file <<EOF ... EOF
- python3 -c "..." < <(cat <<MARKER ...)
- 一次追加超过 20 行

### 编译记录规范

每次编译完成后记录：时间 / 类型 / 耗时 / Flash / RAM / IRAM / 相对上次变化 / commit。

记录位置：CLAUDE.md「编译历史」段（滚动保留最近 10 次）。

### 烧录规范

- **默认只烧 firmware.bin → 0x10000**
- 分区表和 bootloader 未改，不需要重烧
- 项目自带烧录工具会因 crossmux-sticky-v1 manifest 标签拒绝 app-only 烧录
- 改用 flash_download_tool 手动烧：
  - ESP32-S3 / Develop / DIO / 80MHz / 16MB / 起始 0x10000
- 连 partitions.bin 一起烧会清空 NVS（WiFi、设置、进度）

## 十、安全红线（绝对禁止）

| 操作 | 读 | 写 |
|---|---|---|
| eFuse | 允许 | **绝对禁止** |
| Secure Boot | 允许 | **绝对禁止** |
| Flash 加密 | 允许 | **绝对禁止** |
| 生产模式 | 允许 | **绝对禁止** |

翻页动画专项安全约束（不适用于本次功能，但需知晓）：

- 连续局刷 <= 10 次
- 单次动画 <= 15s
- 不使用自定义 LUT / 波形

## 十一、新会话第一件事

**跑文档「三、待探测项」的命令，贴回输出。**
**不要跳过探测直接写代码。**

探测输出会告诉我：
1. advanceJob 完成钩子在哪
2. loop 状态机如何驱动
3. Operation 能否串行复用
4. ShelfRecord 字段结构

然后按「四、实现方案」阶段 2-5 逐步实现。

---

文档版本：2026-09-23
对应 commit：0a253713

---

## 十二、实现完成记录（2026-09-23）

功能已完成，模拟器 + 真机编译通过。

### 入口

管理 Tab → 多本下载（新增菜单项）

### 交互

- 滚轮上下：移动光标（只遍历未下载书籍）
- 短按确认：勾选 / 取消
- 长按确认（700ms）：弹确认框 → 开始批量下载
- 下载中按 Back：中断，已完成保留
- 全部完成：弹汇总「成功 N / 失败 M」

### 已下载书自动隐藏

进入多本下载时，先扫一遍书架，跳过 Storage.exists(finalBookPath) 为真的书。
若全部已下载，弹「没有未下载书籍」。

### 关键改动文件

- src/activities/apps/weread/webapi/WeReadActivity.{h,cpp}
- lib/I18n/translations/{chinese,english}.yaml

### 关键成员

- std::set<int> batchSelectedIndexes_   勾选的书架索引
- std::vector<ShelfRecord> batchQueue_   下载队列
- std::vector<int> batchVisibleIndexes_  未下载书籍的书架索引
- size_t batchQueueIndex_                当前队列位置
- uint32_t batchSuccess_ / batchFailed_  成功/失败计数

### 编译状态

- Flash: 6,430,407 / 6,553,600（98.1%，剩 123 KB）
- RAM: 99,380 / 327,680（30.3%）
- commit: 3782dbfa（提交前）
