# Orgnize Code

- [Orgnize Code](#orgnize-code)
  - [by Multifiles](#by-multifiles)
  - [Using CMake’s CTest](#using-cmakes-ctest)


## by Multifiles

```bash
# file tree
├── CMakeLists.txt
├── main.cpp
├── mylib.cpp
└── mylib.h
```

```h
// mylib.h
void myfunc();
```

> 为了保证headers只include一次, 使用如下

```h
// mylib.h
#ifndef MY_LIBRARY_H
#define MY_LIBRARY_H

void myfunc();

#endif // !MY_LIBRARY_H
```

也可以采用`#pragma once` 代替`#ifndef`, `#pragma once`效果更佳

```h
// mylib.h
#pragma once

void myfunc();
```

```cpp
// mylib.cpp
#include "mylib.h"
#include <iostream>

void myfunc()
{
    std::cout << "hello, myfunc!" << std::endl;
}
```

```cpp
// main.cpp
#include <iostream>
#include "mylib.h"

int main(int, char **)
{
    std::cout << "main" << std::endl;
    myfunc();
}
```

> 编译可以使用`g++ main.cpp mylib.cpp`，也可以使用`CMakeLists.txt`

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.16.0)
project(project01 VERSION 0.1.0)

add_executable(
    project01
    main.cpp
    mylib.cpp
    )
```

## Using CMake’s CTest

> 单元测试要运行函数来测试，C++需要main入口才能运行，所以单元测试本质是编译了多个main函数来进行多次测试。

> 各种测试框架([googletest](https://github.com/google/googletest), [Catch2](https://github.com/catchorg/Catch2))的本质是用宏封装了main。为了简便，也可以不使用框架，比如下面的`test0.cpp`

```bash
project1
├── CMakeLists.txt
├── include
│   ├── complex.h
│   └── simple.h
├── src
│   ├── CMakeLists.txt
│   ├── complex.cpp
│   └── simple.cpp
└── tests
    ├── catch.hpp
    ├── CMakeLists.txt
    ├── test0.cpp
    ├── test1.cpp
    └── test2.cpp
```

```cmake
# project1/CMakeLists.txt
cmake_minimum_required(VERSION 3.16.0)
project(project1)

set(CMAKE_CXX_STANDARD 17)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
include_directories(${PROJECT_SOURCE_DIR}/include)

add_subdirectory(src) # 进入src目录

enable_testing()
add_subdirectory(tests) # 进入tests目录
```

```cmake
# project1/src/CMakeLists.txt
add_library(simple simple.cpp)
add_library(complex complex.cpp)
```

[Catch2](https://github.com/catchorg/Catch2/tree/v2.x)的v2是header-only(recommended), v3是分离的

```cmake
# project1/tests/CMakeLists.txt
add_executable(test0 test0.cpp)
add_executable(test1 test1.cpp)
add_executable(test2 test2.cpp)

# 不使用框架进行单元测试
target_link_libraries(test0 simple complex)

# 使用catch2进行单元测试, catch2是header-only
# link多个library
target_link_libraries(test1 simple complex)
# target_link_libraries(test2 simple complex)

# link单个library
# target_link_libraries(test1 simple)
target_link_libraries(test2 complex)

add_test(
    NAME mytest0
    COMMAND test0
)

add_test(
    NAME mytest1
    COMMAND test1
)

add_test(
    NAME mytest2
    COMMAND test2
)
```

```h
// project1/include/complex.h
#ifndef __COMPLEX__H__
#define __COMPLEX__H__

double mul(double a, double b);
double divide(double a, double b);

#endif  //!__COMPLEX__H__
```

```cpp
// project1/src/complex.cpp
#include "complex.h"

double mul(double a, double b) {
    return a * b;
}

double divide(double a, double b) {
    return 1.0 / b * a;
}
```

```h
// project1/include/simple.h
#ifndef __SIMPLE__H__
#define __SIMPLE__H__

int add(int x, int y);
int sub(int x, int y);

#endif  //!__SIMPLE__H__
```

```cpp
// project1/src/simple.cpp
#include "simple.h"

int add(int x, int y) {
    return x + y;
}

int sub(int x, int y) {
    return x - y;
}
```

```cpp
// project1/tests/test0.cpp
#include "complex.h"
#include "simple.h"

bool test_case1() {
    return add(10, 20) == 30;
}

bool test_case2() {
    // return mul(10, 20) == 100;
    return mul(10, 20) == 200;
}

int main(int argc, char const *argv[]) {
    if (test_case1() & test_case2()) {
        return 0;
    } else {
        return -1;
    }
}
```

```cpp
// project1/tests/test1.cpp
#include "complex.h"
#include "simple.h"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

TEST_CASE("CASE1") {
    int expected = 10;
    int result = add(2, 8);
    REQUIRE(result == expected);
}

TEST_CASE("CASE2") {
    int expected = -6;
    int result = sub(2, 8);
    REQUIRE(result == expected);
}

TEST_CASE("CASE3") {
    double expected = 16;
    double result = mul(2, 8);
    REQUIRE(result == expected);
}

TEST_CASE("CASE4") {
    double expected = 4;
    double result = divide(8, 2);
    REQUIRE(result == expected);
}
```

```cpp
// project1/tests/test2.cpp
#include "complex.h"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

TEST_CASE("CASE1") {
    double expected = 20;
    double result = mul(4, 5);
    REQUIRE(result == expected);
}

TEST_CASE("CASE2") {
    double expected = 4;
    double result = divide(20, 5);
    REQUIRE(result == expected);
}
```

```cpp
// project1/tests/catch.hpp
// download from https://github.com/catchorg/Catch2/releases/download/v2.13.10/catch.hpp
```