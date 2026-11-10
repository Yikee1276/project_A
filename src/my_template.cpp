#include <iostream>
#include <iterator>       // std::distance 需要
#include "../include/my_template.h"  // 包含头文件

int main() {
    std::cout << "--- 测试 Triple ---\n";
    // 实例化 Triple<int> 并测试 GetMax
    Triple<int> t1(3, 9, 5);
    std::cout << "Triple (3, 9, 5) Max: " << t1.GetMax() << "\n"; 
    
    // 实例化 Triple<double> 测试
    Triple<double> t2(1.5, 0.2, 8.8);
    std::cout << "Triple (1.5, 0.2, 8.8) Max: " << t2.GetMax() << "\n";

    std::cout << "\n--- 测试 StaticArray ---\n";
    // 实例化大小为 5 的 int 数组
    StaticArray<int, 5> arr;
    
    // 填充 42
    arr.fill(42);
    
    // 用范围 for 遍历打印
    std::cout << "Array elements: ";
    for (const auto& x : arr) {
        std::cout << x << " ";
    }
    std::cout << "\n";

    // 用 std::distance 计算元素个数
    std::cout << "Size via distance: " << std::distance(arr.begin(), arr.end()) << "\n";

    return 0;
}