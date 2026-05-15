# 项目名称

**一个帮助学习记忆的抽认卡应用**

> 项目说明待补充

```
FlashcardQt/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── AppEngine.h / .cpp
│   ├── models/
│   │   ├── TreeModel.h / .cpp
│   │   ├── StatsTreeModel.h / .cpp
│   │   ├── CardTableModel.h / .cpp
│   │   └── ReviewTableModel.h / .cpp
│   ├── widgets/
│   │   ├── MainWindow.h / .cpp
│   │   ├── LibraryWidget.h / .cpp
│   │   ├── QuizWidget.h / .cpp
│   │   ├── ReviewWidget.h / .cpp
│   │   ├── StatsManagerWidget.h / .cpp
│   │   └── dialogs/
│   │       ├── EditCardDialog.h / .cpp
│   │       ├── BatchImportDialog.h / .cpp
│   │       └── FilterThresholdDialog.h / .cpp
│   ├── core/
│   │   ├── CardManager.h / .cpp
│   │   ├── StatisticsManager.h / .cpp
│   │   └── QuizController.h / .cpp
│   └── util/
│       └── FileUtils.h / .cpp
├── third_party/
│   └── nlohmann/
│       └── json.hpp  (自动下载)
└── data/
    ├── cards/
    └── stats/
```
