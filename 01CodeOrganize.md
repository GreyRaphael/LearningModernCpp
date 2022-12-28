# Orgnize Code

- [Orgnize Code](#orgnize-code)
  - [by Multifiles](#by-multifiles)


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