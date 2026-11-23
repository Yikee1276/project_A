#include "my_string.h"

#include <cstring>

MyString::MyString() : size_(0), data_(nullptr) {}

MyString::MyString(const char* text) : size_(0), data_(nullptr) {
    if (text != nullptr) {
        size_ = std::strlen(text);
        data_ = new char[size_ + 1];
        std::memcpy(data_, text, size_ + 1);  // 连同结尾 '\0' 一起复制
    }
}

MyString::~MyString() {
    delete[] data_;
    data_ = nullptr;
}

// 拷贝构造：深拷贝（新分配一块内存并复制内容）
MyString::MyString(const MyString& other) : size_(0), data_(nullptr) {
    if (other.data_ != nullptr) {
        size_ = other.size_;
        data_ = new char[size_ + 1];
        std::memcpy(data_, other.data_, size_ + 1);
    }
}

// 拷贝赋值：空指针安全（other 为空串时也能正确赋值）
MyString& MyString::operator=(const MyString& other) {
    if (this == &other) {
        return *this;  // 自赋值保护
    }
    if (other.data_ == nullptr) {  // 源为空串
        delete[] data_;
        data_ = nullptr;
        size_ = 0;
        return *this;
    }
    char* help = new char[other.size_ + 1];  // 先申请新内存（异常安全）
    std::memcpy(help, other.data_, other.size_ + 1);
    delete[] data_;  // 再释放旧内存
    data_ = help;
    size_ = other.size_;
    return *this;
}

// 移动构造：直接转移指针
MyString::MyString(MyString&& other) noexcept
    : size_(other.size_), data_(other.data_) {
    other.size_ = 0;
    other.data_ = nullptr;
}

// 移动赋值
MyString& MyString::operator=(MyString&& other) noexcept {
    if (this != &other) {
        delete[] data_;
        data_ = other.data_;
        size_ = other.size_;
        other.data_ = nullptr;
        other.size_ = 0;
    }
    return *this;
}
