// ===========================================================================
//  test_cpp.cpp —— valgrind 专用 C++ 测试入口
//
//  编译方式：make valgrind  或  bash diagnose.sh
//  实际命令：g++ -std=c++17 -g -O0 -Wall -Wextra -Iinclude -o build/test_cpp tests/test_cpp.cpp
//  然后由 valgrind --leak-check=full --show-leak-kinds=all 运行。
//
//  你只需要在 main() 里写测试代码（构造、扩容、拷贝、移动……），
//  valgrind 会报告任何未释放的内存与非法读写。
// ===========================================================================

#include "my_string.h"
#include "my_vector.hpp"

int main() {
    // ---------------------------------------------------------------------
    // TODO: 在这里填写你的 C++ 测试代码，例如：
    //
    //   MyString s1("hello");
    //   MyString s2 = s1;          // 拷贝构造
    //   MyString s3 = std::move(s2);  // 移动构造
    //
    //   MyVector<int> v;
    //   for (int i = 0; i < 1000; ++i) v.push_back(i);  // 触发多次扩容
    // ---------------------------------------------------------------------

    return 0;  // 返回 0 表示测试程序正常结束
}
