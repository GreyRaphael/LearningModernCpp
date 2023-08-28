# Third-Party Library

- [Third-Party Library](#third-party-library)
  - [pybind11](#pybind11)
  - [`nlohmann::json`](#nlohmannjson)
  - [Code Organized by CMake](#code-organized-by-cmake)


## pybind11

> [pybind11](https://github.com/pybind/pybind11) is a lightweight header-only library that exposes C++ types in Python and vice versa, mainly to create Python bindings of existing C++ code.

simple example of **pybind11** in Linux

```bash
├── CMakeLists.txt
├── include
│   └── pybind11
└── main.cpp
```

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.25.0)
project(proj1 VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

# cmake version > 3.12.0, https://cmake.org/cmake/help/latest/module/FindPython.html
find_package (Python COMPONENTS Development REQUIRED)
# include Python.h
include_directories(${Python_INCLUDE_DIRS})
message("> Python_INCLUDE_DIRS = ${Python_INCLUDE_DIRS}")

# include pybind11/*.hpp
# download from https://github.com/pybind/pybind11/tree/master/include/pybind11
include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(proj1 SHARED main.cpp)
# remove prefix "lib": libproj1->proj1
set(CMAKE_SHARED_LIBRARY_PREFIX "")
```

```cpp
// main.cpp
#include <pybind11/pybind11.h>

int add(int i, int j) {
    return i + j;
}

// 因为编译出来是`proj1.so`, 所以需要`PYBIND11_MODULE(proj1, m)`中设置为`proj1`
PYBIND11_MODULE(proj1, m) { // change here libproj1->proj1
    m.doc() = "pybind11 proj1 plugin"; 

    m.def("add", &add, "A function that adds two numbers");
}
```

```py
# test proj1.so
import proj1

num = proj1.add(200, 100) # 300
```

example: support Linux & Windows
> output is `proj1.cp39-win_amd64.pyd` or `proj1.cpython-310-x86_64-linux-gnu.so`

```cmake
cmake_minimum_required(VERSION 3.25.0)
project(proj1 VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)

find_package (Python COMPONENTS Interpreter Development REQUIRED)

message("> Python_INCLUDE_DIRS = ${Python_INCLUDE_DIRS}")
include_directories(${Python_INCLUDE_DIRS})

# get .cp39-win_amd64.pyd or .cpython-310-x86_64-linux-gnu.so
execute_process(
  COMMAND "${Python_EXECUTABLE}" "-c" "import sys, importlib; s = importlib.import_module('distutils.sysconfig' if sys.version_info < (3, 10) else 'sysconfig'); print(s.get_config_var('EXT_SUFFIX') or s.get_config_var('SO'))"
  OUTPUT_VARIABLE _PYTHON_MODULE_EXTENSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
message("> _PYTHON_MODULE_EXTENSION = ${_PYTHON_MODULE_EXTENSION}")
# proj1.cp39-win_amd64.pyd or proj1.cpython-310-x86_64-linux-gnu.so
set(CMAKE_SHARED_LIBRARY_SUFFIX "${_PYTHON_MODULE_EXTENSION}")
# libxxxx.so -> xxxx.so
set(CMAKE_SHARED_LIBRARY_PREFIX "")

# include pybind11/*.hpp
include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(proj1 SHARED main.cpp)

if(WIN32)
    # in windows, should linked with python3x.lib, compiler can be MSVC or mingw
    target_link_libraries(proj1 PUBLIC Python::Python)
endif()
```

```cpp
// main.cpp
#include <pybind11/pybind11.h>

int add(int i, int j) {
    return i + j;
}

PYBIND11_MODULE(proj1, m) {
    m.doc() = "pybind11 proj1 plugin";  // optional module docstring

    m.def("add", &add, "A function that adds two numbers");
}
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

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(proj1 main.cpp)
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

include_directories(${CMAKE_SOURCE_DIR}/mylib1/include)
include_directories(${CMAKE_SOURCE_DIR}/mylib2/include)

add_executable(proj1 main.cpp)

link_directories(${CMAKE_BINARY_DIR}/mylib1)
link_directories(${CMAKE_BINARY_DIR}/mylib2)

target_link_libraries(proj1 PUBLIC mymath)
target_link_libraries(proj1 PUBLIC mypoint)
```

```cmake
# mylib1/CMakeLists.txt
cmake_minimum_required(VERSION 3.25.0)
project(mymath VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(mymath SHARED myadd.cpp)
```

```cmake
# mylib2/CMakeLists.txt
cmake_minimum_required(VERSION 3.25.0)
project(mypoint VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(mypoint SHARED point3d.cpp)
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