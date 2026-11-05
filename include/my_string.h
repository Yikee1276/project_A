#ifndef PROJECT_A_MY_STRING_H
#define PROJECT_A_MY_STRING_H

#include <cstddef>

class MyString {
private:
    size_t size_;
    char* data_; 

public:
    MyString();
    MyString(const char* text); // 带参构造
    ~MyString();
    
    MyString(const MyString& other);//拷贝构造
    MyString& operator=(const MyString& other);//拷贝赋值
    
    MyString(MyString&& other) noexcept;//移动构造
    MyString& operator=(MyString&& other);//移动赋值
    int Getsize();
    void Getc_str();
};

#endif