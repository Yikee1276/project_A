// tests/test_cpp.cpp —— valgrind 专用验收测试
//
// 编译方式：make valgrind  或  bash diagnose.sh
// 实际命令：g++ -std=c++17 -g -O0 -Wall -Wextra -Iinclude -o build/test_cpp \
//               tests/test_cpp.cpp src/my_string.cpp
// 然后由 valgrind --leak-check=full --show-leak-kinds=all 运行。
//
// 覆盖：
//   第 1 周 MyString：拷贝构造 / 拷贝赋值 / 自赋值 / 空串赋值 / 移动构造 / 移动赋值
//   第 2 周 Vector  ：push_back / 扩容 / 迭代器 / 1e6 压测 / 拷贝 / 移动 / 自赋值 / emplace_back
// 退出码：0 = 全部通过；1 = 有失败项

#include <cstdio>
#include <cstring>
#include <utility>

#include "my_string.h"
#include "my_vector_1.hpp"

static int g_fail = 0;

#define CHECK(cond)                                                        \
    do {                                                                   \
        if (!(cond)) {                                                     \
            std::printf("[FAIL] %s:%d  %s\n", __FILE__, __LINE__, #cond);  \
            ++g_fail;                                                      \
        }                                                                  \
    } while (0)

// ---------------------------------------------------------------------------
// 第 1 周：MyString
// ---------------------------------------------------------------------------
static void test_my_string() {
    std::printf("--- MyString ---\n");

    MyString s1("hello");
    CHECK(s1.size() == 5);
    CHECK(s1.c_str() != nullptr && std::strcmp(s1.c_str(), "hello") == 0);

    // 拷贝构造（深拷贝：内容相同、内存不同）
    MyString s2(s1);
    CHECK(s2.size() == 5 && std::strcmp(s2.c_str(), "hello") == 0);
    CHECK(s2.c_str() != s1.c_str());

    // 拷贝赋值 s1 = s2
    MyString s3("world");
    s1 = s3;
    CHECK(s1.size() == 5 && std::strcmp(s1.c_str(), "world") == 0);

    // 自赋值安全
    s1 = s1;
    CHECK(s1.size() == 5 && std::strcmp(s1.c_str(), "world") == 0);

    // 空串（默认构造）拷贝赋值：空指针安全
    MyString empty;
    s1 = empty;
    CHECK(s1.size() == 0 && s1.c_str() == nullptr);

    // 移动构造
    MyString s4("move-me");
    MyString s5(std::move(s4));
    CHECK(s5.size() == 7 && std::strcmp(s5.c_str(), "move-me") == 0);
    CHECK(s4.size() == 0 && s4.c_str() == nullptr);

    // 移动赋值
    MyString s6;
    s6 = std::move(s5);
    CHECK(s6.size() == 7 && std::strcmp(s6.c_str(), "move-me") == 0);
    CHECK(s5.size() == 0 && s5.c_str() == nullptr);
}

// ---------------------------------------------------------------------------
// 第 2 周：Vector
// ---------------------------------------------------------------------------
static void test_vector() {
    std::printf("--- Vector ---\n");

    Vector<int> v;
    CHECK(v.empty());
    CHECK(v.size() == 0 && v.capacity() == 0);

    v.push_back(1);
    v.push_back(2);
    CHECK(v.size() == 2 && v[0] == 1 && v[1] == 2);
    CHECK(v.capacity() >= 2);

    // 扩容：reserve 后容量增大，已有元素保留
    size_t cap0 = v.capacity();
    v.reserve(cap0 * 4);
    CHECK(v.capacity() >= cap0 * 4);
    CHECK(v.size() == 2 && v[0] == 1 && v[1] == 2);

    // 1e6 元素压测（push_back 触发多次 2 倍扩容）
    Vector<int> big;
    for (int i = 0; i < 1000000; ++i) {
        big.push_back(i);
    }
    CHECK(big.size() == 1000000);
    CHECK(big[0] == 0 && big[999999] == 999999);

    // 遍历求和
    long long total = 0;
    for (int x : big) {
        total += x;
    }
    CHECK(total == 499999500000LL);

    // 拷贝构造
    Vector<int> cp(big);
    CHECK(cp.size() == big.size() && cp[12345] == 12345);

    // 移动构造
    Vector<int> mv(std::move(cp));
    CHECK(mv.size() == 1000000 && cp.empty());

    // 拷贝赋值（Copy-and-Swap）+ 自赋值
    Vector<int> other;
    other.push_back(42);
    mv = other;
    CHECK(mv.size() == 1 && mv[0] == 42);
    mv = mv;  // 自赋值安全
    CHECK(mv.size() == 1 && mv[0] == 42);

    // emplace_back 原地构造
    Vector<int> e;
    e.emplace_back(7);
    e.emplace_back(8);
    CHECK(e.size() == 2 && e[0] == 7 && e[1] == 8);
}

int main() {
    test_my_string();
    test_vector();

    if (g_fail == 0) {
        std::printf("\n[OK] 全部验收检查通过\n");
        return 0;
    }
    std::printf("\n[FAIL] 共 %d 项失败\n", g_fail);
    return 1;
}
