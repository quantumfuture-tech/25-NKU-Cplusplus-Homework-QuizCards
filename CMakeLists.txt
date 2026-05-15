cmake_minimum_required(VERSION 3.16)
project(FlashcardQt VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Concurrent)

# 第三方库 nlohmann/json
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)
file(MAKE_DIRECTORY ${THIRD_PARTY_DIR}/nlohmann)
set(JSON_HEADER ${THIRD_PARTY_DIR}/nlohmann/json.hpp)
if(NOT EXISTS ${JSON_HEADER})
    message(STATUS "Downloading json.hpp ...")
    file(DOWNLOAD
        https://github.com/nlohmann/json/releases/download/v3.12.0/json.hpp
        ${JSON_HEADER}
        SHOW_PROGRESS
        TIMEOUT 60)
    message(STATUS "json.hpp downloaded")
endif()

# 源文件
set(SOURCES
    src/main.cpp
    src/AppEngine.cpp
    src/models/TreeModel.cpp
    src/models/StatsTreeModel.cpp
    src/models/CardTableModel.cpp
    src/models/ReviewTableModel.cpp
    src/widgets/MainWindow.cpp
    src/widgets/LibraryWidget.cpp
    src/widgets/QuizWidget.cpp
    src/widgets/ReviewWidget.cpp
    src/widgets/StatsManagerWidget.cpp
    src/widgets/dialogs/EditCardDialog.cpp
    src/widgets/dialogs/BatchImportDialog.cpp
    src/widgets/dialogs/FilterThresholdDialog.cpp
    src/core/CardManager.cpp
    src/core/StatisticsManager.cpp
    src/core/QuizController.cpp
    src/core/Debug.cpp
    src/util/FileUtils.cpp
)

add_executable(flashcard_qt ${SOURCES}
    src/core/Debug.h)

target_include_directories(flashcard_qt PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${THIRD_PARTY_DIR}
)

target_link_libraries(flashcard_qt PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Concurrent
)