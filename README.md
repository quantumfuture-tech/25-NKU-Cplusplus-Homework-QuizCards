# 抽认卡学习应用V2.0 - QtWidgets

[![Qt](https://img.shields.io/badge/Qt-6.5+-green.svg)](https://www.qt.io/)
[![C++](https://img.shields.io/badge/C++-20-blue.svg)](https://isocpp.org/)

一个基于 **Qt6 + C++20** 的本地抽认卡学习工具，采用树形结构管理题库、支持多种抽查模式、实时统计与历史查阅，所有数据存储在本地 JSON 文件中，无需数据库。

## ✨ 功能特性

### 📚 题库管理

- 树形结构（文件夹 / 单元），支持无限层级嵌套
- 卡片增删改查，自动去重（相同问题+答案视为同一卡片）
- 批量导入卡片（`问题|答案` 格式，每行一条）
- 递归激活/停用：勾选文件夹将同时影响其所有子单元及内部卡片

### 🎲 抽查模式（未完整实现）

- **随机分组模式**：支持按每组卡片数、总轮数或百分比动态分组
- **达标递归模式**：反复抽查未达标卡片，达标条件为连续正确次数（可配置）
- 交互流程：显示问题 → ~~点击/按键显示答案~~ → 按钮标记错误，按钮标记正确
- 实时进度显示（当前轮次 / 总轮次，本轮已答 / 本轮总数）
- 提前结束并自动保存统计结果

### 📊 统计与查阅（未完整实现）

- 每次抽查生成 `.session.json` 文件，保存在 `data/stats/`
- 历史统计以**目录树**形式展示，支持：
  - 文件夹聚合：合并文件夹下所有会话的统计数据
  - 移动、重命名、删除会话或文件夹
- 查阅视图支持**快捷筛选**：
  - 全部、未学习、学习次数＜3
  - 正确率＜50%、50%~80%、＞80%
  - 上次错误
  - 自定义阈值（正确率范围 + 最少学习次数）

### 💾 数据持久化

- 题库：`data/cards/` 目录，每个单元对应一个 `.unit.json` 文件
- 统计：`data/stats/` 目录，每次会话生成 `YYYY-MM-DD_HH-MM-SS.session.json`
- 文件 I/O 采用异步写入（QtConcurrent），保证界面流畅

## 🛠️ 技术栈

| 组件      | 技术                                                   |
| --------- | ------------------------------------------------------ |
| GUI 框架  | Qt 6.5+ (Widgets)                                      |
| 构建系统  | CMake 3.16+                                            |
| C++ 标准  | C++20                                                  |
| JSON 处理 | [nlohmann/json](https://github.com/nlohmann/json) v3.12.0 |
| 并发      | QtConcurrent                                           |
| 文件系统  | `std::filesystem`                                    |

## 📦 依赖项

- **Qt 6.5+**（必须包含 `Core`、`Widgets`、`Concurrent` 模块）
- **CMake 3.16+**
- **C++20 编译器**（MSVC 2022 / GCC 11+ / Clang 14+）

> 项目会在 CMake 配置阶段自动下载 `nlohmann/json.hpp` 头文件，无需手动安装。

## 🚀 构建与运行

### 1. 配置并构建

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

> **Windows 用户**：可使用 Visual Studio 开发者命令提示符，或使用 Qt Creator 直接打开 `CMakeLists.txt`。

### 2. 运行

```bash
# 从项目根目录运行（保证 data/ 目录能被找到）
cd ..
./build/FlashcardQt
```

或直接进入 `build/Release/` 双击 `FlashcardQt.exe`（Windows）。

## 📁 文件结构

```
FlashcardQt/
├── CMakeLists.txt
├── README.md
├── data/
│   ├── cards/               # 题库文件 (.unit.json)
│   └── stats/               # 统计会话 (.session.json)
├── src/
│   ├── main.cpp
│   ├── AppEngine.h/.cpp
│   ├── core/                # 核心业务逻辑
│   │   ├── CardManager.h/.cpp
│   │   ├── StatisticsManager.h/.cpp
│   │   ├── QuizController.h/.cpp
│   │   └── Debug.h/.cpp
│   ├── models/              # Qt 数据模型
│   │   ├── TreeModel.h/.cpp
│   │   ├── CardTableModel.h/.cpp
│   │   ├── ReviewTableModel.h/.cpp
│   │   └── StatsTreeModel.h/.cpp
│   ├── widgets/             # UI 组件
│   │   ├── MainWindow.h/.cpp
│   │   ├── LibraryWidget.h/.cpp
│   │   ├── QuizWidget.h/.cpp
│   │   ├── ReviewWidget.h/.cpp
│   │   ├── StatsManagerWidget.h/.cpp
│   │   └── dialogs/
│   │       ├── EditCardDialog.h/.cpp
│   │       ├── BatchImportDialog.h/.cpp
│   │       └── FilterThresholdDialog.h/.cpp
│   └── util/
│       └── FileUtils.h/.cpp
└── third_party/             # 自动下载的 json.hpp
    └── nlohmann/
        └── json.hpp
```

## 📖 使用指南

### 首次使用

1. 运行程序后，进入 **题库** 标签页。
2. 右键根目录或使用上方按钮，创建**单元**（`📁 新建单元`）。
3. 选中单元，点击 `+ 添加卡片`，输入问题和答案。
4. 也可使用 `📋 批量导入` 一次性添加多张卡片（每行格式 `问题|答案`）。

### 开始抽查

1. 切换到 **抽查** 标签页。
2. 选择**题目来源**（默认为所有激活卡片）。
3. 选择抽查模式并设置参数：
   - **随机分组**：设置分组方式（卡片数/轮数/百分比）和对应数值。
   - **达标递归**：设置连续正确次数阈值。
4. 点击 `开始抽查`。
5. 显示问题时，点击卡片~~显示答案~~（未实现），然后选择 `正确` 或 `错误`。
6. 可提前结束，系统自动保存本次会话统计。

### 查阅统计（未完整实现）

- **查阅** 标签页默认显示所有历史会话的聚合统计。
- 点击上方的筛选按钮（如 `正确率＜50%`）可快速过滤。
- 双击列表中的卡片可查看完整答案。

### 管理统计记录（未完整实现）

- ~~切换到 **统计管理** 标签页。~~
- ~~树形视图展示 `data/stats/` 下所有文件与文件夹。~~
- ~~双击会话可查看详细统计数据；右键支持新建文件夹、重命名、移动、删除。~~

## 🤝 贡献

老师布置的作业，~~欢迎提交 Issue 或 Pull Request。请确保代码符合项目风格（使用 `.clang-format` 配置文件）。~~

## ~~📄 许可证~~

~~本项目采用 **老师布置的作业** 许可证~~
