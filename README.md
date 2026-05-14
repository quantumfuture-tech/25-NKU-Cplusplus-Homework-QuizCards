
# 抽认卡学习应用V1.0 - 控制台原型

基于 C++17/20 的控制台抽认卡学习工具，支持题库树管理、灵活抽查模式、统计历史记录与聚合。项目采用 CMake 构建，使用 nlohmann/json 进行数据持久化。

## ✨ 功能特性

- **题库树管理**：文件夹/单元树形结构，支持激活/停用、增删改查卡片，卡片全局去重。
- **抽查模式**：
  - **随机分组模式**：支持按卡片数量、总轮数、百分比三种方式自动分组，每组随机打乱。
  - **达标递归模式**：设定连续正确次数，动态筛选未达标卡片，直至全部达标。
- **统计历史**：每次抽查自动生成 `.session.json` 文件，支持浏览详情与文件夹聚合统计。
- **控制台交互**：清晰菜单导航，进度显示，即时反馈。

## 🛠 技术栈

| 组件     | 技术                                            |
| -------- | ----------------------------------------------- |
| 语言标准 | C++20                                           |
| 构建系统 | CMake 3.16+                                     |
| 编译器   | GCC 10+ / Clang 10+ / MSVC 2019+（需支持C++20） |
| JSON 库  | nlohmann/json (v3.12.0)                         |
| 文件系统 | std::filesystem                                 |

## 📦 构建与运行

### 前置条件

- CMake 3.16 或更高版本
- C++20 编译器（如 g++ 10+、MSVC 2019+、Clang 10+）
- （可选）Git（用于自动下载 json.hpp）

### 获取代码

```bash
git clone <your-repo-url>
cd console-original
```

### 构建步骤

#### 通用方式（适用于 Visual Studio、Xcode、Unix Makefiles 等默认生成器）

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

#### Windows MinGW 用户（明确指定生成器和编译器）

```bash
mkdir build
cd build
cmake -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=g++ ..
cmake --build .
```

**说明**：如果您的 MinGW 编译器不在系统 PATH 中，请使用完整路径，例如：

```bash
cmake -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe ..
```

#### 构建产物位置

- Windows：`build/flashcard_console.exe` 或 `build/Release/flashcard_console.exe`
- Linux/macOS：`build/flashcard_console`

### 运行

> **⚠️ 重要**：程序使用相对路径 `data/cards` 和 `data/stats`，因此**必须保证运行时的当前工作目录为项目根目录**（即包含 `data` 文件夹的目录）。

从项目根目录执行：

```bash
# Windows（根据实际生成路径调整）
build\flashcard_console.exe
# 或
build\Release\flashcard_console.exe

# Linux/macOS
./build/flashcard_console
```

首次运行会自动创建 `data/cards` 和 `data/stats` 目录。

## 📖 使用说明

### 主菜单

```
===== 主菜单 =====
1. 题库管理
2. 开始抽查
3. 浏览统计历史
4. 退出
```

### 题库管理

- **1 显示题库树**：查看文件夹/单元及卡片树形结构。
- **2 添加新单元**：在根目录创建单元（`.unit.json` 文件）。
- **3 为单元添加卡片**：输入问题和答案，自动去重。
- **4 从单元删除卡片**：移除卡片引用（卡片本身仍保留于全局）。

### 开始抽查

1. 选择抽查模式：
   - `1` 随机分组模式 → 选择分组方式（数量/轮数/百分比）
   - `2` 达标递归模式 → 设定连续正确次数
2. 逐张显示问题，按任意键显示答案，输入 `y`（正确）或 `n`（错误）。
3. 进度显示：轮次/总轮次 或 本轮剩余卡片。
4. 抽查结束后自动生成统计文件并显示简要结果。

### 浏览统计历史

- 列出所有 `.session.json` 文件。
- 输入文件名可查看每张卡片的尝试次数和正确次数。

## 📁 文件结构

```
FlashcardConsole/
├── CMakeLists.txt          # 构建配置
├── .gitignore
├── README.md
├── src/
│   ├── CardManager.h/cpp   # 卡片管理、题库树、JSON 序列化
│   ├── StatisticsManager.h/cpp # 统计数据集、会话保存/加载
│   ├── QuizController.h/cpp    # 抽查会话控制、分组逻辑
│   └── main.cpp            # 控制台菜单与主流程
├── third_party/            # 自动下载的 nlohmann/json.hpp
│   └── nlohmann/
│       └── json.hpp
└── build/                  # 构建目录
    └── build/.../flashcard_console[.exe]  #可执行文件
```

`...` 表示可能存在的 `Release/` 或 `Debug/` 子目录，`[.exe]` 表示 Windows 下的扩展名

运行时生成的 `data/` 目录**不包含在源代码树中**，其位置取决于执行程序时的当前工作目录。推荐工作目录 = 项目根目录，此时目录结构为：

```
FlashcardConsole/
├── data/
│   ├── cards/              # 题库存储（.unit.json 文件）
│   │   └── *.unit.json
│   └── stats/              # 统计历史（.session.json 文件）
│       └── *.session.json
└── ...
```

## 🧪 示例数据

`data/cards/示例.unit.json`

```json
{
    "name": "数学基础",
    "active": true,
    "cards": [
        {
            "question": "1+1 = ?",
            "answer": "2",
            "active": true
        }
    ]
}
```

## 🤝 贡献

老师布置的作业，~~欢迎提交 Issue 或 Pull Request。请确保代码符合 C++17/20 规范，并遵循 Conventional Commits 格式。~~

## 📄 许可证

~~老师布置的作业牌许可证~~

## 📌 已知问题与改进方向

- 目前仅支持控制台交互，后续可扩展 Qt/QML 图形界面。
- 单元删除后文件夹路径未自动清理，需手动删除空目录。
- 达标递归模式进度显示可进一步细化（如显示已达标卡片数）。
