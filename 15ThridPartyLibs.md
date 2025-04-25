# Third-Party Library

- [Third-Party Library](#third-party-library)
  - [Code Organized by CMake](#code-organized-by-cmake)
  - [pybind11](#pybind11)
    - [pybind11 wheels without external libs](#pybind11-wheels-without-external-libs)
    - [pybind11 wheels with external libs](#pybind11-wheels-with-external-libs)
      - [preprepared libs](#preprepared-libs)
      - [use preprepared libs in pybind11 project](#use-preprepared-libs-in-pybind11-project)
    - [pybind11 in vcpkg](#pybind11-in-vcpkg)
  - [by cython](#by-cython)
    - [cpp\_lib project](#cpp_lib-project)
    - [cython\_proj](#cython_proj)
  - [`nlohmann::json`](#nlohmannjson)
    - [`json` with struct](#json-with-struct)
    - [`json` with vector](#json-with-vector)
    - [`json` with file](#json-with-file)
    - [`json` with raw-string](#json-with-raw-string)
    - [Read/Write bson](#readwrite-bson)
    - [json deal with `NAN` \& `INFINITY`](#json-deal-with-nan--infinity)
  - [`zlib`](#zlib)
  - [`spdlog`](#spdlog)
  - [`fmt`](#fmt)
  - [`SQLite3`](#sqlite3)
  - [CLI11](#cli11)
  - [range-v3](#range-v3)
    - [zip \& zip\_with](#zip--zip_with)
  - [abseil](#abseil)
    - [abseil Swiss tables map](#abseil-swiss-tables-map)
    - [btree\_multiset](#btree_multiset)
  - [avro-cpp](#avro-cpp)
  - [`atomic_queue`](#atomic_queue)
  - [safe queue with lock](#safe-queue-with-lock)
  - [bshoshany-thread-pool](#bshoshany-thread-pool)
  - [apache arrow](#apache-arrow)
    - [compute](#compute)
    - [get string from ipc file](#get-string-from-ipc-file)
  - [nng messing library](#nng-messing-library)
    - [one producer multiple consume](#one-producer-multiple-consume)
  - [compresssion](#compresssion)
    - [zstd](#zstd)
  - [testing](#testing)
    - [gtest](#gtest)


## Code Organized by CMake

```bash
├─CMakeLists.txt
├─main.cpp
│
├─mylib1
│  │  CMakeLists.txt
│  │  myadd.cpp
│  │
│  └─include
│          myadd.h
│
└─mylib2
    │  CMakeLists.txt
    │  point3d.cpp
    │
    └─include
            point3d.h
```

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.25.0)
project(proj1 VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(mylib1)
add_subdirectory(mylib2)

add_executable(proj1 main.cpp)

target_link_libraries(proj1 PRIVATE mymath)
target_link_libraries(proj1 PRIVATE mypoint)
```

```cmake
# mylib1/CMakeLists.txt
cmake_minimum_required(VERSION 3.25.0)
project(mymath VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)

add_library(mymath SHARED myadd.cpp)
target_include_directories(mymath PUBLIC include)
```

```cmake
# mylib2/CMakeLists.txt
cmake_minimum_required(VERSION 3.25.0)
project(mypoint VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)

add_library(mypoint SHARED point3d.cpp)
target_include_directories(mypoint PUBLIC include)
```

```cpp
// main.cpp
#include <myadd.h>
#include <point3d.h>

#include <iostream>

int main() {
    std::cout << myadd(10, 20) << '\n';
    Point3D p{11, 22, 33};
    p.print();
}
```

```h
// mylib1/myadd.h
int myadd(int x, int y);
```

```cpp
// mylib1/myadd.cpp
#include "myadd.h"

int myadd(int x, int y) {
    return x + y;
}
```

```h
// mylib2/point3d.h
class Point3D {
   private:
    int x_;
    int y_;
    int z_;

   public:
    Point3D(int x, int y, int z) : x_(x), y_(y), z_(z) {}
    void print();
};
```

```cpp
// mylib2/point3d.cpp
#include <iostream>

#include "point3d.h"

void Point3D::print() {
    std::cout << '(' << x_ << ',' << y_ << ',' << z_ << ')' << '\n';
}
```

## pybind11

> [pybind11](https://github.com/pybind/pybind11) is a lightweight header-only library that exposes C++ types in Python and vice versa, mainly to create Python bindings of existing C++ code.  
> but, `Cython` is a better choice.

### pybind11 wheels without external libs

prerequisites: `pip install pybind11 wheel`

```bash
# files structure
.
├── setup.py
├── towrapper
│   ├── tools.cpp
│   └── tools.h
└── wrapped_source.cpp
```

```cpp
// tools.h
#pragma once

int myadd(int x, int y);
```

```cpp
// tools.cpp
#include "tools.h"

int myadd(int x, int y) {
    return x + y;
}
```

```cpp
// wrapped_source.cpp
#include <pybind11/pybind11.h>
#include "tools.h"

double myadd2(double x, double y) {
    return x + y;
}

// proj1 must be consistent with setup.py module name
PYBIND11_MODULE(proj1, m) {
    m.def("addi", &myadd, "integer add two integers, proj1.add(x,y)");

    m.def("addd", &myadd2, "integer add two doubles, proj1.add(x,y)");

    m.def("subi", [](int i, int j) { return i - j; }, "integer substract y from x, proj1.sub(x,y)");

    m.def("subd", [](double i, double j) { return i - j; }, "double substract y from x, proj1.sub(x,y)");
}
```

```py
# setup.py
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
import pybind11

ext_modules = [
    Extension(
        name="proj1",
        sources=["wrapped_source.cpp", "towrapper/tools.cpp"],
        include_dirs=[
            pybind11.get_include(),  # Path to pybind11 headers
            "towrapper",  # Any other include paths
        ],
        language="c++",
        # extra_compile_args=["-std=c++17"], # depends on compiler
        # py_limited_api=True, # not support in pybind11, but in cython
        # define_macros=[("Py_LIMITED_API", "0x03090000")], # not support in pybind11, but in cython
    ),
]

setup(
    name="proj1",
    version="1.0.0",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
)
```

> `python setup bdist_wheel`

```bash
# output
.
├── build
│   ├── bdist.linux-x86_64
│   ├── lib.linux-x86_64-cpython-310
│   │   └── proj1.cpython-310-x86_64-linux-gnu.so # maybe usable
│   └── temp.linux-x86_64-cpython-310
│       ├── towrapper
│       │   └── tools.o
│       └── wrapped_source.o
├── dist
│   └── proj1-1.0.0-cp310-cp310-linux_x86_64.whl # target file
├── proj1.egg-info
│   ├── PKG-INFO
│   ├── SOURCES.txt
│   ├── dependency_links.txt
│   ├── not-zip-safe
│   └── top_level.txt
├── setup.py
├── towrapper
│   ├── tools.cpp
│   └── tools.h
└── wrapped_source.cpp
```

### pybind11 wheels with external libs

prerequisites: `pip install pybind11 wheel`

#### preprepared libs

```bash
.
├── CMakeLists.txt
├── xxx.cpp
├── xxx.h
├── yyy.cpp
└── yyy.h
```

```cmake
cmake_minimum_required(VERSION 3.28.0)
project(xxxyyy VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_POSITION_INDEPENDENT_CODE ON) # must be set for pybind11
add_library(xxx STATIC xxx.cpp)
add_library(yyy SHARED yyy.cpp)
```

```cpp
// xxx.h
#pragma once
#include <cstdio>
#include <string>

int mymul(int a, int b);

template <typename T>
struct Student {
    T value;
    std::string name;
    void say_hello() {
        printf("hello, I am %s", name.c_str());
    }
    T calc(T x, T y) {
        return x + y + value;
    }
};

auto do_somthing(Student<double>& stu, double value);

double wrapped_do_somthing(double value);
```

```cpp
// xxx.cpp
#include "xxx.h"

#include <cstdio>

int mymul(int a, int b) {
    return a * b;
}

auto do_somthing(Student<double> &stu, double value) {
    stu.value = value;
    return stu.calc(100.1, 200.2);
}

double wrapped_do_somthing(double value) {
    Student<double> stu{.name = "gewei"};
    auto result = do_somthing(stu, value);
    printf("name=%s, result=%f", stu.name.c_str(), result);
    return result;
}
```

```cpp
// yyy.h
#pragma once

double mydiv(double a, double b);
```

```cpp
// yyy.cpp
#include "yyy.h"

double mydiv(double a, double b) {
    return a / b;
}
```

```bash
# build release output
build/
    libxxx.a    # recommended
    libyyy.so
```

#### use preprepared libs in pybind11 project

```bash
.
├── setup.py
├── shared_lib1
│   ├── libyyy.so
│   └── yyy.h
├── static_lib2
│   ├── libxxx.a
│   └── xxx.h
└── wrapped_source.cpp
```

```cpp
// simplified xxx.h
int mymul(int a, int b);
double wrapped_do_somthing(double value);
```

```cpp
// yyy.h
double mydiv(double a, double b);
```

```cpp
// wrapped_source.cpp
#include <pybind11/pybind11.h>

#include "xxx.h"
#include "yyy.h"

// proj1 must be consistent with setup.py module name
PYBIND11_MODULE(proj1, m) {
    m.def("muli", &mymul, "multiply add two integers, proj1.muli(x,y)");
    m.def("do_somthing", &wrapped_do_somthing, "custom do_something, proj1.do_somthing(value)");

    m.def("divd", &mydiv, "divide two doubles, proj1.divd(x, y)");

    m.def("subi", [](int i, int j) { return i - j; }, "integer substract y from x, proj1.sub(x,y)");
    m.def("subd", [](double i, double j) { return i - j; }, "double substract y from x, proj1.sub(x,y)");
}
```

```py
# setup.py
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
import pybind11

ext_modules = [
    Extension(
        name="proj1",
        sources=["wrapped_source.cpp"],
        include_dirs=[
            pybind11.get_include(),  # Path to pybind11 headers
            "shared_lib1",  # Any other include paths
            "static_lib2",  # Any other include paths
        ],
        library_dirs=[
            "shared_lib1",  # Any other library paths
            "static_lib2",  # Any other library paths
        ],
        libraries=[
            "xxx",
            "yyy",
        ],
        language="c++",
        extra_link_args=["-Wl,-rpath=./"],
    ),
]

setup(
    name="proj1",
    version="1.0.0",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
)
```

> `python setup.py bdist_wheel`

```bash
.
├── build
│   ├── bdist.linux-x86_64
│   ├── lib.linux-x86_64-cpython-310
│   │   └── proj1.cpython-310-x86_64-linux-gnu.so # # maybe usable
│   └── temp.linux-x86_64-cpython-310
│       └── wrapped_source.o
├── dist
│   └── proj1-1.0.0-cp310-cp310-linux_x86_64.whl # target whl file
├── proj1.egg-info
│   ├── PKG-INFO
│   ├── SOURCES.txt
│   ├── dependency_links.txt
│   ├── not-zip-safe
│   └── top_level.txt
├── setup.py
├── shared_lib1
│   ├── libyyy.so
│   └── yyy.h
├── static_lib2
│   ├── libxxx.a
│   └── xxx.h
└── wrapped_source.cpp
```

### pybind11 in vcpkg

skip python3 dependencies from vcpkg for pybind11, but use system python3

```bash
myproject/
├─ overlays/
│  └─ python3/
│     ├ vcpkg.json
│     └ portfile.cmake
└─ CMakeLists.txt
```

```json
// vcpkg.json
{
  "name": "python3",
  "version": "3.0.0",
  "port-version": 0
}
```

```cmake
# portfile.cmake
# Tell vcpkg this is an “empty package”—nothing to build or install
set(VCPKG_POLICY_EMPTY_PACKAGE enabled)
```

```bash
# If you’re driving everything through CMake:
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_OVERLAY_PORTS=${PWD}/overlays

# Or if you’re manually invoking vcpkg:
vcpkg install pybind11 --overlay-ports=${PWD}/overlays
```

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.20.0)

# must come *before* project()
set(CMAKE_TOOLCHAIN_FILE
    "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING
    "Vcpkg toolchain file")
set(VCPKG_OVERLAY_PORTS ${CMAKE_SOURCE_DIR}/overlays CACHE STRING "Vcpkg skipped ports")

project(proj1 VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
add_executable(proj1 main.cpp)
target_include_directories(proj1 PRIVATE ctp)

find_package(boost_dll CONFIG REQUIRED)
target_link_libraries(proj1 PRIVATE Boost::dll)

find_package(Python COMPONENTS Interpreter Development)
find_package(pybind11 CONFIG)
```

## by cython

```bash
# tree
.
├── cpp_lib
│   ├── CMakeLists.txt
│   ├── build
│   │   ├── libxxx.a
│   │   └── libyyy.so
│   ├── build_script.sh
│   ├── xxx.cpp
│   ├── xxx.h
│   ├── yyy.cpp
│   └── yyy.h
└── cython_proj
    ├── libs
    │   ├── libxxx.a -> ../../cpp_lib/build/libxxx.a
    │   ├── libyyy.so -> ../../cpp_lib/build/libyyy.so
    │   ├── sim_xxx.h
    │   └── sim_yyy.h
    ├── setup.py
    └── wrapper.pyx
```

### cpp_lib project

> `source build_script.sh`

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.28.0)
project(xxxyyy VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_POSITION_INDEPENDENT_CODE ON) # must exist
add_library(xxx STATIC xxx.cpp)
add_library(yyy SHARED yyy.cpp)
```

```bash
# build_script.sh
cmake -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_TOOLCHAIN_FILE:STRING=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_MAKE_PROGRAM:STRING=ninja -Bbuild -G Ninja
cmake --build build --config Release --target all --
```

```cpp
// xxx.h
#pragma once
#include <cstdio>
#include <string>

int mymul(int a, int b);

template <typename T>
struct Student {
    T value;
    std::string name;
    void say_hello() {
        printf("hello, I am %s\n", name.c_str());
    }
    T calc(T x, T y) {
        return x + y + value;
    }
};

auto do_somthing(Student<double>& stu, double value);

double wrapped_do_somthing(double value);
```

```cpp
// xxx.cpp
#include "xxx.h"

#include <cstdio>

int mymul(int a, int b) {
    return a * b;
}

auto do_somthing(Student<double> &stu, double value) {
    stu.value = value;
    return stu.calc(100.1, 200.2);
}

double wrapped_do_somthing(double value) {
    Student<double> stu{.name = "gewei"};
    auto result = do_somthing(stu, value);
    printf("name=%s, result=%f\n", stu.name.c_str(), result);
    return result;
}
```

```cpp
// yyy.h
#pragma once
double mydiv(double a, double b);
```

```cpp
// yyy.cpp
#include "yyy.h"

double mydiv(double a, double b) {
    return a / b;
}
```

### cython_proj

in windows: `python setup.py bdist_wheel --py-limited-api=cp37`
> Require: `pip install cython wheel`

in linux
> Require: `pip install cython wheel auditwheel patchelf`
- `python setup.py bdist_wheel --py-limited-api=cp37`
- `auditwheel show dist/*.whl`: check manylinux platform
- `auditwheel repair dist/*.whl --plat manylinux_2_24_x86_64`

```cpp
// sim_xxx.h
int mymul(int a, int b);
double wrapped_do_somthing(double value);
```

```cpp
// sim_yyy.h
double mydiv(double a, double b);
```

```py
# cython: language_level=3

cdef extern from "sim_xxx.h":
    int mymul(int a, int b)
    double wrapped_do_somthing(double value)

cdef extern from "sim_yyy.h":
    double mydiv(double a, double b)

def py_mul(int x, int y):
    return mymul(x, y)

def py_div(double x, double y):
    return mydiv(x, y)

def py_do(double x):
    return wrapped_do_somthing(x)
```

```py
# setup.py
from setuptools import setup, Extension
from Cython.Build import cythonize
import shutil
import os

build_lib_dir = "build_lib"
os.makedirs(build_lib_dir, exist_ok=True)
shutil.copy("libs/libyyy.so", f"{build_lib_dir}/libyyy.so")

extensions = [
    Extension(
        name="proj1",
        sources=["wrapper.pyx"],
        include_dirs=["libs"],
        library_dirs=["libs"],
        libraries=["xxx"],
        runtime_library_dirs=["$ORIGIN"],
        extra_link_args=["-lyyy", "-Wl,-rpath,$ORIGIN", f"-o{build_lib_dir}/proj1.so"],
        language="c++",
        # support python3.7 and above
        py_limited_api=True,
        define_macros=[("CYTHON_LIMITED_API", "0x03070000"), ("Py_LIMITED_API", "0x03070000")],
    )
]


setup(
    name="proj1",
    version="1.0.0",
    ext_modules=cythonize(extensions),
    options={"build": {"build_lib": build_lib_dir}},
)
```

## `nlohmann::json`

> [nlohmann::json](https://github.com/nlohmann/json)

simple example

```bash
include
    └──nlohmann
        └── json.hpp
main.cpp
CMakeLists.txt
```

```cmake
cmake_minimum_required(VERSION 3.25.0)
project(proj1 VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)

add_executable(proj1 main.cpp)
target_include_directories(proj1 PRIVATE include)
```

```cpp
// main.cpp
#include <iostream>
#include <nlohmann/json.hpp>

class Foo {
   private:
    int id_ = 0;
    double score_ = 0;

   public:
    Foo() = default;
    nlohmann::json to_json() {
        nlohmann::json j;
        j["id"] = id_;
        j["score"] = score_;
        return j;
    }
    void from_json(nlohmann::json const& j) {
        id_ = j["id"];
        score_ = j["score"];
    }
    void print_info() {
        std::cout << "id=" << id_ << ", score=" << score_ << '\n';
    }
};

int main() {
    Foo f;
    f.print_info();  // id=0, score=0
    f.from_json({{"id", 10}, {"score", 90.5}});
    f.print_info();  // id=10, score=90.5
    auto j = f.to_json();
    std::cout << j << '\n';  // {"id":10,"score":90.5}
}
```

### `json` with struct

```cpp
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

struct Person {
    int age;
    double height;
    char symbols[3];
};

struct Student {
    std::string name;
    int scores[3];
};

void to_json(json& j, const Person& p) {
    j = json{{"age", p.age}, {"height", p.height}, {"symbols", p.symbols}};
}

void from_json(const json& j, Person& p) {
    j.at("age").get_to(p.age);
    j.at("height").get_to(p.height);
    j.at("symbols").get_to(p.symbols);
}

void to_json(json& j, const Student& s) {
    j = json{{"name", s.name}, {"scores", s.scores}};
}

void from_json(const json& j, Student& s) {
    j.at("name").get_to(s.name);
    j.at("scores").get_to(s.scores);
}

int main() {
    // Serialize Person
    Person person{25, 175.5, {'a', 'b', 'c'}};
    json personJson = person;
    std::cout << "Person JSON:\n"<< personJson.dump(2) << std::endl;

    // Serialize Student
    Student student{"Alice", {90, 85, 95}};
    json studentJson = student;
    std::cout << "Student JSON:\n"<< studentJson.dump(2) << std::endl;
}
```

### `json` with vector

```cpp
#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>

int main() {
    const int N = 5;
    std::vector<nlohmann::json> jsons(N);
    for (size_t i = 0; i < N; ++i) {
        jsons[i]["id"] = 10 + i;
    }
    nlohmann::json root_json;
    root_json["jsons"] = jsons;

    // to_json
    std::cout << root_json << '\n';  // {"jsons":[{"id":10},{"id":11},{"id":12},{"id":13},{"id":14}]}

    // parse json
    std::string json_str = R"({"jsons":[{"id":20},{"id":21},{"id":22},{"id":23},{"id":24}]})";
    auto j = nlohmann::json::parse(json_str);
    // for (auto& e : j["jsons"]) {
    //     std::cout << e["id"] << ',';
    // } // 20,21,22,23,24,
    for (size_t i = 0; i < N; ++i) {
        std::cout << j["jsons"][i]["id"] << ',';
    } // 20,21,22,23,24,
}
```

example: from/to json for vector

```cpp
#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>

class Operator {
   public:
    Operator(int val) : _vsum(val + 100) {}
    void to_json(nlohmann::json& state) {
        state["_vsum"] = _vsum;
    }
    void from_json(const nlohmann::json& state) {
        _vsum = state["_vsum"];
    }
    void print() { std::cout << _vsum << '\n'; }

   private:
    int _vsum = 0;
};

class Factor1 {
   public:
    Factor1() {
        for (int i = 0; i < PRICE_COUNT_; i++) {
            auto op = Operator(i);
            _rolling_ask_prices.push_back(op);
        }
    }
    void to_json(nlohmann::json& state);
    void from_json(const nlohmann::json& state);

    std::vector<Operator> _rolling_ask_prices;

   private:
    int const PRICE_COUNT_ = 5;
};

void Factor1::to_json(nlohmann::json& state) {
    std::vector<nlohmann::json> j1s(PRICE_COUNT_);
    for (size_t i = 0; i < PRICE_COUNT_; ++i) {
        _rolling_ask_prices[i].to_json(j1s[i]);
    }
    state["_rolling_ask_prices"] = j1s;
}

void Factor1::from_json(const nlohmann::json& state) {
    auto j1s = state["_rolling_ask_prices"];
    for (size_t i = 0; i < PRICE_COUNT_; ++i) {
        _rolling_ask_prices[i].from_json(j1s[i]);
    }
}

int main() {
    {
        nlohmann::json state;
        Factor1 f1;
        f1.to_json(state);
        std::cout << state.dump() << '\n';  //{"_rolling_ask_prices":[{"_vsum":100},{"_vsum":101},{"_vsum":102},{"_vsum":103},{"_vsum":104}]}
    }
    {
        std::string json_str = R"({"_rolling_ask_prices":[{"_vsum":200},{"_vsum":201},{"_vsum":202},{"_vsum":203},{"_vsum":204}]})";
        nlohmann::json state = nlohmann::json::parse(json_str);
        Factor1 f1;
        f1.from_json(state);
        for (auto& op : f1._rolling_ask_prices) {
            op.print();
        }
    }
}
```

### `json` with file

example: `load` from json file & `dump` to json file

```json
// example.json
{
    "name":"grey",
    "年龄":23,
    "scores":[60, 95, 32]
}
```

```json
// output.json
{"id":100010,"name":"grey","scores":[60,95,32],"年龄":23}
```

```cpp
// main.cpp
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

int main() {
    // load from file
    std::ifstream fin{"example.json"};
    auto j = nlohmann::json::parse(fin); // nlohmann::json

    std::cout << j << '\n';                           // {"age":23,"name":"grey","scores":[60,95,32]}
    std::cout << j["scores"] << '\n';                 // [60,95,32]
    std::cout << typeid(j["scores"]).name() << '\n';  // nlohmann allocator

    // dump to file
    j["id"] = 100010;
    std::ofstream fout{"output.json"};
    fout << j;
}
```

### `json` with raw-string

example: json `load` from string & `dump` to string

```cpp
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <vector>

int main() {
    // generate data
    std::ifstream fin{"example.json"};
    std::ostringstream oss;
    oss << fin.rdbuf();

    // load from string
    {
        std::string raw_str1 = oss.str();
        std::cout << raw_str1 << '\n';
        // load from raw string, 无需考虑\"
        auto j1 = nlohmann::json::parse(raw_str1);
        std::cout << j1 << '\n';

        // load from string literal
        std::string raw_str2 = R"({"name":"Tim","scores":[60,95,32],"年龄":54})";
        auto j2 = nlohmann::json::parse(raw_str2);
        std::cout << j2 << '\n';

        // load from string literal, stupid
        std::string raw_str3 = "{\"name\":\"Jerry\",\"scores\":[60,95,32],\"年龄\":66}";
        auto j3 = nlohmann::json::parse(raw_str3);
        std::cout << j3 << '\n';
    }

    // generate data
    std::vector<int> v{11, 22, 33};
    nlohmann::json j{
        {"name", "James"},
        {"年龄", 43},
        {"scores", v}

    };
    // dump to string
    {
        std::string s1 = j.dump();
        std::cout << s1 << '\n';

        // string to file
        std::ofstream fout{"output.json"};
        fout << s1;
    }
}
```

### Read/Write bson

[bson](https://json.nlohmann.me/api/basic_json/to_bson): Binary  Json

```cpp
#include <cmath>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main() {
    // Create a JSON object
    json jsonData = {
        {"name", "Tom"},
        {"age", 23},
        // {"field", NAN}, // NOT support NAN, INFINITY
        {"city", 12.3}};

    // write bson
    {
        // Convert JSON to BSON
        std::vector<uint8_t> bsonData = json::to_bson(jsonData);

        // Write BSON data to a file
        std::ofstream outFile("data.bson", std::ios::binary);
        if (!outFile) {
            std::cerr << "Failed to open file for writing." << std::endl;
            return 1;
        }
        outFile.write(reinterpret_cast<const char*>(bsonData.data()), bsonData.size());
        outFile.close();
    }
    // read bson
    {
        // Read BSON data from the file
        std::ifstream inFile("data.bson", std::ios::binary);
        if (!inFile) {
            std::cerr << "Failed to open file for reading." << std::endl;
            return 1;
        }
        std::vector<char> buffer((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
        inFile.close();

        // Convert BSON to JSON
        json decodedData = json::from_bson(buffer);

        // Print the decoded JSON data
        std::cout << decodedData.dump(4) << std::endl;
    }
}
```

### json deal with `NAN` & `INFINITY`

json not support `NAN` & `INFINITY`
> `NAN` & `INFINITY` are dumped as `null` in `nlohmann::json`

```cpp
#include <cmath>
#include <iostream>
#include <nlohmann/json.hpp>

int main() {
    nlohmann::json j{
        {"key1", NAN},
        {"key2", INFINITY},
        {"key3", 3.1415926}};
    std::cout << j << '\n'; // {"key1":null,"key2":null,"key3":3.1415926}
    std::string j_str = j.dump();

    auto j2 = nlohmann::json::parse(j_str);
    std::cout << j2 << '\n'; // // {"key1":null,"key2":null,"key3":3.1415926}
}
```

Solution for parse, visit [solutions](examples/ch15-json-nan-inf.cc):
1. single double value: **double -> bytes -> double**
2. double in a container: **double container -> bytes container -> double container**

## `zlib`

`vcpkg install zlib`

```bash
├─main.cpp
├─CMakeLists.txt
```

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.28.0)
project(proj1 VERSION 0.1.0 LANGUAGES C CXX)

add_executable(proj1 main.cpp)

find_package(ZLIB REQUIRED)
target_link_libraries(proj1 PRIVATE ZLIB::ZLIB)
```

```cpp
//main.cpp
#include <zlib.h>

#include <fstream>
#include <iostream>
#include <sstream>


std::string readGzFile(const std::string& filename) {
    gzFile file = gzopen(filename.c_str(), "rb");
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return "";
    }

    std::string result;
    char buffer[2048];
    int bytesRead = 0;

    while ((bytesRead = gzread(file, buffer, sizeof(buffer))) > 0) {
        result.append(buffer, bytesRead);
    }

    gzclose(file);

    return result;
}

void writeGzFile(const std::string& filename, const std::string& data) {
    gzFile file = gzopen(filename.c_str(), "wb");
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    gzwrite(file, data.c_str(), data.length());

    gzclose(file);
}

int main() {
    std::string source_filename{"data.json"};
    std::string gz_filename{"test.json.gz"};
    // generate string data
    std::ifstream fin{source_filename};
    std::ostringstream oss;
    oss << fin.rdbuf();
    std::string data = oss.str();

    {
        // write gz file
        writeGzFile(gz_filename, data);
    }
    {
        // read gz file
        std::string data = readGzFile(gz_filename);
        std::cout << data << '\n';
    }
}
```

## `spdlog`

> [spdlog](https://github.com/gabime/spdlog): Very fast, header-only/compiled, C++ logging library. ci

```cpp
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

int main(){
    // config logger
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("log/custom.log", true);  // true: delete previous log.txt

    spdlog::logger logger("mylogger", {console_sink, file_sink});
    spdlog::set_default_logger(std::make_shared<spdlog::logger>(logger));  // SPDLOG_XXX is default logger
    spdlog::set_pattern("[%^%4!l%$] [%T] [%s:%#:%!] %v");

    // use spdlog
    SPDLOG_INFO("test info");
    SPDLOG_WARN("test warning");
    SPDLOG_ERROR("test error");
}
```

## `fmt`

[fmt](https://github.com/fmtlib/fmt) is an open-source formatting library providing a fast and safe alternative to C stdio and C++ iostreams.
> [fmt cheat sheet](https://hackingcpp.com/cpp/libs/fmt.html)

## `SQLite3`

Use `sqlite3` in C++
1. download source code of [sqlite](https://www.sqlite.org/download.html), e.g. `sqlite-amalgamation-3430100.zip`
2. extract `sqlite3.h` & `sqlite3.c` from `sqlite-amalgamation-3430100.zip` file to your dependency folder
3. create `CMakeLists.txt`
4. build with your project

example: C++ Read/Write SQLite3

```bash
_deps/
    sqlite-3.43.1/
        sqlite3.c
        sqlite3.h
        CMakeLists.txt
main.cpp
CMakeLists.txt
```

```cmake
# _deps/sqlite-3.43.1/CMakeLists.txt
# add_library(sqlite3 sqlite3.c) # static lib
add_library(sqlite3 SHARED sqlite3.c) # shared lib
```


```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.25.0)
project(proj1 VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(_deps/sqlite-3.43.1)

add_executable(proj1 main.cpp)
target_include_directories(proj1 PRIVATE _deps/sqlite-3.43.1)
target_link_libraries(proj1 PRIVATE sqlite3)
```

```cpp
#include <sqlite3.h>

#include <cmath>   // std::round
#include <format>  // std::format
#include <iostream>
#include <tuple>
#include <vector>

void writeDb(sqlite3 *db, std::string const &sql) {
    std::cout << "Writing to database: " << sql << '\n';
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        // Error handling if executing the INSERT statement fails
        std::cout << "insert error: " << sqlite3_errmsg(db) << '\n';
    }
}

std::vector<std::tuple<std::string, int, double>> readDb(sqlite3 *db, std::string const &sql) {
    std::cout << "Reading from database: " << sql << '\n';
    std::vector<std::tuple<std::string, int, double>> results;
    int rc = sqlite3_exec(
        db, sql.c_str(), [](void *data, int argc, char **argv, char **colNames) {
            // result->data->vec
            auto vec = static_cast<std::vector<std::tuple<std::string, int, double>> *>(data);
            vec->push_back(std::make_tuple(std::string{argv[0]}, std::stoi(argv[1]), std::stod(argv[2])));
            return 0;
        },
        &results, nullptr);
    if (rc != SQLITE_OK) {
        std::cout << "SQL error: " << sqlite3_errmsg(db) << '\n';
    }
    return results;
}

int main() {
    sqlite3 *db;
    int rc = sqlite3_open("your_database.db", &db);
    if (rc) {
        // Error handling if opening the database fails
        std::cout << "cannot open database" << sqlite3_errmsg(db) << '\n';
        return rc;
    }

    auto createTableSql = "CREATE TABLE IF NOT EXISTS your_table (column1 TEXT, column2 INT, column3 REAL);";
    rc = sqlite3_exec(db, createTableSql, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        // Error handling if executing the CREATE TABLE statement fails
        std::cout << "cannot create table" << sqlite3_errmsg(db) << '\n';
        sqlite3_close(db);
        return rc;
    }
    // write date to sqlite3
    {
        for (size_t i = 0; i < 10; ++i) {
            auto name = std::format("stu-{}", i);
            auto score = std::round(1.1 * i * 100) / 100;  // .2f
            auto insertSql = std::format("INSERT INTO your_table (column1, column2, column3) VALUES ('{}', {}, {});", name, i, score);
            writeDb(db, insertSql);
        }
    }
    // read from sqlite3
    {
        auto results = readDb(db, "SELECT * FROM your_table");
        for (auto &row : results) {
            auto [name, id, score] = row;
            std::cout << name << " " << id << " " << score << '\n';
        }
    }
    sqlite3_close(db);
}
```

## CLI11

[CLI11](https://github.com/CLIUtils/CLI11): `vcpkg install cli11`

```cpp
#include <fmt/core.h>
#include <CLI/CLI.hpp>
#include <string>

int main(int argc, char** argv) {
    CLI::App app("future trade client");

    // order
    CLI::App* subcommand1 = app.add_subcommand("order", "send orders");
    int side = 0;
    subcommand1->add_option("-s,--side", side, "trading direction");
    int volume = 100;
    subcommand1->add_option("-v,--vol", volume, "trading volume");
    std::string stock_info{"input/stock.csv"};
    subcommand1->add_option("-i,--in", stock_info, "stock price info");
    std::string client_info{"clients/gewei.json"};
    subcommand1->add_option("-u,--u", client_info, "account info");
    int order_type = 1;
    subcommand1->add_option("-t,--type", side, "trading direction");

    // cancel
    CLI::App* subcommand2 = app.add_subcommand("cancel", "cancel orders");

    // query
    CLI::App* subcommand3 = app.add_subcommand("query", "query");

    app.parse(argc, argv);

    if (app.got_subcommand("order")) {
        fmt::println("send orders");
        fmt::println("input {}: {}, {}, {}, {}, {}", app.count_all(), side, volume, stock_info, client_info, order_type);
    } else if (app.got_subcommand("cancel")) {
        fmt::println("cancel orders");
    } else if (app.got_subcommand("query")) {
        fmt::println("query orders");
    } else {
    }
}
```

## range-v3

`vcpkg install range-v3`

### zip & zip_with

```cpp
#include <format>
#include <iostream>
#include <map>
#include <range/v3/view.hpp>
#include <vector>

int main() {
    std::vector a{10, 20, 30, 40, 50};
    std::vector<std::string> b{"one", "two", "three", "four"};
    std::map<std::string, int> c{{"one", 1}, {"two", 2}, {"three", 3}, {"four", 4}};

    for (const auto& [num, name, pair] : ranges::views::zip(a, b, c))
        std::cout << std::format("{} -> {} -> {}\n", num, name, pair.second);
}
```

```cpp
#include <iostream>
#include <range/v3/all.hpp>
#include <vector>

int main() {
    std::vector prices = {100, 200, 150, 180, 130};
    std::vector costs = {10, 20, 50, 40, 100};

    auto compute = [](const auto& p, const auto& c) { return p - c; };
    auto income = ranges::views::zip_with(compute, prices, costs);

    std::cout << ranges::accumulate(income, 0);              // 540
    std::cout << ranges::fold_left(income, 0, std::plus{});  // 540
    auto vec_income = ranges::to<std::vector>(income);       // convert to vector
}
```

## abseil

`vcpkg install abseil`

```cmake
find_package(absl CONFIG REQUIRED)
# # note: 181 additional targets are not displayed.
target_link_libraries(proj1 PRIVATE absl::strings absl::str_format absl::time)
```

```cpp
#include <absl/time/civil_time.h>
#include <absl/time/clock.h>
#include <absl/time/time.h>

#include <iostream>

int main() {
    absl::Time currentTime = absl::Now();
    std::cout << currentTime << '\n';  // 2024-04-16T02:02:18.5981171+08:00

    auto cs = absl::ToCivilSecond(currentTime, absl::LocalTimeZone());
    std::cout << cs.year() << '\n';
    std::cout << cs.month() << '\n';
    std::cout << cs.day() << '\n';
    std::cout << cs.hour() << '\n';
    std::cout << cs.minute() << '\n';
    std::cout << cs.second() << '\n';
    auto v = absl::GetCurrentTimeNanos();
    auto civil_time = absl::FromCivil(cs, absl::LocalTimeZone());
    auto dur = currentTime - civil_time;
    std::cout << dur / absl::Milliseconds(1) << '\n';  // 598

    auto lat = absl::LocalTimeZone();
    auto info = lat.At(currentTime);
    std::cout << info.zone_abbr << '\n';  // CST
    std::cout << info.cs << '\n';         // 2024-04-16T02:02:18

    auto wd = absl::GetWeekday(info.cs);
    std::cout << wd << '\n';       // Tuesday
    std::cout << int(wd) << '\n';  // 1
}
```

### abseil Swiss tables map

`vcpkg install abseil`

[abseil container](https://abseil.io/docs/cpp/guides/container) better than `std::map` & `std::unordered_map`

```cmake
cmake_minimum_required(VERSION 3.20.0)
project(prj1 VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
add_executable(prj1 main.cpp)

# this is heuristically generated, and may not be correct
find_package(absl CONFIG REQUIRED)
# note: 180 additional targets are not displayed.
target_link_libraries(prj1 PRIVATE absl::node_hash_map)
```

> not use `target_link_libraries(prj1 PRIVATE absl::absl_node_hash_map)`

```cpp
#include <absl/container/node_hash_map.h>

#include <BS_thread_pool.hpp>
#include <cmath>
#include <cstdlib>
#include <format>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

struct TickData {
    std::string secucode;
    double total_ask_volume;
    double total_bid_volume;
};

template <typename T>
concept HqData = std::is_constructible_v<TickData>;

template <typename T>
concept NumericVal = std::is_floating_point_v<T> || std::is_integral_v<T>;

template <HqData T, NumericVal U>
struct Factor {
    std::string name;
    U feature;
    virtual U update_feature(T const& hq) = 0;
};

struct Factor01 : Factor<TickData, double> {
    double update_feature(TickData const& tick) override {
        std::cout << std::format("invoke Factor01\n");
        return tick.total_bid_volume;
    }
};

struct Factor02 : Factor<TickData, double> {
    double update_feature(TickData const& tick) {
        std::cout << std::format("invoke Factor02\n");
        return tick.total_ask_volume;
    }
};

struct Factor03 : Factor<TickData, double> {
    double update_feature(TickData const& tick) {
        std::cout << std::format("invoke Factor03\n");
        return tick.total_ask_volume / tick.total_bid_volume;
    }
};

using FactorData = Factor<TickData, double>;
using FactorResults = absl::node_hash_map<std::string, std::vector<double>>;

int main() {
    BS::thread_pool pool;
    TickData tick{
        .secucode = "000001",
        .total_ask_volume = 233,
        .total_bid_volume = 166,
    };

    constexpr int factor_num = 3;
    std::vector<std::shared_ptr<FactorData>> factor_ptrs;
    factor_ptrs.reserve(factor_num);
    factor_ptrs.emplace_back(std::make_shared<Factor01>());
    factor_ptrs.emplace_back(std::make_shared<Factor02>());
    factor_ptrs.emplace_back(std::make_shared<Factor03>());

    auto futures = pool.submit_sequence(0, factor_num, [&factor_ptrs, &tick](int i) {
        return factor_ptrs[i]->update_feature(tick);
    });

    FactorResults results;
    results[tick.secucode] = futures.get();
    std::cout << results["000001"][0] << '\n';
}
```

### btree_multiset

```cpp
#include <iostream>

#include "absl/container/btree_set.h"  // Include the B-tree multiset header

struct MyData {
    int id;
    double value;

    // Implement the correct comparator as operator<
    bool operator<(const MyData& other) const {
        if (id != other.id) {
            return id < other.id;
        }
        return value > other.value;
    }
};

int main() {
    // Create a multiset with the default comparator, which is operator<
    absl::btree_multiset<MyData> mySet;

    // Insert some elements
    mySet.insert({1, 100.0});
    mySet.insert({1, 200.0});
    mySet.insert({2, 300.0});
    mySet.insert({2, 150.0});

    // Output the elements in the multiset
    for (const auto& element : mySet) {
        std::cout << "ID: " << element.id << ", Value: " << element.value << std::endl;
    }
}
```

## avro-cpp

```bash
# 1. change ~/vcpkg/ports/avro-cpp/portfile.cmake
vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}/lang/c++"
    OPTIONS
        -DBUILD_TESTING=OFF
        # add this line
        -DCMAKE_CXX_STANDARD=20
        ${FEATURE_OPTIONS}
)
# 2. change ~/vcpkg/triplets/x64-linux.cmake
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic) # static to dynamic

set(VCPKG_CMAKE_SYSTEM_NAME Linux)

# 3. install avro-cpp
vcpkg install avro-cpp
```

read avro file with `re` and `im` field

```cpp
#include <avro/DataFile.hh>
#include <avro/Generic.hh>
#include <format>
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "provide an avro file!" << '\n';
        return -1;
    }

    avro::DataFileReader<avro::GenericDatum> reader(argv[1]);
    auto dataSchema = reader.dataSchema();

    avro::GenericDatum datum(dataSchema);
    while (reader.read(datum)) {
        if (datum.type() == avro::AVRO_RECORD) {
            const avro::GenericRecord& r = datum.value<avro::GenericRecord>();
            // TODO: pull out each field
            auto x1 = r.field("re").value<double>();
            auto x2 = r.field("im").value<double>();
            std::cout << std::format("{} + {}i\n", x1, x2);
        }
    }
    std::cout << dataSchema.toJson() << '\n';
}
```

## `atomic_queue`

[atomic_queue](https://github.com/max0x7ba/atomic_queue): C++14 multiple-producer-multiple-consumer lock-free queues based on circular buffer and std::atomic. Designed with a goal to minimize the latency between one thread pushing an element into a queue and another thread popping it from the queue.

[benchmark](https://max0x7ba.github.io/atomic_queue/) with other implementations
- `boost::lockfree::queue`
- `boost::lockfree::spsc_queue`
- `moodycamel::ConcurrentQueue`
- `moodycamel::ReaderWriterQueue`

> `vcpkg install atomic-queue`

```cpp
#include <format>
#include <iostream>
#include <optional>
#include <thread>
#include <vector>

#include "atomic_queue/atomic_queue.h"

template <typename T>
concept IsOptional = std::is_same_v<T, std::optional<typename T::value_type>>;

template <typename T>
using ThreadSafeQueue = atomic_queue::AtomicQueue2<T, 100>;

template <IsOptional T>
void worker_function(int idx, ThreadSafeQueue<T>& input_queue, ThreadSafeQueue<T>& output_queue) {
    while (true) {
        T item = input_queue.pop();
        if (!item) break;
        std::cout << std::format("thread-{} processing {}\n", idx, *item);
        output_queue.push(*item * 100000);
    }
    std::cout << std::format("thread-{} stop\n", idx);
}

int main() {
    const int num_threads = std::thread::hardware_concurrency();

    std::vector<ThreadSafeQueue<std::optional<int>>> input_queues(num_threads);
    ThreadSafeQueue<std::optional<int>> output_queue;
    std::vector<std::jthread> workers;

    // Start worker threads
    for (int i = 0; i < num_threads; ++i) {
        workers.emplace_back(worker_function<std::optional<int>>, i, std::ref(input_queues[i]), std::ref(output_queue));
    }

    // Push data to input queues
    for (int i = 0; i < 10; ++i) {
        input_queues[i % num_threads].push(i);
    }

    // Collect results from output queue
    for (int i = 0; i < 10; ++i) {
        auto result = output_queue.pop();
        std::cout << std::format("mainthread receive: {:06d}\n", *result);
    }
    std::cout << "mainthreaed receive all!\n";

    for (size_t i = 0; i < num_threads; ++i) {
        input_queues[i].push(std::nullopt);
    }
}
```

safe queue in a class

```cpp
#include <absl/container/node_hash_map.h>
#include <atomic_queue/atomic_queue.h>
#include <fmt/core.h>

#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <ranges>
#include <string>
#include <thread>
#include <vector>

#include "factors.h"

template <typename T>
using ThreadSafeQueue = atomic_queue::AtomicQueue2<std::optional<T>, 100>;

template <HqData T>
class FactorEngine {
   public:
    using QuoteQueue = ThreadSafeQueue<T>;
    using FactorQueue = ThreadSafeQueue<nlohmann::json>;
    using FactorPtrs = absl::node_hash_map<std::string, std::vector<std::shared_ptr<Factor<T>>>>;
    FactorEngine(QuoteQueue& quote_queue, FactorQueue& output_queue, FactorPtrs& fptr, size_t n_threads = 4) : _fptrs(fptr) {
        _workers.reserve(n_threads);
        for (size_t i = 0; i < n_threads; ++i) {
            _workers.emplace_back(
                [this, i, &quote_queue, &output_queue]() {
                    this->worker_function(i, quote_queue, output_queue);
                });
        }
    }

    void worker_function(size_t idx, QuoteQueue& quote_queue, FactorQueue& output_queue) {
        while (auto quote = quote_queue.pop()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            std::cout << fmt::format("thread-{} processing {}\n", idx, quote->secucode);
            nlohmann::json j = calc_factors(*quote);
            output_queue.push(j);
        }
        std::cout << std::format("thread-{} stop\n", idx);
    }

    nlohmann::json calc_factors(T const& quote) {
        nlohmann::json j;
        j["symbol"] = quote.secucode;
        j["timestamp"] = quote.timestamp;

        for (auto& factor : _fptrs[quote.secucode]) {
            j[factor->name] = factor->update_feature(quote);
        }
        return j;
    }

   private:
    std::vector<std::jthread> _workers;
    FactorPtrs _fptrs;
};

using TickFactorPtrVec = std::vector<std::shared_ptr<Factor<TickData>>>;

auto create_factorptrs() {
    TickFactorPtrVec vec{};

    vec.emplace_back(std::make_shared<Factor01>());
    vec.emplace_back(std::make_shared<Factor02>());
    vec.emplace_back(std::make_shared<Factor03>());
    vec.emplace_back(std::make_shared<Factor04>());

    return vec;
}

auto init_factorptrs_map(std::vector<std::string> const& codeVec) {
    absl::node_hash_map<std::string, TickFactorPtrVec> factorptrs_map{};
    for (auto&& code : codeVec) {
        factorptrs_map[code] = create_factorptrs();
    }
    return factorptrs_map;
}

int main(int argc, char const* argv[]) {
    constexpr int n_threads = 4;

    ThreadSafeQueue<TickData> input_queue;
    ThreadSafeQueue<nlohmann::json> output_queue;

    auto numbers = std::views::iota(0, 10) | std::views::transform([](int i) { return fmt::format("{:06d}", i); });
    std::vector<std::string> vec(numbers.begin(), numbers.end());
    auto fptr = init_factorptrs_map(vec);

    {
        for (size_t i = 0; i < 10; ++i) {
            float tot_av = 100 + i;
            float tot_bv = 200 + i;
            float dt = 1000000 + i;
            TickData tick{.timestamp = dt, .secucode = fmt::format("{:06d}", i), .total_ask_volume = tot_av, .total_bid_volume = tot_bv};
            input_queue.push(tick);
        }
        for (size_t i = 0; i < 10; ++i) {
            float tot_av = 100 + i;
            float tot_bv = 200 + i;
            float dt = 1000000 + i;
            TickData tick{.timestamp = dt, .secucode = fmt::format("{:06d}", i), .total_ask_volume = tot_av, .total_bid_volume = tot_bv};
            input_queue.push(tick);
        }
    }
    {
        FactorEngine<TickData> engine(input_queue, output_queue, fptr, n_threads);

        // Collect results from output queue
        for (int i = 0; i < 20; ++i) {
            auto result = output_queue.pop();
            std::cout << std::format("mainthread receive: {}\n", result->dump());
        }
        std::cout << "mainthreaed receive all!\n";

        for (size_t i = 0; i < n_threads; ++i) {
            input_queue.push(std::nullopt);
        }
    }
}
```

## safe queue with lock

```cpp
#include <condition_variable>
#include <format>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include <vector>

template <typename T>
class ThreadSafeQueue {
   private:
    std::queue<std::optional<T>> queue;
    mutable std::mutex mutex;
    std::condition_variable cond;

   public:
    ThreadSafeQueue() = default;
    ThreadSafeQueue(const ThreadSafeQueue<T>&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue<T>&) = delete;

    void push(std::optional<T> value) {
        std::lock_guard<std::mutex> lock(mutex);
        queue.push(std::move(value));
        cond.notify_one();
    }

    std::optional<T> try_pop() {
        std::lock_guard<std::mutex> lock(mutex);
        if (queue.empty()) {
            return std::nullopt;
        }
        T tmp = std::move(queue.front());
        queue.pop();
        return tmp;
    }

    std::optional<T> wait_and_pop() {
        std::unique_lock<std::mutex> lock(mutex);
        cond.wait(lock, [this] { return !queue.empty(); });
        auto tmp = std::move(queue.front());
        queue.pop();
        return tmp;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.empty();
    }
};

template <typename T>
void worker_function(int idx, ThreadSafeQueue<T>& input_queue, ThreadSafeQueue<T>& output_queue) {
    while (true) {
        auto item = input_queue.wait_and_pop();
        if (!item) break;  // Assuming std::nullopt is used to signal shutdown
        std::cout << std::format("thread-{} processing {}\n", idx, *item);
        output_queue.push(*item * 100000);
    }
    std::cout << std::format("thread-{} stop\n", idx);
}

int main() {
    constexpr int num_threads = 2;
    std::vector<ThreadSafeQueue<int>> input_queues(num_threads);
    ThreadSafeQueue<int> output_queue;
    std::vector<std::jthread> workers;

    // Start worker threads
    for (int i = 0; i < num_threads; ++i) {
        workers.emplace_back(worker_function<int>, i, std::ref(input_queues[i]), std::ref(output_queue));
    }

    // Push data to input queues
    for (int i = 0; i < 10; ++i) {
        input_queues[i % num_threads].push(i);
    }

    // Collect results from output queue
    for (int i = 0; i < 10; ++i) {
        auto result = output_queue.wait_and_pop();
        std::cout << std::format("mainthread receive:{}\n", *result);
    }
    std::cout << "mainthread finished\n";

    // Signal workers to stop (could use a special sentinel value or a shutdown signal)
    for (auto& queue : input_queues) {
        queue.push(std::nullopt);  // Assuming using std::optional<int>
    }
}
```

## bshoshany-thread-pool

> a fast, lightweight, and easy-to-use C++17 thread pool library

`vcpkg install bshoshany-thread-pool`

```cpp
#include <cstddef>
#include <iostream>
#include <ranges>
#include <vector>

#include "BS_thread_pool.hpp"

auto r = std::ranges::views::iota(0, 1001);

auto worker(int const index) {
    return r[index] * 1.1;
}

int main() {
    BS::thread_pool pool;
    int last_idx = 1000;
    auto sequence_future = pool.submit_sequence(0, last_idx + 1, worker);
    auto results = sequence_future.get();
    for (size_t i = 0; i < last_idx + 1; ++i)
        std::cout << std::format("in={:04d},out={:.2f}\n", i, results[i]);
}
```

## apache arrow

`vcpkg install arrow`

### compute

`vcpkg install arrow[compute]`

```cpp
#include <arrow/api.h>
#include <arrow/compute/api.h>

#include <format>
#include <iostream>
arrow::Result<std::shared_ptr<arrow::RecordBatch>> CreateRecordBatch() {
    arrow::Int32Builder builder1;
    arrow::FloatBuilder builder2;

    ARROW_RETURN_NOT_OK(builder1.AppendValues({1, 2, 5, 3, 4}));
    ARROW_RETURN_NOT_OK(builder2.AppendValues({1.1, 3.2, 4.3, 5.6, 7.8}));

    std::shared_ptr<arrow::Array> array1;
    std::shared_ptr<arrow::Array> array2;

    ARROW_RETURN_NOT_OK(builder1.Finish(&array1));
    ARROW_RETURN_NOT_OK(builder2.Finish(&array2));

    auto schema = arrow::schema({arrow::field("column1", arrow::int32()), arrow::field("column2", arrow::float32())});
    return arrow::RecordBatch::Make(schema, array1->length(), {array1, array2});
}

auto FilterRecordBatch(const std::shared_ptr<arrow::RecordBatch>& rb) {
    // array
    auto col1 = rb->column(0);
    // boolean array
    // auto mask = arrow::compute::CallFunction("greater_equal", {col1, arrow::MakeScalar(3)})->make_array();
    auto mask = arrow::compute::CallFunction("greater_equal", {col1, arrow::MakeScalar(3)}).ValueOrDie();

    auto filtered_datum = arrow::compute::CallFunction("filter", {rb, mask});
    auto filtered_tb = filtered_datum->record_batch();
    return filtered_tb;
}

int main() {
    auto result = CreateRecordBatch();
    if (result.ok()) {
        auto rb = result.ValueOrDie();
        auto filtered_rb = FilterRecordBatch(rb);
        std::cout << std::format("rb={}, filtered_rb={}\n", rb->ToString(), filtered_rb->ToString());
    } else {
        std::cerr << "Failed to create RecordBatch: " << result.status().message() << std::endl;
    }
}
```

### get string from ipc file

```py
# export ipc
import polars as pl

df = pl.DataFrame({"col1": ["hello", "world"], "col2": [1, 2]})
df.write_ipc("test.ipc")
```

```cpp
#include <arrow/api.h>
#include <arrow/io/api.h>
#include <arrow/ipc/api.h>

#include <iostream>

arrow::Status read_ipc(std::string const& filename) {
    ARROW_ASSIGN_OR_RAISE(auto infile, arrow::io::ReadableFile::Open(filename, arrow::default_memory_pool()));
    ARROW_ASSIGN_OR_RAISE(auto reader, arrow::ipc::RecordBatchFileReader::Open(infile));
    // only 1 record batch
    auto rb = reader->ReadRecordBatch(0).MoveValueUnsafe();

    std::cout << rb->schema()->ToString() << '\n';  // get the schema to know the Array type

    auto col1_arr = std::static_pointer_cast<arrow::StringViewArray>(rb->GetColumnByName("col1"));
    std::cout << col1_arr->GetString(0) << '\n';  // type is std::string

    auto col2_arr = std::static_pointer_cast<arrow::Int64Array>(rb->GetColumnByName("col2"));
    std::cout << col2_arr->Value(0) << '\n';

    return arrow::Status::OK();
}

int main(int, char**) {
    auto result = read_ipc("test.ipc");
}
#include <arrow/api.h>
#include <arrow/io/api.h>
#include <arrow/ipc/api.h>

#include <iostream>

arrow::Status read_ipc(std::string const& filename) {
    ARROW_ASSIGN_OR_RAISE(auto infile, arrow::io::ReadableFile::Open(filename, arrow::default_memory_pool()));
    ARROW_ASSIGN_OR_RAISE(auto reader, arrow::ipc::RecordBatchFileReader::Open(infile));
    // only 1 record batch
    auto rb = reader->ReadRecordBatch(0).MoveValueUnsafe();

    std::cout << rb->schema()->ToString() << '\n';  // get the schema to know the Array type

    auto col1_arr = std::static_pointer_cast<arrow::StringViewArray>(rb->GetColumnByName("col1"));
    std::cout << col1_arr->GetString(0) << '\n';  // type is std::string

    auto col2_arr = std::static_pointer_cast<arrow::Int64Array>(rb->GetColumnByName("col2"));
    std::cout << col2_arr->Value(0) << '\n';

    return arrow::Status::OK();
}

int main(int, char**) {
    auto result = read_ipc("test.ipc");
}
```

## nng messing library

history: ZeroMQ -> nanomsg -> nng
> `vcpkg install nng`

### one producer multiple consume

```bash
├── CMakeLists.txt
├── recver.cpp
└── sender.cpp
```

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.5.0)
project(proj4 VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
add_executable(sender sender.cpp)
add_executable(recver recver.cpp)

find_package(nng CONFIG REQUIRED)
target_link_libraries(sender PRIVATE nng::nng)
target_link_libraries(recver PRIVATE nng::nng)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(sender PRIVATE spdlog::spdlog)
target_link_libraries(recver PRIVATE spdlog::spdlog)
```

```cpp
// sender.cpp
#include <nng/nng.h>
#include <nng/protocol/pubsub0/pub.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

struct Data {
    int id;
    int preclose;
    int open;
    int high;
    int low;
    int last;
    double amount;
    int64_t volume;
    int ask_vols[10];
    int bid_vols[10];
    double ask_pxs[10];
    double bid_pxs[10];
};

int main() {
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("send.log");
    spdlog::logger logger("mylogger", {file_sink});
    spdlog::set_default_logger(std::make_shared<spdlog::logger>(logger));  // SPDLOG_XXX is default logger
    spdlog::set_pattern("%T.%F,%v");

    nng_socket pub_sock;
    nng_pub0_open(&pub_sock);
    nng_listen(pub_sock, "ipc:///tmp/pubsub.ipc", NULL, 0);

    Data data{};
    size_t idx = 0;
    while (true) {
        data.id = idx;
        data.ask_pxs[5] = idx * 1.1;

        SPDLOG_INFO("BEGIN SEND");
        nng_send(pub_sock, &data, sizeof(Data), 0);
        SPDLOG_INFO("END SEND;{};{}", data.id, data.ask_pxs[5]);
        ++idx;
    }

    nng_close(pub_sock);
}
```

```cpp
// recver.cpp
#include <nng/nng.h>
#include <nng/protocol/pubsub0/sub.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

struct Data {
    int id;
    int preclose;
    int open;
    int high;
    int low;
    int last;
    double amount;
    int64_t volume;
    int ask_vols[10];
    int bid_vols[10];
    double ask_pxs[10];
    double bid_pxs[10];
};

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("too less arguments, enter ./sender filename.log\n");
        exit(-1);
    }
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(argv[1]);
    spdlog::logger logger("mylogger", {file_sink});
    spdlog::set_default_logger(std::make_shared<spdlog::logger>(logger));  // SPDLOG_XXX is default logger
    spdlog::set_pattern("%T.%F,%v");

    nng_socket sub_sock;
    nng_sub0_open(&sub_sock);
    nng_socket_set(sub_sock, NNG_OPT_SUB_SUBSCRIBE, "", 0);
    nng_dial(sub_sock, "ipc:///tmp/pubsub.ipc", NULL, 0);

    Data data{};
    size_t sz = sizeof(Data);
    while (true) {
        SPDLOG_INFO("BEGIN RECV");
        int bytes_received = nng_recv(sub_sock, &data, &sz, 0);
        SPDLOG_INFO("END RECV;{};{}", data.id, data.ask_pxs[5]);
    }

    nng_close(sub_sock);
}
```

```py
# statistic.py
import polars as pl


def get_df(logfile: str) -> pl.DataFrame:
    return pl.read_csv(logfile, separator=",", has_header=False).select(
        pl.col("column_1").str.to_time().cast(pl.Int64).alias("time"),
        pl.col("column_2").str.split(";").list.get(1, null_on_oob=True).alias("id"),
        pl.col("column_2").str.split(";").list.get(2, null_on_oob=True).alias("value"),
    )


df_send = get_df("build/send.log")
df_recv1 = get_df("build/recv1.log")
df_recv2 = get_df("build/recv2.log")


def calc_diff(df: pl.DataFrame) -> pl.DataFrame:
    df1 = df.filter(pl.col("id").is_null()).rename({"time": "t1", "id": "id1", "value": "v1"})
    df2 = df.filter(pl.col("id").is_not_null()).rename({"time": "t2", "id": "id2", "value": "v2"})
    return pl.concat([df1, df2], how="horizontal").with_columns((pl.col("t2") - pl.col("t1")).alias("diff"))


stat_send = calc_diff(df_send)  # mean 15.25us, mid 11.7us
stat_recv1 = calc_diff(df_recv1)  # mean 21.52us, mid 18.6us
stat_recv2 = calc_diff(df_recv2)

stat_send.describe()  # mean 17.6us, mid 16.1us, max 8.1 ms

stat_recv1.describe()  # mean 22.86us, mid 20.2us, max 7.5ms

stat_recv2.describe()  # mean 25.44us, mid 26.9us, max 9.2ms

dfx = df_recv1.join(df_send, on="id", how="left").with_columns((pl.col("time") - pl.col("time_right")).alias("diff"))
dfx.describe()  # mean 96.9us, mid 11.9us max 7.47ms

dfx[dfx.select(pl.col("diff").arg_max()).row(0)]
```

## compresssion

### zstd

`vcpkg install zstd`

```cpp
#include <zstd.h>  // Zstandard library

#include <bit>
#include <cstring>  // for std::memcpy
#include <iostream>
#include <vector>

struct MyStruct {
    int id;
    double value;
    char name[10];
};

std::vector<char> serialize(const MyStruct& data) {
    std::vector<char> buffer(sizeof(data));
    // stack to heap
    std::memcpy(buffer.data(), &data, sizeof(data));
    return buffer;
}

std::vector<char> compress(const std::vector<char>& data) {
    size_t compressedSize = ZSTD_compressBound(data.size());
    std::vector<char> compressedData(compressedSize);
    size_t actualCompressedSize = ZSTD_compress(compressedData.data(), compressedSize, data.data(), data.size(), 10);

    if (ZSTD_isError(actualCompressedSize)) {
        std::cerr << "Compression error: " << ZSTD_getErrorName(actualCompressedSize) << std::endl;
        return {};
    }

    compressedData.resize(actualCompressedSize);
    return compressedData;
}

MyStruct deserializeAndDecompress(const std::vector<char>& compressedData) {
    size_t originalSize = sizeof(MyStruct);
    std::vector<char> decompressedData(originalSize);
    size_t decompressedSize = ZSTD_decompress(decompressedData.data(), originalSize, compressedData.data(), compressedData.size());

    if (ZSTD_isError(decompressedSize)) {
        std::cerr << "Decompression error: " << ZSTD_getErrorName(decompressedSize) << std::endl;
        throw std::runtime_error("Decompression failed");
    }

    MyStruct result;
    // // just ref heap
    // auto xx = std::bit_cast<MyStruct*>(decompressedData.data());
    // heap to stack
    std::memcpy(&result, decompressedData.data(), decompressedSize);
    return result;
}

int main() {
    MyStruct original = {1, 3.14, "example"};

    auto serialized = serialize(original);
    auto compressed = compress(serialized);
    auto decompressedStruct = deserializeAndDecompress(compressed);

    std::cout << "Original ID: " << original.id << ", Decompressed ID: " << decompressedStruct.id << std::endl;
    std::cout << "Original Value: " << original.value << ", Decompressed Value: " << decompressedStruct.value << std::endl;
    std::cout << "Original Name: " << original.name << ", Decompressed Name: " << decompressedStruct.name << std::endl;
}
```

## testing

### gtest

`vcpkg install gtest`

simple gtest framework

```bash
.
├── CMakeLists.txt
├── main.cpp
├── ops.cpp
├── ops.h
└── ops_test.cpp
```

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.20.0)
project(proj1 VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
add_executable(proj1 main.cpp ops.cpp)

# Option to build tests
option(BUILD_TESTS "Build test programs" OFF)

if(BUILD_TESTS)
    find_package(GTest CONFIG REQUIRED)
    enable_testing()

    add_executable(ops_test ops.cpp ops_test.cpp)
    target_link_libraries(ops_test PRIVATE GTest::gtest GTest::gtest_main)
    add_test(NAME OpsTest COMMAND ops_test)
endif()
```

```cpp
// ops.h
#pragma once

int myadd(int x, int y);
int mysub(int x, int y);
```

```cpp
// ops.cpp
#include "ops.h"

int myadd(int x, int y) {
    return x + y;
}

int mysub(int x, int y) {
    return x - y;
}
```

```cpp
// ops_test.cpp
#include "ops.h"

#include <gtest/gtest.h>

TEST(OpsTest, Add) {
    EXPECT_EQ(myadd(1, 2), 3);
}

TEST(OpsTest, Sub) {
    EXPECT_EQ(mysub(1, 2), -1);
}
```

```cpp
// main.cpp
#include <cstdio>
#include "ops.h"

int main(int argc, char const *argv[]) {
    printf("myadd(10, 20)=%d\n", myadd(10, 20));
}
```

normal file tree of gtest

```bash
.
├── CMakeLists.txt
├── main.cpp
├── ops.cpp
├── ops.h
└── tests
    ├── CMakeLists.txt
    └── ops_test.cpp
```

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.20.0)
project(proj1 VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
add_executable(proj1 main.cpp ops.cpp)

# Option to build tests
option(BUILD_TESTS "Build test programs" ON)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
```

```cmake
# tests/CMakeLists.txt
find_package(GTest CONFIG REQUIRED)

# Test executable for Student
add_executable(ops_test ops_test.cpp ${PROJECT_SOURCE_DIR}/ops.cpp)
target_link_libraries(ops_test PRIVATE GTest::gtest GTest::gtest_main)
add_test(NAME OpsTest COMMAND ops_test)
```

```cpp
// tests/ops_test.cpp
#include <gtest/gtest.h>

#include "../ops.h"

TEST(OpsTest, CheckFunction) {
    EXPECT_EQ(myadd(10, 20), 30);
}
```