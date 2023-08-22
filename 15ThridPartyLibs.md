# Third-Party Library

- [Third-Party Library](#third-party-library)
  - [pybind11](#pybind11)


## pybind11

> [pybind11](https://github.com/pybind/pybind11) is a lightweight header-only library that exposes C++ types in Python and vice versa, mainly to create Python bindings of existing C++ code.

simple example of **pybind11**
> 因为编译出来是`libproj1.so`, 所以需要`PYBIND11_MODULE(libproj1, m)`中设置为`libproj1`

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
message("> Python_INCLUDE_DIRS = ${Python_INCLUDE_DIRS}")
# include Python.h
include_directories(${Python_INCLUDE_DIRS})

# include pybind11/*.hpp
# download from https://github.com/pybind/pybind11/tree/master/include/pybind11
include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(proj1 SHARED main.cpp)
```

```cpp
// main.cpp
#include <pybind11/pybind11.h>

int add(int i, int j) {
    return i + j;
}

PYBIND11_MODULE(libproj1, m) { // must be libproj1
    m.doc() = "pybind11 libproj1 plugin";  // optional module docstring

    m.def("add", &add, "A function that adds two numbers");
}
```

change output prefix

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.25.0)
project(proj1 VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

find_package (Python COMPONENTS Development REQUIRED)
include_directories(${Python_INCLUDE_DIRS})

message("> Python_INCLUDE_DIRS = ${Python_INCLUDE_DIRS}")
include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(proj1 SHARED main.cpp)
# remove prefix "lib": libproj1->proj1
set_target_properties(proj1 PROPERTIES PREFIX "")
```

```cpp
// main.cpp
#include <pybind11/pybind11.h>

int add(int i, int j) {
    return i + j;
}

PYBIND11_MODULE(proj1, m) { // change here libproj1->proj1
    m.doc() = "pybind11 proj1 plugin"; 

    m.def("add", &add, "A function that adds two numbers");
}
```