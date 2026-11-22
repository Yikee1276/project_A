cmake_minimum_required(VERSION 3.15)
project(hello_example)

# 寻找 Python3 组件（必须包含 Development 和 Interpreter）
find_package(Python3 REQUIRED COMPONENTS Development Interpreter)

# 添加 pybind11 支持
find_package(pybind11 REQUIRED)

# 定义一个名为 my_cpp_lib 的模块
# 这里的名字必须和 PYBIND11_MODULE 里的第一个参数一致！
pybind11_add_module(my_cpp_lib hello.cpp)