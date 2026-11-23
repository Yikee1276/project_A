#ifndef PROJECT_A_MY_STRING_H
#define PROJECT_A_MY_STRING_H

#include <cstddef>

class MyString {
private:
    size_t size_;   // 字符数（不含结尾 '\0'）
    char* data_;

public:
    MyString();
    explicit MyString(const char* text);  // 带参构造（explicit 防隐式转换）
    ~MyString();

    MyString(const MyString& other);               // 拷贝构造（深拷贝）
    MyString& operator=(const MyString& other);    // 拷贝赋值

    MyString(MyString&& other) noexcept;           // 移动构造
    MyString& operator=(MyString&& other) noexcept;  // 移动赋值

    size_t size() const { return data_ ? size_ : 0; }
    const char* c_str() const { return data_; }    // 空串时为 nullptr
    bool empty() const { return size() == 0; }
};

#endif
