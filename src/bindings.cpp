// ===========================================================================
//  bindings.cpp —— C++ 与 Python 之间的桥
//
//  本文件目前只是一副"空骨架"：
//    * 只注册了模块 fast_cpp 和一个用于冒烟测试的 add()；
//    * 具体业务类（MyString / MyVector ...）一律不在此处写死，
//      留给使用者在下方 TODO 区域按需暴露。
//
//  为什么保持最小？因为核心 C++ 类由项目作者手写，绑定层不应假设
//  任何成员函数名、构造签名或返回值类型。
// ===========================================================================

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>  // 便于以后暴露 std::vector 等 STL 容器（可选）

namespace py = pybind11;

// 冒烟测试函数：用于确认 C++ -> Python 调用链是否通畅
int add(int a, int b) {
    return a + b;
}

PYBIND11_MODULE(fast_cpp, m) {
    m.doc() = "fast_cpp —— C++/Python 混合开发模块（骨架，业务绑定待填充）";

    // ---- 链路冒烟检查 ---------------------------------------------------
    m.def("add", &add, py::arg("a"), py::arg("b"),
          "返回 a + b，仅用于验证模块可正常导入与调用");
    // --------------------------------------------------------------------

    // =====================================================================
    // TODO: 将你手写的 MyVector 和 MyString 在这里暴露给 Python
    //
    // 参考写法（写出你自己的类之后再取消注释并改成实际签名）：
    //
    //   #include "my_string.h"
    //   #include "my_vector.hpp"
    //
    //   py::class_<MyString>(m, "MyString")
    //       .def(py::init<const char *>(), py::arg("text"))
    //       .def("size", &MyString::size)
    //       .def("__len__", &MyString::size)
    //       .def("__repr__", [](const MyString &s) { /* 返回 std::string */ });
    //
    //   py::class_<MyVector<int>>(m, "IntVector")
    //       .def(py::init<>())
    //       .def("push_back", &MyVector<int>::push_back, py::arg("value"))
    //       .def("__len__", &MyVector<int>::size);
    //
    // 注意：模板类需要为每种具体类型分别做一次 class_ 绑定
    //       （如 MyVector<int> / MyVector<double>）。
    // =====================================================================
}
