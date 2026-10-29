#ifndef PROJECT_A_MY_STRING_H
#define PROJECT_A_MY_STRING_H

#include <cstddef>

// 一个极其简单的字符串类，仅用于演示 C++ <-> Python 混合开发链路。
// 内部仅持有一个 const char*，字符串内存由本对象负责分配与释放。
class MyString {
public:
    // 默认构造：空串
    MyString();

    // 由 C 风格字符串构造（深拷贝）
    explicit MyString(const char *text);

    // 析构函数：释放内部缓冲区
    ~MyString();

    // 拷贝构造（深拷贝）
    MyString(const MyString &other);

    // 移动构造（接管 other 的缓冲区，不分配内存）
    MyString(MyString &&other) noexcept;

    // 拷贝赋值
    MyString &operator=(const MyString &other);

    // 移动赋值
    MyString &operator=(MyString &&other) noexcept;

    // 只读访问内部数据（永不为 nullptr）
    const char *c_str() const;

    // 字符串长度（不含结尾 '\0'）
    std::size_t size() const;

    bool empty() const { return size() == 0; }

private:
    // 深拷贝一段文本到新分配的缓冲区
    void copy_from(const char *text);

    const char *data_;
};

#endif // PROJECT_A_MY_STRING_H
