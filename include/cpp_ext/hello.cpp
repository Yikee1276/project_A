#include <pybind11/pybind11.h>
#include <string>

namespace py = pybind11;

// 这是一个普通的 C++ 函数
std::string hello(const std::string &name) {
    return "Hello, " + name + "! This is a speaking.";
}

// 这是 Python 调用的入口
// 第一个参数 "my_cpp_lib" 是模块名，Python import 时要用这个名字
PYBIND11_MODULE(my_cpp_lib, m) {
    m.doc() = "My first pybind11 extension"; // 模块文档

    // 将 C++ 函数 hello 绑定到 Python 模块中，Python 里也叫 hello
    m.def("hello", &hello, "A function that adds 'Hello' to a name");
}