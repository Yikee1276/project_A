// tests/vector_demo.cpp —— Vector<T> 功能演示 / 自测程序（独立于验收测试，可单独编译运行）
//
// 编译运行：
//   g++ -std=c++17 -Iinclude tests/vector_demo.cpp -o build/vector_demo && ./build/vector_demo

#include <iostream>
#include <string>

#include "my_vector_1.hpp"

// 1. 定义一个带计数器的测试类
struct Foo {
    int id;
    std::string name;
    static int copy_count;
    static int move_count;

    Foo(int i, std::string n) : id(i), name(std::move(n)) {}

    // 拷贝构造（记录次数）
    Foo(const Foo& other) : id(other.id), name(other.name) {
        copy_count++;
    }

    // 移动构造（记录次数，必须是 noexcept 才能被 move_if_noexcept 优先选中）
    Foo(Foo&& other) noexcept : id(other.id), name(std::move(other.name)) {
        move_count++;
    }
};

// 初始化静态成员
int Foo::copy_count = 0;
int Foo::move_count = 0;

int main() {
    std::cout << "=== 测试 1：emplace_back 原地构造与扩容 ===\n";
    {
        Vector<Foo> vec;

        // 插入第 1 个元素：不触发扩容，原地构造
        vec.emplace_back(1, "one");

        // 插入第 2 个元素：触发扩容！
        // 此时旧元素 "one" 会被 move_if_noexcept 移动到新内存（+1 Move）
        // 新元素 "two" 是原地构造（+0 Move, +0 Copy）
        vec.emplace_back(2, "two");

        std::cout << "Copies: " << Foo::copy_count << " (预期: 0)\n";
        std::cout << "Moves: " << Foo::move_count << " (预期: 1)\n";
    }

    // 重置计数器，准备下一轮测试
    Foo::copy_count = 0;
    Foo::move_count = 0;

    std::cout << "\n=== 测试 2：const 对象访问验证 ===\n";
    {
        Vector<Foo> vec;
        vec.emplace_back(10, "const_test");

        // 创建一个 const 的 Vector 引用
        const Vector<Foo>& const_vec = vec;

        // 验证 const 版本的 operator[] 和 begin()
        std::cout << "Const access: " << const_vec[0].id << ", " << const_vec[0].name << "\n";

        for (const auto& item : const_vec) {
            std::cout << "Range for: " << item.id << ", " << item.name << "\n";
        }
    }

    std::cout << "\n=== 测试 3：1e6 int 压测 ===\n";
    {
        Vector<int> intVec;
        for (int i = 0; i < 1000000; ++i) {
            intVec.emplace_back(i);
        }
        std::cout << "1e6 test passed! Size: " << intVec.size() << "\n";
    }

    std::cout << "\n=== 测试 4：Copy-and-Swap 自赋值验证 ===\n";
    {
        Vector<Foo> vec;
        vec.emplace_back(100, "self_assign");

        // 极其危险的自赋值操作！旧代码必崩，新代码稳如泰山
        vec = vec;

        std::cout << "Self-assign success! Data: " << vec[0].id << ", " << vec[0].name << "\n";
    }

    return 0;
}
