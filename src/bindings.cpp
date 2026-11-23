// ===========================================================================
//  bindings.cpp —— C++ 与 Python 之间的桥
//
//  本文件把项目里手写的 C++ 类暴露给 Python：
//    * fast_cpp.add()        —— 冒烟测试函数；
//    * fast_cpp.IntVector    —— 手写模板类 Vector<int> 的绑定；
//    * fast_cpp.DoubleVector —— 手写模板类 Vector<double> 的绑定。
//
//  模板类需要为每种具体类型分别做一次 class_ 绑定（见 bind_vector 的实现）。
//  Vector<T> 的实现位于 include/my_vector_1.hpp（模板实现必须写在头文件里）。
// ===========================================================================

#include <type_traits>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>  // 便于以后暴露 std::vector 等 STL 容器（可选）

#include "my_vector_1.hpp"  // 手写容器 Vector<T>

namespace py = pybind11;

// 冒烟测试函数：用于确认 C++ -> Python 调用链是否通畅
int add(int a, int b) {
    return a + b;
}

// ---------------------------------------------------------------------------
// 绑定辅助：为手写模板类 Vector<T> 生成通用的 Python 类声明
//
// 注意：模板类不能直接 class_<Vector<T>> 泛化注册，
//       需要在模块初始化时针对具体类型（如 int / double）分别实例化。
// ---------------------------------------------------------------------------
template <typename T>
void bind_vector(py::module_ &m, const char *name) {
    py::class_<Vector<T>>(m, name)
        .def(py::init<>())
        .def("__len__", &Vector<T>::size)
        .def("size", &Vector<T>::size)
        .def("capacity", &Vector<T>::capacity)
        .def("empty", &Vector<T>::empty)
        .def("reserve",
             [](Vector<T> &v, size_t new_cap) { v.reserve(new_cap); },
             py::arg("new_cap"))
        .def("push_back",
             [](Vector<T> &v, T value) { v.push_back(value); },
             py::arg("value"))
        .def("__getitem__",
             [](const Vector<T> &v, size_t i) {
                 if (i >= v.size()) throw py::index_error();
                 return v[i];
             },
             py::arg("i"))
        .def("__iter__",
             [](const Vector<T> &v) {
                 return py::make_iterator(v.begin(), v.end());
             },
             py::keep_alive<0, 1>())  // 迭代期间保持 Vector 存活
        .def_static("from_list",
                    [](py::iterable seq) {
                        Vector<T> v;
                        for (py::handle h : seq) v.push_back(h.cast<T>());
                        return v;
                    },
                    py::arg("values"),
                    "用 Python 可迭代对象一次性填充一个全新 Vector")
        .def("to_list",
             [](const Vector<T> &v) {
                 py::list out;
                 for (const auto &x : v) out.append(x);
                 return out;
             },
             "复制为 Python list")
        .def("sum",
             [](const Vector<T> &v) {
                 // 整型累加必须用 64 位，避免 1e5 级数据求和溢出 int32
                 using SumT = std::conditional_t<std::is_integral_v<T>, long long, T>;
                 SumT s = 0;
                 for (const auto &x : v) s += x;
                 return s;
             },
             "在 C++ 侧原生循环求和（绕开 Python 逐元素调用开销，用于性能对比）");
}

PYBIND11_MODULE(fast_cpp, m) {
    m.doc() = "fast_cpp —— C++/Python 混合开发模块（手写 Vector<T> 已暴露）";

    // ---- 链路冒烟检查 ---------------------------------------------------
    m.def("add", &add, py::arg("a"), py::arg("b"),
          "返回 a + b，仅用于验证模块可正常导入与调用");
    // --------------------------------------------------------------------

    // ---- 手写容器 Vector<T> --------------------------------------------
    bind_vector<int>(m, "IntVector");
    bind_vector<double>(m, "DoubleVector");
}
