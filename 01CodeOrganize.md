# Orgnize Code

- [Orgnize Code](#orgnize-code)
  - [clangd for C++](#clangd-for-c)
  - [by Multifiles](#by-multifiles)
  - [Class Organize](#class-organize)

## clangd for C++

in Linux:
1. `sudo apt install clangd-15`
2. install vscode extension [clangd](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd)
3. config `settings.json` of vscode
4. add global format file to `/home/username/.clang-format`

```json
// settings.json
{
    "clangd.path": "/usr/bin/clangd-15",
    "clangd.arguments": [
        "--clang-tidy"
    ]
}
```

```bash
# /home/username/.clang-format
IndentWidth: 4
ColumnLimit: 0
```

in Windows:
1. download [llvm-mingw](https://github.com/mstorsjo/llvm-mingw/releases)
2. install vscode extension [clangd](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd)
3. config `settings.json` of vscode
4. add global format file to `D:/.clang-format`, which is the top level of your projects

```json
// settings.json
{
    "clangd.path": "D:/Dev/llvm-mingw/bin/clangd.exe",
    "clangd.arguments": [
        "--clang-tidy",
    ],
}
```

```bash
# D:/.clang-format
IndentWidth: 4
ColumnLimit: 0
```

> Attention: vscode extension [clangd](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd)没有debugger功能，需要另外安装[CodeLLDB](https://marketplace.visualstudio.com/items?itemName=vadimcn.vscode-lldb), [CodeLLDB Debug Tutorial](README.md#lldb-in-vscode)

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

## Class Organize

```cpp
#pragma once

#include <ostream>

class point3d {
    int x_;
    int y_;
    int z_;

   public:
    point3d(int const x = 0, int const y = 0, int const z = 0) : x_(x), y_(y), z_(z) {}

    int x() const { return x_; }
    point3d& x(int const x) {
        x_ = x;
        return *this;
    }
    int y() const { return y_; }
    point3d& y(int const y) {
        y_ = y;
        return *this;
    }
    int z() const { return z_; }
    point3d& z(int const z) {
        z_ = z;
        return *this;
    }

    bool operator==(point3d const& pt) const {
        return x_ == pt.x_ && y_ == pt.y_ && z_ == pt.z_;
    }

    bool operator!=(point3d const& pt) const {
        return !(*this == pt);
    }

    bool operator<(point3d const& pt) const {
        return x_ < pt.x_ || y_ < pt.y_ || z_ < pt.z_;
    }

    friend std::ostream& operator<<(std::ostream& stream, point3d const& pt) {
        stream << "(" << pt.x_ << "," << pt.y_ << "," << pt.z_ << ")";
        return stream;
    }

    void offset(int const offsetx, int const offsety, int const offsetz) {
        x_ += offsetx;
        y_ += offsety;
        z_ += offsetz;
    }

    static point3d origin() { return point3d{}; }
};
```