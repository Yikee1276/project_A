#ifndef MY_TEMPLATE_H
#define MY_TEMPLATE_H
#include <cstddef>

template<typename T>
class Triple{
private:
    T a_,b_,c_;    
public:
    Triple(const T &a,const T &b,const T &c):
    a_(a),b_(b),c_(c)
    {}
    T GetMax()const;
};
template<typename T>
T Triple<T>::GetMax() const{
    T m=a_;
    m=a_<b_?b_:a_;
    m=m<c_?m:c_;
    return m;
}

template<typename T,int N>
class StaticArray{
private:
    T data_[N];
public:
    using value_type=T;
    using iterator=T*;
    iterator begin(){
        return data_;
    }
    iterator end(){
        return data_+N;
    }
    int size()const{
        return N;
    }
    void fill(const T& value){
        for(int i=0;i<N;i++){
            data_[i]=value;
        }
    }
};


#endif