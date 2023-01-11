# Preprocessing and Compilation

- [Preprocessing and Compilation](#preprocessing-and-compilation)
  - [Conditional Compilaton](#conditional-compilaton)
  - [stringification \& concatenation](#stringification--concatenation)
  - [`alignas`, `alignof`, `sizeof`](#alignas-alignof-sizeof)

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

## stringification & concatenation

```cpp
#include <iostream>

#define MAKE_STR2(x) #x
#define MAKE_STR(x) MAKE_STR2(x)

#define MERGE2(X, Y) X##Y
#define MERGE(X, Y) MERGE2(X, Y)
#define Jitterbug 3

#define DECL_MAKE(x) DECL_MAKE2(x)
#define DECL_MAKE2(x) x* make##_##x() { return new x(); }

struct bar {};
struct foo {};

DECL_MAKE(foo)
DECL_MAKE(bar)

int main()
{
    std::string s1=MAKE_STR2(apple);
    std::cout<<s1<<std::endl; //apple
    
    std::string s2=MAKE_STR(hello);
    std::cout<<s2<<std::endl; //hello

    auto MERGE2(variable, 2) = 10;
    std::cout<<variable2<<std::endl; //10

    auto a = MERGE(Jitter, bug);
    std::cout<<a<<std::endl; //3

    auto f = make_foo(); // f is a foo*
    auto b = make_bar(); // b is a bar*
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