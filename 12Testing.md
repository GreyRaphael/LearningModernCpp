# Testing Frameworks

- [Testing Frameworks](#testing-frameworks)
  - [catch2](#catch2)
  - [Using CMake’s CTest](#using-cmakes-ctest)

## catch2

simple example by [catch2](https://github.com/catchorg/Catch2/tree/v2.x)

```bash
catch2/
    catch.hpp
main.cpp
CMakelists.txt
```

```cmake
# CMakelists.txt
cmake_minimum_required(VERSION 3.18.0)
project(proj1 VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

add_executable(proj1 main.cpp)
```

```cpp
// main.cpp
#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include <iostream>

int myadd(int x, int y) { return x + y; }
int mymul(int x, int y) { return x * y; }

TEST_CASE("case0") {
    auto result = myadd(11, 22);
    auto expected = 33;
    REQUIRE(result == expected);
}

// learn, catch is tags
TEST_CASE("case1", "[learn][catch]") {
    // several section in one case
    SECTION("func1") {
        auto result = myadd(100, 200);
        auto expected = 300;
        REQUIRE(result == expected);
    }
    SECTION("func2") {
        auto result = mymul(100, 200);
        auto expected = 20000;
        REQUIRE(result == expected);
    }
}

TEST_CASE("case2") {
    SECTION("func3") {
        auto result = myadd(100, 0);
        auto expected = 100;
        REQUIRE(result == expected);
    }
    SECTION("func4") {
        auto result = mymul(100, 0);
        auto expected = 200;
        REQUIRE(result == expected);
    }
}

TEST_CASE("case3", "[learn][catch]") {
    // nested sections
    SECTION("test_methods") {
        SECTION("func5") {
            auto result = myadd(100, 11);
            auto expected = 111;
            REQUIRE(result == expected);
        }
        SECTION("func6") {
            auto result = mymul(11, 11);
            auto expected = 120;
            REQUIRE(result == expected);
        }
    }
}
```

```bash
# run all test cases
./proj1
# run single case
./proj case2
# run a section
./proj case2 -c func4
# run a nested section
./proj case3 -c test_methods -c func6
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