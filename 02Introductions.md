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
    - [custom range-base class](#custom-range-base-class)
  - [explicit](#explicit)
    - [basic usage](#basic-usage-1)
  - [unamed namespace for static globals](#unamed-namespace-for-static-globals)
  - [inline namespace for versioning](#inline-namespace-for-versioning)
  - [structured bindings for multi-return](#structured-bindings-for-multi-return)
  - [template argument deduction](#template-argument-deduction)

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

### custom range-base class

```cpp
#include <iostream>

template <typename T, size_t const Size>
class dummy_array{
    T data[Size]={};
public:
    T const & GetAt(size_t const index) const{
        if(index<Size) return data[index];
        throw std::out_of_range("index out of range");
    }

    void SetAt(size_t const index, T const & value){
        if(index<Size) data[index]=value;
        else throw std::out_of_range("index out of range");
    }

    size_t GetSize() const {return Size;}
};

template <typename T, typename C, size_t const Size>
class dummy_array_iterator_type
{
    size_t index;
    C& collection;
public:
    dummy_array_iterator_type(C& collection, size_t const index) :collection(collection),index(index){}

    bool operator!= (dummy_array_iterator_type const & other) const{
        return index != other.index;
    }

    T const & operator* () const{
        return collection.GetAt(index);
    }

    dummy_array_iterator_type& operator++ (){
        ++index;
        return *this;
    }

    dummy_array_iterator_type operator++ (int){
        auto temp = *this;         
        ++*this;
        return temp;
    }
};

template <typename T, size_t const Size>
using dummy_array_iterator = dummy_array_iterator_type<T, dummy_array<T, Size>, Size>;

template <typename T, size_t const Size>
using dummy_array_const_iterator = dummy_array_iterator_type<T, dummy_array<T, Size> const, Size>;

template <typename T, size_t const Size>
// inline dummy_array_iterator<T, Size> begin(dummy_array<T, Size>& collection)
inline auto begin(dummy_array<T, Size>& collection){
    return dummy_array_iterator<T, Size>(collection, 0);
}

template <typename T, size_t const Size>
// inline dummy_array_const_iterator<T, Size> begin(dummy_array<T, Size> const & collection)
inline auto begin(dummy_array<T, Size> const & collection){
    return dummy_array_const_iterator<T, Size>(collection, 0);
}

template <typename T, size_t const Size>
// inline dummy_array_iterator<T, Size> end(dummy_array<T, Size>& collection)
inline auto end(dummy_array<T, Size>& collection){
    return dummy_array_iterator<T, Size>(collection, collection.GetSize());
}

template <typename T, size_t const Size>
// inline dummy_array_const_iterator<T, Size> end(dummy_array<T, Size> const & collection)
inline auto end(dummy_array<T, Size> const & collection){
    return dummy_array_const_iterator<T, Size>(collection, collection.GetSize());
}

template <typename T, const size_t Size>
void print_dummy_array(dummy_array<T, Size> const & arr)
{
    std::cout<<std::endl;
    for (auto && e : arr){std::cout << e << ';';}
}

int main(){
    dummy_array<int, 3> arr;
    arr.SetAt(0, 1);
    arr.SetAt(1, 2);
    arr.SetAt(2, 3);

    for(auto&& i:arr){
        std::cout<<i<<';';
    }

    print_dummy_array(arr);
}
```

## explicit

### basic usage

without `explicit`

```cpp
#include <iostream>

struct foo{
    foo() { std::cout << "foo" << '\n'; }
    foo(int const a) { std::cout << "foo(a)\n"; }
    foo(int const a, double const b) { std::cout << "foo(a, b)\n"; }
    //// 如果使用这一句，下面的f8, f9都会报错 double to int
    //// 因为initializer_list的类型需要一致为int
    // foo(std::initializer_list<int> l){std::cout<<"foo(l)\n";} 

    operator bool() const { return true; }
};

void bar(foo const f){}

int main(){
    foo f1;
    foo f2{};

    foo f3(1);
    foo f4=1; // 传统方式，不推荐; foo(a)
    foo f5{1};
    foo f6={1};

    foo f7(1, 2.1);
    foo f8{1, 2.2};
    foo f9={1, 2.3};
    
    bar({}); // foo
    bar(1); // 传统方式, foo(a)
    bar({1, 2.4});// foo(a, b)

    std::cout<< (f1==true)<<std::endl; // true
}
```

with explicit

```cpp
#include <iostream>

class foo{
    int num;
public:
    // constructor
    foo(int const n):num(n){std::cout<<"foo num="<<num<<std::endl;}
    // convert object to bool
    operator bool() const {
        std::cout<<"foo conversion invoked: "<<num<<std::endl;
        return num!=0;
    }
};

class Demo{
    int num;
public:
    explicit Demo(int const n):num(n){std::cout<<"Demo num="<<num<<std::endl;}
    explicit operator bool() const {
        std::cout<<"Demo conversion explicit invoked: "<<num<<std::endl;
        return num!=0;
    }
};

enum ItemSize{Small, Middle, Large}; // unscoped enumeration, not recommended

int main(){
    // without explicit, all working
    foo f1(10);
    foo f2('a'); // 97
    foo f3=20;
    foo f4='b';
    foo f5{30};
    foo f6{'c'};
    foo f7={40};
    foo f8={'d'};
    foo f9={Small}; // 0
    if(f1==f2) std::cout<<"bool equal\n"; // conversion invoked, bool equal
    std::cout<<f1+f2<<std::endl; // conversion invoked, 2


    // 三种方法可以: (),direct-initializer-list{},static_cast
    Demo d1(10);
    Demo d2('a');

    // // error, conversion from int to Demo
    // Demo d3=20;
    // Demo d4='b';

    Demo d5{30};
    Demo d6{'c'};

    //// error, conversion from int to Demo
    // Demo d7={40};
    // Demo d8={'d'};

    Demo d9{Middle}; // 1, from enum to int

    Demo d10 = static_cast<Demo>('e');
    Demo d11 = static_cast<Demo>(Large);
    Demo d12 = static_cast<Demo>(50);

    // if(d1==d2){} // error: no match for 'operator=='
    // std::cout<<d1+d2<<std::endl; // error: no match for 'operator+'
    if(static_cast<bool>(d1)==static_cast<bool>(d2)) std::cout<<"bool equal\n"; // conversion explicit invoked
    std::cout<< static_cast<bool>(d1) + static_cast<bool>(d2) <<std::endl;// 2
}
```

example: scoped enumeration

```cpp
#include <iostream>

enum class Platform{
    LDP,
    SWAP,
};

void func(Platform p){
    if(p==Platform::LDP){
        std::cout<<"ldp"<<std::endl;
    }else if(p==Platform::SWAP){
        std::cout<<"swap"<<std::endl;
    }
}

int main()
{
    auto p=Platform::SWAP;
    func(p); // swap
}
```

## unamed namespace for static globals

```bash
├── CMakeLists.txt
├── main.cpp
├── mylib1.cpp
├── mylib1.h
├── mylib2.cpp
└── mylib2.h
```

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.16.0)
project(myproj VERSION 0.1.0)

add_executable(
    myproj
    main.cpp
    mylib1.cpp
    mylib2.cpp
    )
```

```h
// mylib1.h
#pragma once

namespace myspace
{
    void file1_run();
} // named namespace
```

```h
// mylib2.h
#pragma once

namespace myspace
{
    void file2_run();
} // named namespace
```

> mylib1.cpp使用全局的变量`a`和函数`print`，不再mylib2.cpp同名的变量`a`和函数`print`发生冲突
>
> C语言里面解决该问题的方式是每个文件里面的全局变量或函数使用`static`来修饰；C++里面使用`unamed namespace`

```cpp
// mylib1.cpp
#include "mylib1.h"
#include <iostream>

namespace
{
    int a=100;
    void print(std::string msg){std::cout<<"[file1] "<<msg<<','<<a<<std::endl;}
} // unamed namespace

namespace myspace
{
    void file1_run(){
        print("run in file1");
    }
}
```

```cpp
// mylib2.cpp
#include "mylib2.h"
#include <iostream>

namespace
{
    int a=200;
    void print(std::string msg){std::cout<<"[file2] "<<msg<<','<<a<<std::endl;}
} // unamed namespace

namespace myspace
{
    void file2_run(){
        print("run in file2");
    }
}
```

```cpp
// main.cpp
#include "mylib1.h"
#include "mylib2.h"

using namespace myspace;

int main(int argc, const char** argv) {
    file1_run();
    file2_run();
    return 0;
}
```

Template non-type argument(模板非类型参数)要求: 
- Constant Expressions
- Addresses of function or objects with external linkage
- Addresses of static class members.

```cpp
#include <iostream>
#include <vector>

// swap two num 
template <class T>
void swap_(T* x, T* y){
    T temp = *x;
    *x = *y;
    *y = temp;
}
  
// Bubble Sort
template <class T, int size> // int size就是template non-type argument
void bubble_sort(T arr[]){
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                swap_(&arr[j], &arr[j + 1]);
            }
        }
    }
}
  
// print an array
template <class T, int size> // int size就是template non-type argument
void printArray(T arr[]){
    for (int i = 0; i < size - 1; i++) {
        std::cout << arr[i] << ", ";
    }
    std::cout << arr[size - 1] << std::endl;
}
  
int main(){
    float arr[] = { 1.1, 1.2, 0.3, 4.55, 1.56, 0.6 };

    const int size_arr = sizeof(arr) / sizeof(arr[0]); // 必须是const，使complier能够编译的时候确定
    bubble_sort<float, size_arr>(arr);

    std::cout << "Sorted Array is: ";
    printArray<float, size_arr>(arr);
}
```

```cpp
// error example
template <int const& Size>
class test {};

static int Size1 = 10;

namespace
{
   int Size2 = 10;
}

// test<Size1> t1; // error, Size1 has internal linkage, 在VC++报错,Clang, GCC不报错
test<Size2> t2;
  
int main(){}
```

## inline namespace for versioning

> if a namespace, A, contains an inline namespace, B, that contains an inline namespace, C, then the members of C appear as they were members of both B and A and the members of B appear as they were members of A.

```bash
├── CMakeLists.txt
├── main.cpp
└── mylib1.h
```

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.16.0)
project(myproj VERSION 0.1.0)

add_executable(
    myproj
    main.cpp
    )
```

without `inline namespace`

```h
// mylib1.h
#pragma once

namespace modernlib
{
    // template只能放到hearder文件里面
    template<typename T>
    int test(T value){return 1;}
}
```

```cpp
// main.cpp
#include <iostream>
#include "mylib1.h"

struct foo{int a;};

namespace modernlib // 用户给库开发者写的modernlib增加自定义的功能
{
    template<>
    int test(foo value){return value.a;}
}

int main(int argc, const char** argv) {
    auto x=modernlib::test(42);
    auto y=modernlib::test(foo{42});
    std::cout<<x<<'\t'<<y<<std::endl; // 1 42
    return 0;
}
```

with `inline namespace`

```h
// mylib1.h
// 库开发者更新版本，但是第一个版本的功能接口仍然提供
#pragma once

namespace modernlib
{
    #ifndef LIB_VERSION_2
    inline namespace version_1
    {
        template<typename T>
        int test(T value){return 1;}
    }
    #endif // LIB_VERSION_1

    #ifdef LIB_VERSION_2
    inline namespace version_2
    {
        template<typename T>
        int test(T value){return 2;}
    }
    #endif // LIB_VERSION_2
}
```

```cpp
// main.cpp
#include <iostream>
#define LIB_VERSION_2
#include "mylib1.h"

struct foo{int a;};

namespace modernlib
{
    template<>
    int test(foo value){return value.a;}
}

int main(int argc, const char** argv) {
    auto x=modernlib::test(42);
    auto y=modernlib::test(foo{42});
    std::cout<<x<<'\t'<<y<<std::endl; // 1 42
    return 0;
}
```

## structured bindings for multi-return

```cpp
#include<iostream>
#include<map>

int main(){
    std::map<int, std::string> m;
    {
        // 采用code block{}是因为下面几个it, inserted变量名相同
        // 如果要去掉{}; 要么采用不同的变量名，要么使用std::tie()
        auto [it, inserted]=m.insert({1, "one"});
        std::cout<<it->first<<':'<<it->second<<std::endl; // 1:one
        std::cout<<inserted<<std::endl; // true
    }
    {
        auto [it, inserted]=m.insert({1, "two"}); // insert fail
        std::cout<<it->first<<':'<<it->second<<std::endl; // 1:one
        std::cout<<inserted<<std::endl; // false
    }
    {
        if(auto [it, inserted]=m.insert({1, "three"}); inserted){ // insert fail
            std::cout<<it->first<<':'<<it->second<<std::endl; // 1:one
        } else{
            std::cout<<"insert fail\n";
        }

    }

    std::map<int, std::string>::iterator it;
    bool inserted;
    std::tie(it, inserted)=m.insert({1, "four"});
    std::cout<<it->first<<':'<<it->second<<std::endl; // 1:one
    std::cout<<inserted<<std::endl; // false
}
```

```cpp
#include<iostream>

struct foo{
    int id;
    std::string name;
};

int main(){
    // for array
    int arr[]={1, 2, 3};
    auto [a, b, c]=arr;
    auto& [x, y, z]=arr;
    x+=10;
    y+=20;
    z+=30;
    std::cout<<arr[0]<<'\t';// 11
    std::cout<<arr[1]<<'\t';// 22
    std::cout<<arr[2]<<"\n";// 33
    std::cout<<a<<'\t'<<b<<'\t'<<c<<std::endl; // 1 2 3
    std::cout<<x<<'\t'<<y<<'\t'<<z<<std::endl; // 11 22 33

    // for class
    foo f{42, "Peter"};
    auto [id, name]=f;
    auto& [r_id, r_name]=f;
    r_id=52;
    r_name="Tony";
    std::cout<<id<<'\t'<<name<<std::endl; // 42 Peter
    std::cout<<r_id<<'\t'<<r_name<<std::endl; // 52 Tony
    std::cout<<f.id<<'\t'<<f.name<<std::endl; // 52 Tony
    
    // with lambda, since c++20
    auto l1=[id]{std::cout<<id<<std::endl;};
    l1(); // 42
    auto l2=[=]{std::cout<<id<<','<<name<<std::endl;}; // implicitly capture all value by value
    l2(); // 42,Peter
    auto l3=[&id]{id+=100;};
    l3(); 
    auto l4=[&]{id+=1000;name="Jack";}; // implicitly capture all value by reference
    l4(); 
    std::cout<<id<<'\t'<<name<<std::endl; // 1142 Jack

    std::cout<<f.id<<'\t'<<f.name<<std::endl; // 52 Tony
}
```

## template argument deduction

```cpp
#include<iostream>
#include<vector>

template<typename T>
class foo{
    T data;
public:
    foo(T v):data(v){std::cout<<data<<std::endl;}
};

template<typename T>
foo<T> make_foo(T&& value){
    return foo{value};
}

int main(){
    std::pair<int, char const *> p1{42, "demo"};
    std::pair p2{43, "hello"}; // since c++17
    auto p3=std::make_pair(44, "grey"); // before c++17

    std::vector<int> v1{1, 2, 3};
    std::vector v2{11, 22, 33}; // since c++17

    foo<int> f1{100};
    foo f2{200}; // since c++17
    foo<float> f3{1.1};
    foo f4{2.2};// since c++17
    auto f5=make_foo(300); // before c++17
}
```

example: 使得`std::pair p2{43, "hello"};`推断为`std::pair<int, std::string>`而不是`std::pair<int, char const*>`
```cpp
#include<iostream>

namespace std {
   template <class T>
   pair(T&&, char const*)->pair<T, std::string>;

   template <class T>
   pair(char const*, T&&)->pair<std::string, T>;

   pair(char const*, char const*)->pair<std::string, std::string>;
}

int main(){
    std::pair p1{1, "hello"};
    // std::pair<> p2{2, "world"}; // error
    // std::pair<int> p3{3, "tom"}; // error
    std::cout<<typeid(p1.second).name()<<std::endl; // basic_string
}
```