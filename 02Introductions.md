# Modern C++ Introduction

- [Modern C++ Introduction](#modern-c-introduction)
  - [const](#const)
  - [auto](#auto)
    - [`auto` basic usage](#auto-basic-usage)
    - [`auto` with lambda](#auto-with-lambda)
    - [`auto` attention](#auto-attention)
  - [initializer\_list](#initializer_list)
    - [initializer\_list basic](#initializer_list-basic)
    - [initializer\_list attention](#initializer_list-attention)
  - [`virtual`, `override`, `final`](#virtual-override-final)
    - [basic usage](#basic-usage)
  - [range-base loop](#range-base-loop)
    - [range-based loop basic usage](#range-based-loop-basic-usage)

## const

To understand this, you read the declaration **from right to left**:
- `int& const` : a constant (const) reference (&) to int. Since references refer to the same object since their creation, they are always constant. The const is redundant. Some compilers warn about it, other emit errors.
- `int const&`: reference (&) to a constant (const) int. The integer can't be modified through the reference.
- `const& int`: integer to a reference of const. This makes no sense. This is **illegal**.
- `const int&` : a reference to an int object which happens to be const. **Equivalent to int const &**

For pointers:

```cpp
int const *    // pointer to const int
int * const    // const pointer to int
int const * const   // const pointer to const int
int * const * p3;   // pointer to const pointer to int
const int * const * p4;       // pointer to const pointer to const int

// Now the first const can be on either side of the type so:
const int * == int const *
const int * const == int const * const
```

## auto

### `auto` basic usage

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

### `auto` with lambda

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

### `auto` attention

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

## initializer_list

```cpp
T object{other}; // direct-list-initialization
T object={other}; // copy-list-initialization 
```

### initializer_list basic

```cpp
#include <iostream>
#include <string>
#include <vector>
#include <map>

class foo
{
    int a_;
    double b_;
public:
    foo():a_(0), b_(0) {}
    foo(int a, double b = 0.0):a_(a), b_(b) {}
    void print_value(){std::cout<<a_<<", "<<b_<<std::endl;}
};

int main()
{
    int a1{10};
    int a2={20};
    int a3=30; // normal
    std::cout<<a1<<", "<<a2<<std::endl;

    double* pd1{new double{10.1}};
    double* pd2={new double{20.1}};
    double* pd3 = new double(30.1); // normal
    double* pd4 = new double{40.1}; // normal
    std::cout<<*pd1<<", "<<*pd2<<std::endl;

    char c_arr1[3]{'a', 'b', 'c'};
    char c_arr2[3]={'A', 'B', 'C'};
    std::cout<<c_arr1[1]<<", "<<c_arr2[1]<<std::endl;
    
    float* pf1{new float[3]{1.1, 1.2, 1.3}};
    float* pf2={new float[3]{2.1, 2.2, 2.3}};
    float* pf3=new float[3]{2.1, 2.2, 2.3}; // normal
    std::cout<<pf1[1]<<", "<<pf2[1]<<std::endl;

    std::string s1{"string1"};
    std::string s2={"string2"};
    std::string s3("string3"); // normal
    std::string s4="string4"; // normal
    std::cout<<s1<<", "<<s2<<std::endl;

    // recommended
    foo f1{};
    f1.print_value();
    foo f2={10};
    f2.print_value();
    foo f3={20, 30.5};
    f3.print_value();

    foo f4; // normal
    // foo f4() // error
    foo f5(15);
    foo f6(16, 16.1);

    auto f7{foo()};
    auto f8={foo()};
    auto f9=foo(); // normal

    auto f10{foo(100, 0.1)};
    auto f11={foo(110, 0.1)};
    auto f12=foo(120, 0.1);//normal

    // initializer_list
    auto il1={1, 2, 3};
    // auto il2{4, 5, 6}; // error
    std::cout<<typeid(il1).name()<<std::endl; // initializer_list
    for(auto&& i:il1){std::cout<<i<<", ";}

    // vector
    std::vector<int> v1{11, 22, 33};
    std::vector<float> v2={1.1, 2.2, 3.3};

    // map
    std::map<int, std::string> m1{{1, "grey1"}, {2, "grey2"}}; 
    std::map<int, std::string> m2={{12, "grey12"}, {22, "grey22"}}; 
    // auto m3={{12, "grey12"}, {22, "grey22"}};//error
    for(auto&& p:m2){std::cout<<p.first<<':'<<p.second<<std::endl;}
}
```

high priority of `initializer_list`

```cpp
#include <iostream>

class foo
{
    int a_;
    double b_;
public:
    foo():a_(0), b_(0) {}
    foo(int a, double b = 0.0):a_(a), b_(b) {}
    foo(std::initializer_list<int> l) {} // high priority
    void print_value(){std::cout<<a_<<", "<<b_<<std::endl;}
};

void func(int const a, int const b, int const c)
{
    std::cout<<"call 1st overload function\n";
    std::cout<<a<<' '<<b<<' '<<c<<std::endl;
}

void func(std::initializer_list<int> l)
{
    std::cout<<"call 2nd overload function\n";
    for(auto&& i:l){std::cout<<i<<", ";}
}

int main()
{
    // initializer_list in ctor
    foo f1={1, 2, 3};
    f1.print_value(); // 1640197152, 2.07463e-317
    foo f2{1, 2};
    f1.print_value(); // 1640197152, 2.07463e-317
    foo f3(10, 20);
    f3.print_value(); // 10 20

    // initializer_list in normal function
    func(1, 2, 3); // 1st
    func({1, 2, 3}); // 2nd
}
```

### initializer_list attention

```cpp
#include <iostream>
#include <vector>

void print_vector(auto const v)
{
    for(auto&& i:v){std::cout<<i<<", ";}
}

int main()
{
    // attention1: vector
    std::vector<int> v1{5}; // size=1
    print_vector(v1); // 5, 
    std::vector<int> v2(5); // size=5
    print_vector(v2); // 0, 0, 0, 0, 0, 

    // attention2: implicit conversion
    // int i{1.2}; // error
    int i{static_cast<int>(1.2)};
    double d=47/13; 
    std::cout<<typeid(d).name()<<", "<<d<<std::endl; // double, 3
    // float f{d}; // error
    float f{static_cast<float>(d)};

    // attention3: auto deduce, since c++17
    auto a1={10};
    std::cout<<typeid(a1).name()<<std::endl; // initializer_list
    auto a2{20};
    std::cout<<typeid(a2).name()<<std::endl; // int
    auto a3={30, 31, 32};
    std::cout<<typeid(a3).name()<<std::endl; // initializer_list
    // auto a4{40, 41, 42}; // error
}
```

## `virtual`, `override`, `final`

### basic usage

```cpp
#include <iostream>

class Base
{
public:
    // 只要有virtual xxx=0, 就是纯虚函数，该类不允许实例化
    virtual void foo()=0;
    // 单纯的virtual虚函数表示可以用父类指针指向子类实例
    virtual void bar(){std::cout<<"Base bar() func.\n";}
    virtual void foobar()=0;
};

void Base::foobar(){
    std::cout<<"Base foobar() func.\n";
}

class Derived1: public Base
{
public:
    virtual void foo() override=0;
    // virtual void bar() override {std::cout<<"Derived1 bar() func.\n";} // 简化写法
    virtual void bar() override;
    virtual void foobar() override {std::cout<<"Derived1 foobar() func.\n";}
};

// void Derived1::bar() override // error, override只能在class的definition里面
void Derived1::bar()
{
    std::cout<<"Derived1 bar() func.\n";
}

class Derived2: public Derived1
{
public:
    virtual void foo() final {std::cout<<"Derived2 foo() func.\n";}
};

class Derived3 final: public Derived2
{
public:
    // virtual void foo(){} // error, 因为Derived2.foo已经final
    void bar() override {std::cout<<"Derived3 bar() func.\n";}
};

// class Derived4: public Derived3 {}; // error, 因为Derived3已经final


int main()
{
    // Base b{}; // error
    // Derived1 d1{}; // error
    Derived2 d2{};
    d2.foo();
    d2.bar();
    d2.foobar();

    Derived3 d3{};
    d3.bar();
}
```

## range-base loop

### range-based loop basic usage

```cpp
#include <iostream>
#include <vector>
#include <map>

void print(std::vector<int> v){
    for(int i:v){std::cout<<i<<',';}
    std::cout<<std::endl;
}

void print(std::multimap<bool,int> m){
    for(auto&& [first, second]:m){std::cout<<first<<':'<<second<<"; ";}
    std::cout<<std::endl;
}

auto getVector(){
    return std::vector<int>{1, 1, 2, 3, 4, 5};
}

auto getMap(){
    return std::multimap<bool,int>{
        {true, 1},
        {false, 2},
        {true, 3},
        {false, 4},
        {true, 5},
        {true, 6},
    };
}


int main(){
    auto v =getVector();
    // int
    for(int i:v){i*=2;} // 无法改变vector
    print(v); // 1, 1, 2, 3, 4, 5, 

    for(int& i:v){i*=2;} // 能够改变vector
    print(v); // 2, 2, 4, 6, 8, 10, 
    // auto 
    for(auto& i:v){i*=2;}
    print(v); // 4, 4, 8, 12, 16, 20, 

    for(auto const & i:v){std::cout<<i<<'\t';} // const:read-only

    for(auto&& i:v){i*=2;} // auto&&: universal
    print(v); // 8, 8, 16, 24, 32, 40, 

    // for(auto const && i:v){std::cout<<i<<"\t";} // error: cannot bind rvalue reference of type 'const int&&' to lvalue of type 'int'

    // since c++17
    auto m=getMap();
    for(auto [first, second]:m){
        if(first){second*=2;} // cannot change value
    }
    print(m); //0:2; 0:4; 1:1; 1:3; 1:5; 1:6; 


    for(auto& [first, second]:m){
        if(first){second*=2;}
    }
    print(m); // 0:2; 0:4; 1:2; 1:6; 1:10; 1:12; 

    for(auto&& [first, second]:m){
        if(first){second*=2;}
    }
    print(m); // 0:2; 0:4; 1:4; 1:12; 1:20; 1:24;
}
```