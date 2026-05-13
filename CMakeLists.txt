cmake_minimum_required(VERSION 3.16)
project(FlashcardConsole VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 第三方库：自动下载 json.hpp
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)
file(MAKE_DIRECTORY ${THIRD_PARTY_DIR})
set(JSON_HEADER ${THIRD_PARTY_DIR}/json.hpp)
if(NOT EXISTS ${JSON_HEADER})
    message(STATUS "正在下载 json.hpp ...")
    file(DOWNLOAD 
        https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp
        ${JSON_HEADER}
        SHOW_PROGRESS
        TIMEOUT 60
    )
    message(STATUS "json.hpp 下载完成")
endif()

# ---------- 调试：打印源文件路径 ----------
set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
message(STATUS "源代码目录: ${SOURCE_DIR}")
file(GLOB SOURCE_FILES "${SOURCE_DIR}/*.cpp")
message(STATUS "找到的 .cpp 文件: ${SOURCE_FILES}")

if(NOT SOURCE_FILES)
    message(FATAL_ERROR "在 ${SOURCE_DIR} 中没有找到任何 .cpp 文件！请检查目录是否存在且包含源文件。")
endif()

# 添加可执行文件，使用 GLOB 收集到的源文件
add_executable(flashcard_console ${SOURCE_FILES})

target_include_directories(flashcard_console PRIVATE ${THIRD_PARTY_DIR})
target_compile_features(flashcard_console PRIVATE cxx_std_17)