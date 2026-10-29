cmake_minimum_required(VERSION 3.15)
project(fast_cpp LANGUAGES CXX)

# ---------------------------------------------------------------------------
# C++ 标准
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# 解释器选择：优先项目自带 venv
#
# pybind11 装在 .venv 里。如果直接运行 cmake 而没有激活 venv，
# find_package(Python3) 会命中系统的 /usr/bin/python3，那里没有 pybind11，
# 于是 `python -m pybind11 --cmakedir` 失败。为避免这种"忘记 source"的坑，
# 这里显式把 venv 解释器作为提示传给 CMake（仍可用 -DPython3_EXECUTABLE=... 覆盖）。
# ---------------------------------------------------------------------------
set(PROJECT_VENV_PYTHON ${CMAKE_CURRENT_SOURCE_DIR}/../.venv/bin/python)

if(EXISTS ${PROJECT_VENV_PYTHON})
    if(NOT Python3_EXECUTABLE)
        set(Python3_EXECUTABLE ${PROJECT_VENV_PYTHON})
    endif()
    if(NOT Python_EXECUTABLE)
        set(Python_EXECUTABLE ${PROJECT_VENV_PYTHON})
    endif()
endif()

if(NOT Python3_EXECUTABLE)
    find_package(Python3 REQUIRED COMPONENTS Interpreter Development.Module)
endif()
message(STATUS "Python 解释器：${Python3_EXECUTABLE}")

# ---------------------------------------------------------------------------
# 定位 venv 中 pybind11 的 cmakedir（不在默认搜索路径里，必须显式加）
# ---------------------------------------------------------------------------
execute_process(
    COMMAND ${Python3_EXECUTABLE} -m pybind11 --cmakedir
    OUTPUT_VARIABLE PYBIND11_CMAKE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE PYBIND11_CMAKE_DIR_RESULT
)

if(NOT PYBIND11_CMAKE_DIR_RESULT EQUAL 0 OR NOT PYBIND11_CMAKE_DIR)
    message(FATAL_ERROR
        "无法从 ${Python3_EXECUTABLE} 获取 pybind11 配置目录。\n"
        "请先激活虚拟环境再配置：\n"
        "  source .venv/bin/activate && cd build && cmake ../scripts\n"
        "或在 venv 中补装 pybind11：\n"
        "  pip install pybind11 -i https://pypi.tuna.tsinghua.edu.cn/simple")
endif()

list(APPEND CMAKE_PREFIX_PATH "${PYBIND11_CMAKE_DIR}")
message(STATUS "pybind11 cmakedir: ${PYBIND11_CMAKE_DIR}")

# 使用 pybind11 的 "新 FindPython" 模式（配合上面的 Python_EXECUTABLE 提示，
# 避免 pybind11 回退到已弃用的 FindPythonInterp 而产生开发者警告）
set(PYBIND11_FINDPYTHON ON)

find_package(pybind11 REQUIRED)
message(STATUS "pybind11 version: ${pybind11_VERSION}")

# ---------------------------------------------------------------------------
# 自动收集 src/ 下的实现文件
#
# 约定：
#   * src/bindings.cpp  —— 绑定层，单独显式加入；
#   * src/ 下其它 *.cpp —— 全部自动纳入编译（新增 .cpp 后无需改本文件）；
#   * CONFIGURE_DEPENDS 让 make 检测到新增/删除源文件时自动重新执行 cmake。
# ---------------------------------------------------------------------------
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include)
set(BINDINGS_SOURCE ${SRC_DIR}/bindings.cpp)

file(GLOB PROJECT_SOURCES CONFIGURE_DEPENDS "${SRC_DIR}/*.cpp")
list(REMOVE_ITEM PROJECT_SOURCES "${BINDINGS_SOURCE}")
list(SORT PROJECT_SOURCES)

message(STATUS "自动纳入编译的业务源文件：")
foreach(src ${PROJECT_SOURCES})
    message(STATUS "  - ${src}")
endforeach()
message(STATUS "  绑定层：${BINDINGS_SOURCE}")

# ---------------------------------------------------------------------------
# 生成 Python 扩展模块 fast_cpp
# ---------------------------------------------------------------------------
pybind11_add_module(fast_cpp
    ${BINDINGS_SOURCE}
    ${PROJECT_SOURCES}
)

target_include_directories(fast_cpp PRIVATE ${INCLUDE_DIR})

# 所有构建产物统一放到项目根目录的 build/
set_target_properties(fast_cpp PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY         /home/CH/workspace/project_A/build
    LIBRARY_OUTPUT_DIRECTORY_DEBUG   /home/CH/workspace/project_A/build
    LIBRARY_OUTPUT_DIRECTORY_RELEASE /home/CH/workspace/project_A/build
    RUNTIME_OUTPUT_DIRECTORY         /home/CH/workspace/project_A/build
    ARCHIVE_OUTPUT_DIRECTORY         /home/CH/workspace/project_A/build
)
