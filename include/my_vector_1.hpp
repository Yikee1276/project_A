#include<memory>
#include<cstddef>
#include<iostream>
#include<utility>

template<typename T>
class Vector{

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

#include <iostream>
#include <string>

// 1. 定义一个带计数器的测试类
struct Foo {
    int id;
    std::string name;
    static int copy_count;
    static int move_count;
    
    Foo(int i, std::string n) : id(i), name(std::move(n)) {}
    
    // 拷贝构造（记录次数）
    Foo(const Foo& other) : id(other.id), name(other.name) {
        copy_count++;
    }
    
    // 移动构造（记录次数，必须是 noexcept 才能被 move_if_noexcept 优先选中）
    Foo(Foo&& other) noexcept : id(other.id), name(std::move(other.name)) {
        move_count++;
    }
};

// 初始化静态成员
int Foo::copy_count = 0;
int Foo::move_count = 0;

int main() {
    std::cout << "=== 测试 1：emplace_back 原地构造与扩容 ===\n";
    {
        Vector<Foo> vec;
        
        // 插入第 1 个元素：不触发扩容，原地构造
        vec.emplace_back(1, "one"); 
        
        // 插入第 2 个元素：触发扩容！
        // 此时旧元素 "one" 会被 move_if_noexcept 移动到新内存（+1 Move）
        // 新元素 "two" 是原地构造（+0 Move, +0 Copy）
        vec.emplace_back(2, "two"); 
        
        std::cout << "Copies: " << Foo::copy_count << " (预期: 0)\n";
        std::cout << "Moves: " << Foo::move_count << " (预期: 1)\n";
    }
    
    // 重置计数器，准备下一轮测试
    Foo::copy_count = 0;
    Foo::move_count = 0;
    
    std::cout << "\n=== 测试 2：const 对象访问验证 ===\n";
    {
        Vector<Foo> vec;
        vec.emplace_back(10, "const_test");
        
        // 创建一个 const 的 Vector 引用
        const Vector<Foo>& const_vec = vec;
        
        // 验证 const 版本的 operator[] 和 begin()
        // 如果 Bug 2 没修好，这两行会编译报错
        std::cout << "Const access: " << const_vec[0].id << ", " << const_vec[0].name << "\n";
        
        for (const auto& item : const_vec) {
            std::cout << "Range for: " << item.id << ", " << item.name << "\n";
        }
    }
    
    std::cout << "\n=== 测试 3：1e6 int 压测 ===\n";
    {
        Vector<int> intVec;
        for (int i = 0; i < 1000000; ++i) {
            intVec.emplace_back(i);
        }
        std::cout << "1e6 test passed! Size: " << intVec.size() << "\n";
    }
    
    return 0;

    std::cout << "\n=== 测试 4：Copy-and-Swap 自赋值验证 ===\n";
    {
        Vector<Foo> vec;
        vec.emplace_back(100, "self_assign");
        
        // 极其危险的自赋值操作！旧代码必崩，新代码稳如泰山
        vec = vec; 
        
        std::cout << "Self-assign success! Data: " << vec[0].id << ", " << vec[0].name << "\n";
    }
}