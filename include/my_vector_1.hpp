#ifndef PROJECT_A_MY_VECTOR_1_HPP
#define PROJECT_A_MY_VECTOR_1_HPP

#include <cstddef>
#include <memory>
#include <utility>

template<typename T>
class Vector {

public:
//构造
    Vector():data_(nullptr),size_(0),capacity_(0){}
    Vector(const Vector&other):data_(nullptr),size_(other.size_),capacity_(other.capacity_){
        if(capacity_>0){
            data_=alloc_.allocate(capacity_);
            for(size_t i=0;i<size_;i++){
                new(data_+i)T(other.data_[i]);
            }
        }
    }
    Vector(Vector&&other)noexcept
    :data_(other.data_),size_(other.size_),capacity_(other.capacity_){
        other.size_=0;
        other.capacity_=0;
        other.data_=nullptr;
    }
/*//赋值运算符重载
    Vector& operator=(const Vector&other){
        if(this!=&other){
            for(size_t i=0;i<size_;i++){
                data_[i].~T();
            }
            if(data_){
                alloc_.deallocate(data_,capacity_);
            }
            size_=other.size_;
            capacity_=other.capacity_;
            if(capacity_>0){
                data_=alloc_.allocate(capacity_);
                for(size_t i=0;i<size_;i++){
                    new(data_+i)T(other.data_[i]);
                }
            }
            else data_=nullptr;
        }
        return *this;
    }
    Vector& operator=(Vector&&other)noexcept{
        if(this!=&other){
            for(size_t i=0;i<size_;i++){
                data_[i].~T();
            }
            if(data_){
                alloc_.deallocate(data_,capacity_);
            }
            data_=other.data_;
            size_=other.size_;
            capacity_=other.capacity_;
            other.size_=0;
            other.capacity_=0;
            other.data_=nullptr;
        }
        return*this;
    }*/
//Copy and Swap
    void swap(Vector &other)noexcept{
        std::swap(data_,other.data_);
        std::swap(size_,other.size_);
        std::swap(capacity_,other.capacity_);
    }
    Vector& operator=(Vector other){//遇右值移动构造，左值拷贝
        swap(other);
        return *this;
    }
//功能函数
    size_t size() const {return size_;}
    size_t capacity() const {return capacity_;}
    bool empty() const {return size_==0;}

    void reserve(size_t new_cap){//如果原地皮太少，扩容（换内存）
        if (new_cap<=capacity_){return;}
        //1.买地皮
        T* new_data=alloc_.allocate(new_cap);
        //2.搬家,用placement将旧元素移动到新内存
        for(size_t i=0;i<size_;i++){
            new(new_data+i)T(std::move_if_noexcept(data_[i]));//暂不深究
        }
        //3.拆旧房子（析构原size_个对象，不释放内存）
        for(size_t i=0;i<size_;i++){
            data_[i].~T();//T是内置类型，调用伪析构函数
        }
        //4.卖旧地皮
        if(data_!=nullptr){
            alloc_.deallocate(data_,capacity_);
        }
        //5.更新记录
        data_=new_data;
        capacity_=new_cap;
    }

    void push_back(const T& value){
    //  如果内存满了，扩容，换一块大一倍的地皮
    //  如果是空容器，申请一个地皮
    //  二者条件重合，二合一
        if(capacity_==size_){
            size_t new_cap=(capacity_==0)?1:capacity_*2;
            reserve(new_cap);
        }
        new(data_+size_)T(value);
        size_++;

    }
    void push_back(T&& value)noexcept{
    // 专门处理右值（临时对象）的 push_back
    // 1. 空间不够则扩容（逻辑与左值版本一致）
        if (size_ >= capacity_) {
            size_t new_cap = (capacity_ == 0) ? 1 : capacity_ * 2;
            reserve(new_cap);
        }

    // 2. 核心区别：使用 std::move 触发移动构造，而不是拷贝构造
        new (data_ + size_) T(std::move(value));

    // 3. 更新大小
        ++size_;
    }
    template<typename... Args>//无需先建T类型对象，可直接用T内容push
    T& emplace_back(Args&&... args){
        if(size_>=capacity_){
            size_t new_cap=(capacity_==0)?1:capacity_*2;
            reserve(new_cap);
        }
        new(data_+size_)T(std::forward<Args>(args)...);
        return data_[size_++];//return[size-1];size++
    }


    ~Vector() {
    //  析构函数：先拆所有房子，再退地皮
        for (size_t i = 0; i < size_; ++i) {
           data_[i].~T();
        }
        if (data_ != nullptr) {
            alloc_.deallocate(data_, capacity_);
        }
    }

// 重载 []，方便像数组一样访问
    T& operator[](size_t index) { return data_[index]; }
    const T& operator[](size_t index) const { return data_[index]; }
//迭代器
    T* begin(){return data_;}
    const T* begin() const{return data_;}
    T* end(){return data_+size_;}
    const T* end() const{return data_+size_;}

private:
    T* data_;
    size_t size_;
    size_t capacity_;//地皮个数
    std::allocator<T> alloc_;//地皮管理者 Mr./Mrs.alloc_
};

#endif
