# MiSans 中文字体（SD 卡）

本移植把 **MiSans** 作为唯一中文字体。UI 中文字形内嵌 Flash（8/10/12pt，
GB2312 全集），阅读正文走 SD 卡 cpfont，覆盖 **8–36pt 全量 CJK**（约 20992 字）。

## 一、SD 卡路径

字体族目录名固定为 `MiSans`。SD 卡上有两个可选根目录：

| 根目录 | 说明 |
|---|---|
| `/.fonts/` | 隐藏目录（推荐，保持 SD 卡根目录整洁） |
| `/fonts/` | 可见目录（Windows 资源管理器默认不显示隐藏目录时用） |

两个根目录都会被扫描并合并。同一个字体族在两处同时存在时，`/.fonts/` 优先。

### 目录结构

    SD Card Root/
    ├── .fonts/                     ← 隐藏根（推荐）
    │   └── MiSans/
    │       ├── MiSans_8.cpfont
    │       ├── MiSans_10.cpfont
    │       └── ...
    └── fonts/                      ← 可见根（同样有效）
        └── MiSans/
            └── MiSans_*.cpfont

文件名必须严格遵循 `<family>_<size>.cpfont`：
- `<family>` = 目录名（`MiSans`）
- `<size>` = 点大小（整数，8–36）

不匹配的文件名会被忽略。


## 二、字号规格表

本移植提供 **15 档偶数点大小**（8–36pt）：

| 字号 | 文件名 | 文件大小 | 用途建议 |
|---|---|---|---|
| 8pt | MiSans_8.cpfont | 1.6 MB | 极小字号 |
| 10pt | MiSans_10.cpfont | 2.2 MB | 小字号 |
| 12pt | MiSans_12.cpfont | 3.1 MB | 默认字号 |
| 14pt | MiSans_14.cpfont | 4.0 MB | |
| 16pt | MiSans_16.cpfont | 5.1 MB | |
| 18pt | MiSans_18.cpfont | 6.3 MB | 大字阅读起步 |
| 20pt | MiSans_20.cpfont | 7.6 MB | |
| 22pt | MiSans_22.cpfont | 9.1 MB | |
| 24pt | MiSans_24.cpfont | 10.7 MB | |
| 26pt | MiSans_26.cpfont | 12.4 MB | |
| 28pt | MiSans_28.cpfont | 14.4 MB | |
| 30pt | MiSans_30.cpfont | 16.4 MB | |
| 32pt | MiSans_32.cpfont | 18.6 MB | |
| 34pt | MiSans_34.cpfont | 20.9 MB | |
| 36pt | MiSans_36.cpfont | 23.4 MB | 最大字号 |

**总计约 168 MB。**

完整 CJK 覆盖（24 个 Unicode 区间，约 20992 字形）：

- CJK 符号、平假名、片假名、注音符号
- CJK 统一表意文字（U+4E00–U+9FFF）
- CJK 兼容表意文字（U+F900–U+FAFF）
- 全角字母、数字、标点（U+FF00–U+FFEF）



## 三、获取 MiSans 源文件

本仓库**不包含** MiSans 源字体和 `.cpfont` 文件（体积过大，且字体有独立授权）。

### 1. 下载 MiSans

官方地址：**https://hyperos.mi.com/font/zh/download/**

下载 **MiSans Regular** 字重，解压得到 `MiSans-Regular.ttf`。

### 2. 放入项目路径

把下载的字体复制到：

    lib/EpdFont/builtinFonts/source/NotoSansSC/NotoSansSC-Regular.otf

**文件名保持不变**（`NotoSansSC-Regular.otf`），只替换内容。生成脚本按这个路径读。

**Linux / WSL**：

    cp /mnt/c/Users/你的用户名/Downloads/MiSans/MiSans/ttf/MiSans-Regular.ttf \
       ~/crossmux/lib/EpdFont/builtinFonts/source/NotoSansSC/NotoSansSC-Regular.otf

**Windows PowerShell**：

    Copy-Item "$env:USERPROFILE\Downloads\MiSans\MiSans\ttf\MiSans-Regular.ttf" `
      "\\wsl.localhost\Ubuntu\home\你的用户名\crossmux\lib\EpdFont\builtinFonts\source\NotoSansSC\NotoSansSC-Regular.otf"

### 3. 验证字体身份

    python3 -c "
    from fontTools.ttLib import TTFont
    f = TTFont('lib/EpdFont/builtinFonts/source/NotoSansSC/NotoSansSC-Regular.otf')
    print('字体名:', f['name'].getDebugName(1))
    "

预期输出：`字体名: MiSans`


## 四、生成 cpfont 字体文件

### 前置依赖

需要 Python 3 + 两个库：

    pip install freetype-py fonttools

**Linux / WSL**：用项目自带的 venv：

    cd ~/crossmux
    source .venv/bin/activate

**Windows PowerShell**：需要自己装 Python 3.10+，然后：

    cd \\wsl.localhost\Ubuntu\home\你的用户名\crossmux
    pip install freetype-py fonttools

### 生成全部 15 档（8–36pt）

**Linux / WSL**：

    cd ~/crossmux
    source .venv/bin/activate
    mkdir -p /tmp/misans
    python3 lib/EpdFont/scripts/fontconvert_sdcard.py \
      --name MiSans \
      --intervals cjk \
      --sizes 8,10,12,14,16,18,20,22,24,26,28,30,32,34,36 \
      --style regular \
      lib/EpdFont/builtinFonts/source/NotoSansSC/NotoSansSC-Regular.otf \
      --output-dir /tmp/misans/

**Windows PowerShell**（在 WSL 路径里跑）：

    cd \\wsl.localhost\Ubuntu\home\你的用户名\crossmux
    python lib\EpdFont\scripts\fontconvert_sdcard.py `
      --name MiSans `
      --intervals cjk `
      --sizes 8,10,12,14,16,18,20,22,24,26,28,30,32,34,36 `
      --style regular `
      lib\EpdFont\builtinFonts\source\NotoSansSC\NotoSansSC-Regular.otf `
      --output-dir $env:TEMP\misans

### 生成指定几档

只生成 18/20/22 三档：

    python3 lib/EpdFont/scripts/fontconvert_sdcard.py \
      --name MiSans \
      --intervals cjk \
      --sizes 18,20,22 \
      --style regular \
      lib/EpdFont/builtinFonts/source/NotoSansSC/NotoSansSC-Regular.otf \
      --output-dir /tmp/misans/

### 预估耗时

- 全部 15 档约 **2–3 分钟**
- 单档 10–30 秒（字号越大越慢）


## 五、拷贝到 SD 卡

生成完 `.cpfont` 文件后，需要复制到真机 SD 卡。

### 方式 1：WSL 挂载 F 盘（推荐）

    # 1. 挂载 F 盘到 /mnt/f（需要 sudo 密码）
    sudo mkdir -p /mnt/f
    sudo mount -t drvfs F: /mnt/f -o metadata,uid=1000,gid=1000,umask=022

    # 2. 创建 MiSans 目录
    mkdir -p /mnt/f/fonts/MiSans

    # 3. 拷贝 15 个 cpfont
    cp -v /tmp/misans/*.cpfont /mnt/f/fonts/MiSans/

    # 4. 同步并安全弹出
    sync
    sudo umount /mnt/f

    # 5. 拔出读卡器，插回真机

### 方式 2：Windows PowerShell

先把 cpfont 从 WSL 拷到 Windows 临时目录：

    # 在 WSL 里
    mkdir -p /mnt/c/Users/你的用户名/Desktop/misans_tmp
    cp /tmp/misans/*.cpfont /mnt/c/Users/你的用户名/Desktop/misans_tmp/

然后在 **Windows PowerShell** 里：

    # 假设 F: 是 SD 卡盘符
    $src = "$env:USERPROFILE\Desktop\misans_tmp"
    $dst = "F:\fonts\MiSans"
    New-Item -ItemType Directory -Force -Path $dst
    Copy-Item "$src\*.cpfont" $dst
    Write-Host "已复制 $((Get-ChildItem $dst).Count) 个文件"

### 方式 3：Windows 资源管理器

1. 打开 WSL 生成目录（在 Windows 资源管理器地址栏输入）：

       \\wsl.localhost\Ubuntu\tmp\misans

2. 全选 15 个 `.cpfont` 文件，Ctrl+C
3. 打开 SD 卡盘符（如 `F:\`），进入 `fonts\` 目录
4. 新建 `MiSans` 文件夹
5. 进入 `MiSans`，Ctrl+V 粘贴

### 方式 4：网页上传

1. 真机 → 文件传输 → 创建热点 或 加入网络
2. 浏览器打开设备显示网址
3. 进入 **Fonts** 标签页 → 上传 `.cpfont`


## 六、阅读器识别逻辑

### 启动时扫描

`SdCardFontRegistry::discover()` 在开机时扫描 SD 卡：

1. 扫 `/.fonts/` 和 `/fonts/` 两个根目录（隐藏 + 可见）
2. 每个子目录视为一个字体族，目录名 = family name（如 `MiSans`）
3. 解析 `MiSans_<size>.cpfont` 里的 `<size>`
4. 同 size 重复文件时后者覆盖前者
5. `/.fonts/` 里的族优先于 `/fonts/` 同名族

### 字体菜单

**设置 → 文字设置 → 阅读字体**：

- 列出所有已注册的字体族
- 选择 `MiSans` 后 → **阅读字号** → 列出 SD 卡上实际存在的字号
- 选中某档字号后立即生效

### 运行时加载

- **阅读正文**：走 SD 卡 cpfont，读取速度取决于 SD 卡（通常 <100ms）
- **preload 加速**：可选。Flash 缓存容量上限 **6.55 MB**，超过则跳过
  - 8–18pt 可 preload
  - 20pt 及以上超过 6.55 MB，无法 preload
  - 跳过后弹提示「字体超过 Flash 剩余空间，已加载 SD 卡字体文件」
- **UI 中文**：始终走内置 Flash 字体（8/10/12pt，GB2312），不依赖 SD 卡

### 缓存的优先级

| 层 | 来源 | 优先级 |
|---|---|---|
| 内置 UI 字体 | Flash | 最高（UI 专用） |
| 阅读字体 | SD 卡 cpfont | 阅读优先 |
| 阅读兜底 | Flash（内置 12pt） | SD 失败时启用 |


## 七、常见问题

### Q1：字体菜单里看不到 MiSans

检查：

1. SD 卡上目录名是否严格是 `MiSans`（大小写敏感）
2. 文件名是否严格是 `MiSans_<size>.cpfont`（如 `MiSans_18.cpfont`）
3. SD 卡是否在 `/.fonts/MiSans/` 或 `/fonts/MiSans/`
4. 重启真机，让 registry 重新扫描

### Q2：选了大字号后弹「字体超过 Flash 剩余空间」

**正常行为**，不是 bug。

- Flash 缓存区上限 6.55 MB
- 大字号 cpfont 超过上限（如 24pt = 10.7 MB）
- 阅读字体直接从 SD 卡读，不经过 Flash 缓存
- 弹一次提示后不再打扰

### Q3：中文字显示为方块 □

可能原因：

1. **SD 卡 cpfont 文件损坏**——重新生成并替换
2. **字体族名不匹配**——目录名必须是 `MiSans`
3. **文件名不符合规则**——必须 `<family>_<size>.cpfont`
4. **UI 中文字**——这个走内置 Flash 字体，不受 SD 卡影响

### Q4：拷贝字体后 SD 卡容量不够

15 档共约 **168 MB**。如果 SD 卡空间紧张，只留常用档：

    # 只留 18/20/22 三档（约 23 MB）
    rm /mnt/f/fonts/MiSans/MiSans_{8,10,12,14,16,24,26,28,30,32,34,36}.cpfont

### Q5：生成时提示「字体缺少某些字形」

MiSans Regular 覆盖完整 GB2312 全集（6763 字）。如果提示缺字，可能：

1. 用了其它字库（非 MiSans）
2. MiSans 版本过旧（建议用 4.009+）
3. 用了 CJK 之外的字形（如 Emoji、生僻汉字）

检查字体身份：

    python3 -c "
    from fontTools.ttLib import TTFont
    f = TTFont('lib/EpdFont/builtinFonts/source/NotoSansSC/NotoSansSC-Regular.otf')
    print(f['name'].getDebugName(1), f['name'].getDebugName(5))
    "

### Q6：cpfont 文件格式是 v4 吗

是。CrossMux 只接受 cpfont **v4** 格式。生成脚本 `fontconvert_sdcard.py` 输出的就是 v4。

旧的 v1/v2/v3 文件不会被识别。

## 八、相关文档

- [SD Card Fonts（上游英文版）](./sd-card-fonts.md)
- [项目 README](../README.md)
- [CLAUDE.md（AI 助手工作记忆）](../CLAUDE.md)
- [MiSans 官网](https://hyperos.mi.com/font/zh/)
- [MiSans 常见问题](https://hyperos.mi.com/font/faq)
- [MiSans 许可协议](https://hyperos.mi.com/font/zh/download/)
