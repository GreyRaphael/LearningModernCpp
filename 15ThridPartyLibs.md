# Third-Party Library

- [Third-Party Library](#third-party-library)
  - [pybind11](#pybind11)


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