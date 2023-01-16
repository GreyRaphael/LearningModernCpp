# Preprocessing and Compilation

- [Preprocessing and Compilation](#preprocessing-and-compilation)
  - [Conditional Compilaton](#conditional-compilaton)
  - [stringification \& concatenation](#stringification--concatenation)
  - [`alignas`, `alignof`, `sizeof`](#alignas-alignof-sizeof)
  - [`static_assert`](#static_assert)
  - [`std::enable_if`](#stdenable_if)
  - [`constexpr`](#constexpr)

## Conditional Compilaton

```cpp
#ifndef I2SWAP_CUSTOMCONVERTER_H
#define I2SWAP_CUSTOMCONVERTER_H

#include <string>
#include <cstring>

#ifdef _WIN32
    #include "windows.h"
#elif __linux__
    #include "iconv.h"
#endif
```

```cpp
#include <iostream>

void show_compiler()
{
#if defined _MSC_VER
   std::cout << "Visual C++\n";
#elif defined __clang__
   std::cout << "Clang\n";
#elif defined __GNUG__
   std::cout << "GCC\n";
#else
   std::cout << "Unknown compiler\n";
#endif
}

void show_compiler_version()
{
#if defined _MSC_VER

#if (_MSC_VER >= 1900)
   std::cout << "Visual C++ 2019 or newer\n";
#else
   std::cout << "Visual C++ " << _MSC_FULL_VER << '\n';
#endif

#elif defined __clang__

#if (__clang_major__ == 3) && (__clang_minor__ >= 9)
   std::cout << "Clang 3.9 or newer\n";
#else
   std::cout << "Clang " << __clang_version__ << '\n';
#endif

#elif defined __GNUG__

#if __GNUC__ >= 5 && __GNUC_MINOR__ > 0
   std::cout << "GCC 5.0 or newer\n";
#else
   std::cout << "GCC " << __VERSION__ << '\n';
#endif

#else
   std::cout << "Unknown compiler" << '\n';
#endif
}

void show_architecture()
{
#if defined _MSC_VER

#if defined _M_X64
   std::cout << "AMD64\n";
#elif defined _M_IX86
   std::cout << "INTEL x86\n";
#elif defined _M_ARM
   std::cout << "ARM\n";
#else
   std::cout << "unknown\n";
#endif

#elif defined __clang__ || __GNUG__

#if defined __amd64__
   std::cout << "AMD64\n";
#elif defined __i386__
   std::cout << "INTEL x86\n";
#elif defined __arm__
   std::cout << "ARM\n";
#else
   std::cout << "unknown\n";
#endif

#else
#error Unknown compiler
#endif
}

void show_configuration()
{
#ifdef _DEBUG
   std::cout << "debug\n";
#else
   std::cout << "release\n";
#endif
}

int main()
{
   show_compiler();
   show_compiler_version();
   show_architecture();
   show_configuration();
}
```

## stringification & concatenation

```cpp
#include <iostream>

#define MAKE_STR2(x) #x
#define MAKE_STR(x) MAKE_STR2(x)

#define MERGE2(X, Y) X##Y
#define MERGE(X, Y) MERGE2(X, Y)
#define Jitterbug 3

#define DECL_MAKE(x) DECL_MAKE2(x)
// 下划线_前后应该使用##分隔开
// #define DECL_MAKE2(x) x* make##_##x() { return new x(); }
#define DECL_MAKE2(x) x* make_##x() { return new x(); }

struct bar {int x=10;};
struct foo {double y=2;};

// creat instance of each classes
DECL_MAKE(foo)
DECL_MAKE(bar)

int main()
{
    std::string s1=MAKE_STR2(apple);
    std::cout<<s1<<std::endl; //apple
    std::string s2=MAKE_STR(hello);
    std::cout<<s2<<std::endl; //hello

    #define NUMBER 42
    std::string s3=MAKE_STR2(NUMBER);
    std::cout<<s3<<std::endl; //NUMBER
    // attention, 嵌套之后，变成了42而不是NUMBER
    std::string s4=MAKE_STR(NUMBER);
    std::cout<<s4<<std::endl; //42

    auto MERGE2(variable, 2) = 10;
    std::cout<<variable2<<std::endl; //10

    auto a = MERGE(Jitter, bug);
    std::cout<<a<<std::endl; //3

    auto b = make_bar(); // b is a bar*
    auto f = make_foo(); // f is a foo*
    std::cout<<b->x<<std::endl;
    std::cout<<f->y<<std::endl;
}
```

## `alignas`, `alignof`, `sizeof`

> `alignas(num)`: num必须是 8, 16, 32, 64...

1. 结构体中元素是按照定义顺序一个个放到内存中的，但并不是紧密排列的。从结构体存储的首地址开始，每一个元素放置到内存中时，它都会认为内存是以它自己的大小来划分的，因此元素放置的位置一定会在自己大小的整数倍上开始（以结构体变量首地址为0计算）。
2. 按照第一规则进行内存分配后，需要检查计算分配的内存大小是否为所有成员中大小最大的成员的大小的整数倍，若不是，则补齐为它的整数倍。
3. 如果有alignas修饰，需要在完成规则1和规则2之后，按照alignas来对齐
4. 如果一个结构体B里嵌套另一个结构体A，则结构体A应从offset为A内部最大成员的整数倍的地方开始存储。（struct B里存有struct A，A里有char，int，double等成员，那A应该从8的整数倍开始存储。），结构体A中的成员的对齐规则仍满足规则1、规则2。

```cpp
struct  test1{
	char c; // 1 byte
	int  i; // 4 bytes
	double d; // 8 bytes
};

// 编译器处理的结果
struct  test1{
	char c; // 1 byte
    char pad_0[3]; // padding 3
	int  i; // 4 bytes, padding 0
	double d; // 8 bytes, padding 0
}; // sizeof(test1) is 16, alignof(test1) is 8

// test1 等价于 test2
struct alignas(8) test2{
	char c;
	int  i;
	double d;
};
```

> 假设char的内存地址是0, int内存地址是4的倍数, 所以char后面padding了3个；  
> double来了之后，前面的正好凑齐8的倍数，所以紧挨着int;  
> 并且最终的大小是double大小的整数倍;  
> 最终size=1+3+4+8=16

```cpp
struct alignas(16) test3{
	char c;
	int  i;
	double d;
};

// 编译器处理的结果
struct alignas(16) test3{
	char c; // 1 byte
    char pad_0[3]; // padding 3
	int  i; // 4 bytes, padding 0
	double d; // 8 bytes, padding 0
}; // sizeof(test3) is 16, alignof(test3) is 16
```

> 假设char的内存地址是0, int内存地址是4的倍数, 所以char后面padding了3个；  
> double来了之后，前面的正好凑齐8的倍数，所以紧挨着int;  
> 并且最终的大小是alignas(16)大小的整数倍;  
> 最终size=1+3+4+8=16

```cpp
struct alignas(32) test4{
	char c; // 1 byte
    char pad_0[3]; // padding 3
	int  i; // 4 bytes, padding 0
	double d; // 8 bytes, padding 0
    char pad_0[16]; // padding 16
};
```

> 假设char的内存地址是0, int内存地址是4的倍数, 所以char后面padding了3个；  
> double来了之后，前面的正好凑齐8的倍数，所以紧挨着int;  
> 并且最终的大小不是alignas(32)大小的整数倍;  
> 最终size=1+3+4+8+16=32

```cpp
#include <iostream>

struct  test1{
	char c;
	int  i;
	double d;
};
 
struct alignas(8) test2{
	char c;
	int  i;
	double d;
};
 
struct alignas(16) test3{
	char c;
	int  i;
	double d;
};
 
struct alignas(32) test4{
	char c;
	int  i;
	double d;
};

int main()
{
    std::cout<<sizeof(test1)<<','<<alignof(test1)<<std::endl; // 16, 8
    std::cout<<sizeof(test2)<<','<<alignof(test2)<<std::endl; // 16, 8
    std::cout<<sizeof(test3)<<','<<alignof(test3)<<std::endl; // 16, 16
    std::cout<<sizeof(test4)<<','<<alignof(test4)<<std::endl; // 32, 32
}
```

example:

```cpp
#include <iostream>

struct  test1{
	char c;
	int  i;
	double d;
    int b;
};
 
struct alignas(8) test2{
	char c;
	int  i;
	double d;
    int b;
};
 
struct alignas(16) test3{
	char c;
	int  i;
	double d;
    int b; // b的后面padding 8 byte
};
 
struct alignas(32) test4{
	char c;
	int  i;
	double d;
    int b;
};

// static_assert(sizeof(test2)==16, "size of item must be 16 bytes");

int main()
{
    std::cout<<sizeof(test1)<<','<<alignof(test1)<<std::endl; // 24, 8
    std::cout<<sizeof(test2)<<','<<alignof(test2)<<std::endl; // 24, 8
    std::cout<<sizeof(test3)<<','<<alignof(test3)<<std::endl; // 32, 16
    std::cout<<sizeof(test4)<<','<<alignof(test4)<<std::endl; // 32, 32
}
```

## `static_assert`

> `static_assert(condition, message)`, as of C++17, the message is optional.

- runtime assertion: `assert`
- compile-time assertion: `static_assert`, static assertion checks is with template metaprogramming

example: `static_assert` used in a namespace

```cpp
struct alignas(8) item
{
    int      id;
    bool     active;
    double   value;
}; // 4+1+3+8=16

static_assert(sizeof(item)==16, "size of item must be 16 bytes");
```

example: `static_assert` used in a class

```cpp
#include <iostream>

template <typename T>
class pod_wrapper
{
    static_assert(std::is_floating_point_v<T>, "must be floating value!"); // since c++17
    // static_assert(std::is_integral_v <T>, "must be integral value!"); // since c++17
    T value;
};

pod_wrapper<double> p1;
// pod_wrapper<int> p2; // error


int main(){}
```

example: `static_assert` used in a block or function

```cpp
#include <iostream>

template<typename T>
auto mul(T const a, T const b)
{
    static_assert(std::is_integral_v<T>, "Integral type expected");
    return a * b;
}

int main(){}
```

## `std::enable_if`

> `enable_if`是为了constrain template, c++20引入concept之后，能够更好地constrain template

`std::enable_if`的实现

```cpp
template <bool Cond, typename Result=void>
struct enable_if { };

template <typename Result>
struct enable_if<true, Result> {
    using type = Result;
};
```

`enable_if<true, R>::type` is an alias for `R`, whereas `enable_if<false, T>::type` is ill-formed; 
> `enable_if<true>::type` is an alias for `void`
- `using type = Result;`中的`type`被当作static, 通过`::`来使用
- `using enable_if_t = typename enable_if<B,T>::type;`, 其中 `enable_if_t`是语法糖(since c++14)
- `typename=`也可以写成`class=`, `template <typename T>` 与`template <class T>`等价, [Reference](https://stackoverflow.com/questions/49408294/c-template-class-typename)

```cpp
#include <iostream>

// overload1
// 第二个typename没有template argument,只有默认值
// 第三个typename只是为了显式说明这是一个type，可以省略
template <typename F, typename= typename std::enable_if<std::is_floating_point<F>::value>::type>
// template <class F, class= typename std::enable_if<std::is_floating_point<F>::value>::type>
auto negate(F f){ return -f+1;}

// overload2
double negate(double i){return -i;}

int main()
{
    std::cout<<negate(2.2)<<std::endl; // -2.2, 如果没有overload2, 那么结果为-1.2
    std::cout<<negate<double>(2.2)<<std::endl; // -1.2
}
```

语法糖(since c++17): `inline constexpr bool is_integral_v = is_integral<T>::value;`



```cpp
// 语法糖简化
#include <iostream>

// overload1
template <typename F, typename= std::enable_if_t<std::is_floating_point_v<F>>>
auto negate(F f){ return -f+1;}

int main()
{
    std::cout<<negate(2.2)<<std::endl; // -1.2, 
    std::cout<<negate<>(2.2)<<std::endl; //-1.2
    std::cout<<negate<double>(2.2)<<std::endl; // -1.2
}
```

example

```cpp
#include <iostream>

template <typename T>
using EnableIf = typename std::enable_if_t<T::value>;

template <typename T>
using DisableIf = typename std::enable_if_t<!T::value>;

template <typename T, typename = EnableIf<std::is_integral<T>>>
auto compute(T const a, T const b){ return a+b;}

// 多加了一个typename=void, 避免redefinition
template <typename T, typename = DisableIf<std::is_integral<T>>, typename = void>
auto compute(T const a, T const b){ return a*b;}

int main()
{
    std::cout<<compute(1, 2)<<std::endl; // 3
    std::cout<<compute(1.1, 2.0)<<std::endl; // 2.2
}
```

## `constexpr`

> since c++17

> `constexpr` is for simplification of variadic templates and `std::enable_if`-based code, for **conditional compilation**

example: 改写[上文例子](#stdenable_if)

```cpp
#include <iostream>

template <typename T>
auto compute(T const a, T const b){
    if constexpr(std::is_integral_v<T>){
        return a+b;
    }else{
        return a*b;
    } 
}

int main()
{
    std::cout<<compute(1, 2)<<std::endl; // 3
    std::cout<<compute(1.1, 2.0)<<std::endl; // 2.2
}
```

example: 改写`byte8`, `byte16`, `byte32`[例子](03NumbersAndStrings.md#raw-user-defined-literals)

```cpp
// modification: struct with constexpr
// 原理: 位运算
#include <iostream>

namespace binary
{
    using byte8 = unsigned char;
    using byte16 = unsigned short;
    using byte32 = unsigned int;

    namespace binary_literals
    {
        namespace binary_literals_internals
        {
            template <typename CharT, char digit, char... bits>
            constexpr CharT binary_eval(){
                if constexpr (sizeof...(bits)==0){
                    return static_cast<CharT>(digit-'0');
                }else if constexpr (digit=='0'){
                    return binary_eval<CharT, bits...>();
                }else if constexpr (digit=='1'){
                    return static_cast<CharT>(1<< sizeof...(bits) | binary_eval<CharT, bits...>());
                }
            }
        }

        template <char... bits>
        constexpr byte8 operator""_b8()
        {
            static_assert(sizeof...(bits) <= 8, "binary literal b8 must be up to 8 digits long");
            return binary_literals_internals::binary_eval<byte8, bits...>();
        }

        template <char... bits>
        constexpr byte16 operator""_b16()
        {
            static_assert(sizeof...(bits) <= 16, "binary literal b16 must be up to 16 digits long");
            return binary_literals_internals::binary_eval<byte16, bits...>();
        }

        template <char... bits>
        constexpr byte32 operator""_b32()
        {
            static_assert(sizeof...(bits) <= 32, "binary literal b32 must be up to 32 digits long");
            return binary_literals_internals::binary_eval<byte32, bits...>();
        }
    }
}

int main()
{
    using namespace binary;
    using namespace binary_literals;

    auto b1 = 1010_b8;
    auto b2 = 101010101010_b16;
    // auto b3 = 10101010101010101010101_b32;
    std::cout << (int)b1 << std::endl; // 10
    std::cout << b2 << std::endl; // 2730
    // std::cout << b3 << std::endl;
}
```