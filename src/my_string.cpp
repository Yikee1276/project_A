#include "my_string.h"

#include <cstring>
#include <utility>

namespace {

// 内部缓冲区一律用 new char[] 分配，因此释放时使用 delete[]
char *dup_text(const char *text) {
    const std::size_t len = std::strlen(text) + 1; // 包含结尾 '\0'
    char *buf = new char[len];
    std::memcpy(buf, text, len);
    return buf;
}

} // namespace

MyString::MyString() : data_(nullptr) {}

MyString::MyString(const char *text) : data_(nullptr) {
    copy_from(text);
}

MyString::~MyString() {
    delete[] data_;
}

MyString::MyString(const MyString &other) : data_(nullptr) {
    copy_from(other.data_);
}

MyString::MyString(MyString &&other) noexcept : data_(other.data_) {
    other.data_ = nullptr;
}

MyString &MyString::operator=(const MyString &other) {
    if (this != &other) {
        MyString tmp(other);          // 先完整拷贝，异常安全
        std::swap(data_, tmp.data_);  // tmp 析构时释放旧缓冲区
    }
    return *this;
}

MyString &MyString::operator=(MyString &&other) noexcept {
    if (this != &other) {
        delete[] data_;
        data_ = other.data_;
        other.data_ = nullptr;
    }
    return *this;
}

const char *MyString::c_str() const {
    return data_ != nullptr ? data_ : "";
}

std::size_t MyString::size() const {
    return data_ != nullptr ? std::strlen(data_) : 0;
}

void MyString::copy_from(const char *text) {
    data_ = nullptr;
    if (text != nullptr) {
        data_ = dup_text(text);
    }
}
