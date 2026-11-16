#include<memory>
#include<cstddef>
#include<iostream>
template<typename T>
class Vector{

public:
    Vector():data_(nullptr),size_(0),capacity_(0){}
    size_t size(){return size_;}
    size_t capacity(){return capacity_;}
    bool empty(){return size_==0;}

    void reserve(size_t new_cap){//如果原地皮太少，扩容（换内存）
        if (new_cap<=capacity){return;}
        //1.买地皮
        T* new_data=alloc.allocate(new_cap);
        //2.搬家,用placement将旧元素移动到新内存
        for(size_t i=0;i<size_;size++){
            new(new_data+i)T(std::move_if_noexcept(data_[i]));//暂不深究
        }
        //3.拆旧房子（析构原size_个对象，不释放内存）
        for(size_t i=0;i<size_;i++){
            data_[i].T~();//T是内置类型，调用伪析构函数
        }
        //4.卖旧地皮
        if(data_!=nullptr){
            alloc.deallocate(data_,capacity_);
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
    

private:
    T* data;
    size_t size_;
    size_t capacity_;//地皮个数
    std::allocator<T> alloc_;//地皮管理者 Mr./Mrs.alloc_
};
int main(){
    Vector<int> v;
    std::cout<<v.size()<<","<<v.capacity()<<std::endl;
}

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
    return 0;
}