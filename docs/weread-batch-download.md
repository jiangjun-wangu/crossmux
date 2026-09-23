# 微信读书多选批量下载 · 新会话启动文档

## 启动指令

继续 CrossMux 微信读书「多选批量下载」功能。
项目路径：~/crossmux
我这次要做：【微信读书多选批量下载】

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
