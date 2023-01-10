# Preprocessing and Compilation

- [Preprocessing and Compilation](#preprocessing-and-compilation)
  - [Conditional Compilaton](#conditional-compilaton)
  - [stringification \& concatenation](#stringification--concatenation)

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