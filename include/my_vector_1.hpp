#include<memory>
#include<cstddef>
#include<iostream>
template<typename T>
class Vector{

public:
//构造
    Vector():data_(nullptr),size_(0),capacity_(0){}
    Vector(const Vector&other):data_(nullptr),size_(other.size_),capacity_(other.capacity_){
        if(capacity_>0){
            data_=alloc_.allocate(capacity_);
            for(int i=0;i<size_;i++){
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
//赋值运算符重载
    Vector& operator=(const Vector&other){
        if(this!=other){
            for(int i=0;i<size_;i++){
                data_[i].~T;
            }
            if(data_){
                alloc_.deallocate(size_,capacity_);
            }
            size_=other.size_;
            capacity_=other.capacity_;
            if(capacity_>0){
                for(int i=0;i<size_;i++){
                    new(data_+i)T(other.data_[i]);
                }
            }
            else data_=nullptr;
        }
        return *this;
    }
    Vector& operator=(Vector&&other)noexcept{
        if(this!=other){
            for(int i=0;i<size_;i++){
                data_[i].~T;
            }
            if(data_){
                alloc_.deallocate(size_,capacity_);
            }
            data_=other.data_;
            size_=other.size_;
            capacity_=other.capacity_;
            other.size_=0;
            other.capacity_=0;
            other.data_=nullptr;
        }
        return*this;
    }
//功能函数
    size_t size(){return size_;}
    size_t capacity(){return capacity_;}
    bool empty(){return size_==0;}
    const size_t size(){return size_;}
    const size_t capacity(){return capacity_;}
    const bool empty(){return size_==0;}

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

    void push(const T& value){
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
    void push(T&& value)noexcept{
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
    const T* begin(){return data_;}
    T* end(){return data_+size_;}
    const T* end(){return data_+size_;}

private:
    T* data_;
    size_t size_;
    size_t capacity_;//地皮个数
    std::allocator<T> alloc_;//地皮管理者 Mr./Mrs.alloc_
};
/*int main(){
    Vector<int> v;
    std::cout<<v.size()<<","<<v.capacity()<<std::endl;
}*/

#include <cassert>
int main() {
    Vector<int> v;
    for (int i = 0; i < 1000000; ++i) {
        v.push(i);
    }
    
    assert(v.size() == 1000000);
    
    long long sum = 0;
    for (size_t i = 0; i < v.size(); ++i) {
        sum += v[i];
    }
    
    assert(sum == 499999500000LL);
    std::cout << "1e6 test passed!\n";

    Vector<std::string> strVec;
    std::string longStr(1000, 'A');
    strVec.push(std::move(longStr)); // 触发移动语义
    std::cout << "Vector size: " << strVec.size() << std::endl;
    std::cout << "Original string size: " << longStr.size() << std::endl;
    return 0;
}