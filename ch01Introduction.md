# Modern C++ Introduction

- [Modern C++ Introduction](#modern-c-introduction)
  - [auto](#auto)
    - [auto一般用法](#auto一般用法)
    - [auto with lambda](#auto-with-lambda)
    - [auto 注意事项](#auto-注意事项)

## auto

### auto一般用法

```cpp
#include <iostream>

void func1(char const x)
{
    // std::tolower返回值是int
    auto myupper =[](char const c){return std::tolower(c);};
    auto a=myupper(x);
    std::cout<<typeid(a).name()<<' '<<a<<std::endl;
}

void func2(auto const x, auto const y) // 形参auto需要-std=c++20
{
    auto myadd=[](auto const a, auto const b){return a+b;};
    auto a=myadd(x, y);
    std::cout<<typeid(a).name()<<' '<<a<<std::endl;
}

template <typename F, typename T>
auto apply(F&& f, T value)
{
    return f(value);
}

auto increase(int x)
{
    return x+10;
}

auto decrease(double x)
{
    return x-0.1;
}

void func3()
{
    auto a=apply(increase, 1);
    std::cout<<typeid(a).name()<<' '<<a<<std::endl;

    auto b=apply(decrease, 10);
    std::cout<<typeid(b).name()<<' '<<b<<std::endl;
}

int main()
{
    func1('a');
    func1('B');
    
    func2(10, 20); // i
    func2(10.1, 20.2); // d
    func2(10, 20.2); // d
    
    func3();
}
```

### auto with lambda

> [位于成员函数尾部的const](https://blog.csdn.net/qq_55621259/article/details/126322634)

```cpp
#include <iostream>
#include <string>

struct{
    template<typename T, typename U>
    auto operator() (T const a, U const b) const // struct或class才能在函数后加const, 表示不修改成员类变量
    {
        return a+b;
    }
} L; // L是一个object不是type

struct {
    float operator() (int x, int y) const { return ((float)x)/y; }
} int_div; //  int_div是object不是type

struct int_multiply { // int_multiply是type
    float operator() (int x, int y) const { return ((float)x)*y; }
} ;

int main()
{
    auto x1=L(10, 20);
    std::cout<<typeid(x1).name()<<' '<<x1<<std::endl;
    
    auto x2=L(10.1, 20.2);
    std::cout<<typeid(x2).name()<<' '<<x2<<std::endl; // double

    auto x3=L(10, 20.2);
    std::cout<<typeid(x3).name()<<' '<<x3<<std::endl; // double

    using namespace std::literals;
    // 末尾s是literal operator s
    auto x4=L("hello"s, "world"s); // std::string
    std::cout<<typeid(x4).name()<<' '<<x4<<std::endl; // helloworld

    auto x5=int_div(10, 20);
    std::cout<<typeid(x5).name()<<' '<<x5<<std::endl; // float

    auto x6=int_multiply();
    std::cout<<typeid(x6).name()<<std::endl; // object
    auto x7=x6(10, 20);
    std::cout<<typeid(x7).name()<<' '<<x7<<std::endl; // float
}
```

### auto 注意事项

```cpp
#include <iostream>

class foo
{
    int x_;
public:
    foo(int const x = 0) :x_{ x } {}
    int& get() { return x_; }
};

int main()
{
    auto f=foo(42);
    auto x1=f.get();
    x1=100;
    std::cout<<typeid(x1).name()<<' '<<x1<<std::endl;
    std::cout<<f.get()<<std::endl; //42

    auto& x2=f.get(); // auto& 之后才能修改
    x2=200;
    std::cout<<typeid(x2).name()<<' '<<x2<<std::endl;
    std::cout<<f.get()<<std::endl; // 200
}
```