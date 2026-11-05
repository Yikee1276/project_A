#include "my_string.h"
#include <cstring>
#include <cstdio>

MyString::MyString() {
    data_ = nullptr;
    size_ = 0;
}

MyString::MyString(const char* text) {
    if (text != nullptr) {
        size_ = std::strlen(text);
        data_ = new char[size_ + 1];
        std::strcpy(data_, text);
    } else {
        data_ = nullptr;
        size_ = 0;
    }
}   

int MyString::Getsize() {
    if (data_ != nullptr) {
        return size_;
    } else {
        return 0;
    }
}

void MyString::Getc_str() {
    if (data_ != nullptr) {
        std::printf("%s", data_);
    } else {
        std::printf("null");
    }
}

MyString& MyString::operator=(const MyString& other) {
    if (this == &other) {
        return *this;
    }
    // 这就是你亲手写的 help 暂存法
    char* help = new char[other.size_ + 1];
    std::strcpy(help, other.data_);
    
    size_ = other.size_;
    delete[] data_; 
    data_ = help;
    return *this;
}

MyString::~MyString() {
    delete[] data_;
    data_ = nullptr;
}

MyString::MyString(MyString&& other) noexcept{
    this->data_=other.data_;
    this->size_=other.size_;
    other.data_=nullptr;
    other.size_=0;
}

MyString& MyString::operator=(MyString&& other){
    if(this!=&other){
        delete[] data_;
        data_=other.data_;
        size_=other.size_;
        other.size_=0;
        other.data_=nullptr;
    }
    return *this;
}