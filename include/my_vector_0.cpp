/*
new delete申内存，构造，析构，还内存
1 allocate deallocate(类型感知)只申内存，还内存
2 operator new operator delete类似上
placement 已有内存上构造
*/
#include<iostream>
#include<memory>//1
#include<new>//2
typedef struct  Foo{
    int i;
    Foo(int i){
        std::cout<<"构造Foo"<<i<<"\n";
    }
    ~Foo(){
        std::cout<<"析构Foo"<<"\n";
    }
}Foo;
int main(){
    std::cout<<"new delete"<<"\n";
    Foo*p1=new Foo(1);
    delete p1;

    std::cout<<"operator new delete C底层风格"<<"\n";
    void*mem1=::operator new(sizeof(Foo));
    Foo*p2=static_cast<Foo*>(mem1);
    new(p2)Foo(2);
    p2->~Foo();
    ::operator delete(p2);

    std::cout<<"allocate deallocate Vector专属"<<"\n";
    std::allocator<Foo> alloc;//实例化模板类，得到一个名为alloc的分配器，管理Foo类型内存
    Foo*p3=alloc.allocate(1);//申请一个Foo类型内存空间
    new(p3)Foo(3);
    p3->~Foo();
    alloc.deallocate(p3,1);
    
    return 0;
}   
