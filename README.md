 Learning Modern C++

>  Learning Notes for Modern C++, examples from [Modern C++ Programming Cookbook - 2nd Edition](https://github.com/PacktPublishing/Modern-CPP-Programming-Cookbook-Second-Edition)

- [Development Environment in Debian](#development-environment-in-debian)
  - [GCC \& Clang in VSCode](#gcc--clang-in-vscode)
  - [clangd for C++](#clangd-for-c)
- [Development Environment Online](#development-environment-online)
- [Othre configuration](#othre-configuration)
  - [linux locale config](#linux-locale-config)
- [How to debug program with arguments in vscode](#how-to-debug-program-with-arguments-in-vscode)
- [How to use vcpkg in vscode](#how-to-use-vcpkg-in-vscode)
  - [add proxy for aliyun](#add-proxy-for-aliyun)
  - [msvc](#msvc)
  - [mingw(not recomended)](#mingwnot-recomended)
  - [linux](#linux)
  - [vcpkg tips](#vcpkg-tips)
    - [fix vcpkg hash mismatch](#fix-vcpkg-hash-mismatch)
    - [Export vcpkg](#export-vcpkg)
- [check hash](#check-hash)
- [cmake](#cmake)
  - [use pthread in cmake](#use-pthread-in-cmake)
  - [linux library usage](#linux-library-usage)
  - [windows library usage](#windows-library-usage)
- [build clangd from source](#build-clangd-from-source)
- [use git submodule](#use-git-submodule)

## Development Environment in Debian

> 实机和wsl均可这么配置，下文以WSL为例

1. Download [wsl debian11 image](https://learn.microsoft.com/en-us/windows/wsl/install-manual), and install(直接解压，然后以Administrator权限运行debian.exe)
2. update to debian `testing` version(testing版debian软件版本较新)
3. Install development environment

```bash
# 1. Download image & Install
# 2. Debain11 -> testing
sudo apt update
sudo apt install apt-transport-https ca-certificates

# 使用https://mirrors.tuna.tsinghua.edu.cn/help/debian/的Debian11源，将所有bullseye修改为testing，
sudo vi /etc/apt/sources.list
sudo apt update && sudo apt upgrade -y

# 在powershell中wslconfig /t Debian,然后重新进入wsl

# 检查版本
cat /etc/debian_version

sudo apt --purge autoremove -y

# 3. Install development environment
sudo apt install build-essential
sudo apt install clang gdb git cmake ninja-build -y
# rust
sudo apt install rustc rust-src rustfmt
```

Debian11->12 problems
- [solution](https://github.com/microsoft/WSL/issues/4279#issuecomment-1639165782) for problem: `mv: cannot move '/lib/x86_64-linux-gnu/security' to '/usr/lib/x86_64-linux-gnu/security': Permission denied`

Debian11->testing problems
- [solution](https://github.com/microsoft/WSL/issues/10397#issuecomment-1682139166) for problem: `/etc/passwd lock: Invalid argument`

### GCC & Clang in VSCode

Just install vscode extension [clangd](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd)

config `setttings.json` in linux
> gcc-13和clang-16都能使用gdb来调试

```json
// setings.json in linux
{
    "cmake.cmakePath": "/usr/bin/cmake",
    "cmake.generator": "Ninja",
    "cmake.configureSettings": {
        "CMAKE_MAKE_PROGRAM": "/usr/bin/ninja"
    },
    "clangd.path": "clangd-16",
    "clangd.arguments": [
        "--clang-tidy",
    ]
}
```

windows下的MinGw可以使用如下配置(不推荐MinGW)

```json
// settings.json in windows of MingW by gcc and llvm
{
  "cmake.cmakePath": "D:/Dev/winlibs-mingw64/bin/cmake.exe",
  "cmake.generator": "Ninja",
  "cmake.configureSettings": {
      "CMAKE_MAKE_PROGRAM": "D:/Dev/winlibs-mingw64/bin/ninja.exe"
  },
  "cmake.debugConfig": {
      "MIMode": "gdb",
      "miDebuggerPath": "D:/Dev/winlibs-mingw64/bin/gdb.exe"
  },
  // 给vscode提供其他compiler列表
  "cmake.additionalCompilerSearchDirs": [
      "D:/Dev/winlibs-mingw64/bin",
  ],
}
```

### clangd for C++

in Linux:
1. `sudo apt install clangd-16`
2. install vscode extension [clangd](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd)
3. config `settings.json` of vscode
4. add global format file to `/home/username/.clang-format`

```json
// settings.json
{
    "clangd.path": "/usr/bin/clangd-16",
    "clangd.arguments": [
        "--clang-tidy"
    ]
}
```

```bash
# /home/username/.clang-format
BasedOnStyle: Google
IndentWidth: 4
ColumnLimit: 0
```

## Development Environment Online

- [Wandbox](https://wandbox.org/)
- [Compiler Explorer](https://godbolt.org/)

## Othre configuration

### linux locale config

```bash
# install locale
locale -a
# C
# C.utf8
# POSIX
# en_US.utf8
sudo dpkg-reconfigure locales
# 空格键选择zh_CN.gbk
# 然后选择main display language: en_US.utf8

locale -a
# C
# C.utf8
# POSIX
# en_US.utf8
# zh_CN.gbk
```

## How to debug program with arguments in vscode

instal vGDB extension

```cmake
cmake_minimum_required(VERSION 3.24.0)
project(proj1 VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
add_executable(proj1 main.cpp)
```

```cpp
// program for check args
#include <format>
#include <iostream>

int main(int argc, char const *argv[]) {
    for (size_t i = 0; i < argc; ++i) {
        std::cout << std::format("arg-{}: {}\n", i, argv[i]);
    }
}
```

Method1: by `.vscode/launch.json`
1. create a `launch.json` in vscode left panel, change `args` field.
2. run `(gdb) Launch` in left panel

```json
// launch.json
{
    "version": "0.2.0",
    "configurations": [
        {
            "type": "vgdb",
            "request": "launch",
            "name": "C/C++ Debug",
            "program": "${workspaceFolder}/build/proj1",
            "args": [],
            "cwd": "${workspaceFolder}"
        }
    ]
}
```

Method2: by `.vscode/settings.json`
1. create `settings.json`, add following fields
2. debug

```json
// settings.json
{
    "cmake.debugConfig": {
        "args": [
            "config\\journal_config.json",
            "strategy",
            "config\\sbq_config.json",
        ]
    },
}
```

## How to use vcpkg in vscode

### add proxy for aliyun

> by [ShellCrash](https://github.com/juewuy/ShellCrash/blob/dev/README_CN.md)

```bash
sudo -i #切换到root用户，如果需要密码，请输入密码
bash #如已处于bash环境可跳过
export url='https://gh.jwsc.eu.org/master' && bash -c "$(curl -kfsSl $url/install.sh)" && source /etc/profile &> /dev/null

# 直接配置订阅地址，不转换
# 配置本机证书和代理
```

### msvc

step1: install vcpkg

```bash
cd D:\Dev\
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# install package
vcpkg install fmt
# specify triplet for *.dll
vcpkg install spdlog:x64-windows
# or use static library *.lib
vcpkg install spdlog:x64-windows-static
```

step2: config in vscode

```json
"cmake.configureSettings": {
    "CMAKE_TOOLCHAIN_FILE":"D:/Dev/vcpkg/scripts/buildsystems/vcpkg.cmake",
},
```

step3: [cmake integration](https://learn.microsoft.com/en-us/vcpkg/users/buildsystems/cmake-integration)
> add triplet according to your installation before `project()`

```cmake
cmake_minimum_required(VERSION 3.28.0)

# leave empty or 
set(VCPKG_TARGET_TRIPLET x64-windows-static)

project(proj1 VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(proj1 main.cpp)

# consume packages
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(proj1 PRIVATE spdlog::spdlog)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(proj1 PRIVATE fmt::fmt)
```

### mingw(not recomended)

step1: install vcpkg

```bash
cd D:\Dev\
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# install first package
vcpkg install spdlog --triplet=x64-mingw-dynamic --host-triplet=x64-mingw-dynamic
# install other package
vcpkg install fmt:x64-mingw-dynamic
```

step2: config in vscode

```json
"cmake.configureSettings": {
    "CMAKE_TOOLCHAIN_FILE":"D:/Dev/vcpkg/scripts/buildsystems/vcpkg.cmake",
},
```

step3: [cmake integration](https://learn.microsoft.com/en-us/vcpkg/users/buildsystems/cmake-integration)
> add triplet according to your installation before `project()`

```cmake
cmake_minimum_required(VERSION 3.28.0)

# must specify triplet
set(VCPKG_TARGET_TRIPLET x64-mingw-dynamic)

project(proj1 VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(proj1 main.cpp)

# consume packages
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(proj1 PRIVATE spdlog::spdlog)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(proj1 PRIVATE fmt::fmt)
```

### linux

step1: install vcpkg

```bash
cd
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh

# default use static libary
vcpkg install spdlog
# use shared library *.so
vcpkg install fmt:x64-linux-dynamic
```

step2: config in vscode

```json
"cmake.configureSettings": {
    "CMAKE_TOOLCHAIN_FILE":"/home/your_name/vcpkg/scripts/buildsystems/vcpkg.cmake",
},
```

step3: [cmake integration](https://learn.microsoft.com/en-us/vcpkg/users/buildsystems/cmake-integration)
> add triplet according to your installation before `project()`

```cmake
cmake_minimum_required(VERSION 3.28.0)

# leave empty or use shard library
set(VCPKG_TARGET_TRIPLET x64-linux-dynamic)

project(proj1 VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(proj1 main.cpp)

# consume packages
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(proj1 PRIVATE spdlog::spdlog)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(proj1 PRIVATE fmt::fmt)
```

### vcpkg tips

```cmake
# scripts/buildsystems/vcpkg.cmake
# ...
# add this line to the bottom
set(CMAKE_CXX_STANDARD 20)
```

```cmake
# triplets/x64-linux.cmake
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)
set(VCPKG_BUILD_TYPE release)

set(VCPKG_CMAKE_SYSTEM_NAME Linux)

set(VCPKG_FIXUP_ELF_RPATH ON)
```

simple vcpkg with version project

```bash
.
├── CMakeLists.txt
├── main.cpp
└── vcpkg.json
```

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.28.0)
project(proj1 VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/debug)

add_executable(proj1 main.cpp)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(proj1 PRIVATE spdlog::spdlog)
```

```cpp
// main.cpp
#include <spdlog/spdlog.h>

int main(int, char**) {
    SPDLOG_INFO("HELLO-{}", 100);
}
```

```json
// vcpkg.json
{
    "dependencies": [
        "spdlog"
    ],
    "builtin-baseline": "01f602195983451bc83e72f4214af2cbc495aa94",
    "overrides": [
        {
            "name": "spdlog",
            "version": "1.13.0"
        }
    ]
}
```

#### fix vcpkg hash mismatch

```bash
/home/cauchy/vcpkg/buildtrees/versioning_/versions/thrift/6855be1ce96497811d4eb0a9879baf6cf1b3610c: info: installing overlay port from here
-- Downloading https://github.com/apache/thrift/archive/0.20.0.tar.gz -> apache-thrift-0.20.0.tar.gz...
error: Missing apache-thrift-0.20.0.tar.gz and downloads are blocked by x-block-origin.
error: File does not have the expected hash:
url: https://github.com/apache/thrift/archive/0.20.0.tar.gz
File: /home/cauchy/vcpkg/downloads/apache-thrift-0.20.0.tar.gz.142.part
Expected hash: 5e4ee9870b30fe5ba484d39781c435716f7f3903793dc8aae96594ca813b1a5a73363b84719038ca8fa3ab8ef0a419a28410d936ff7b3bbadf36fc085a6883ae
Actual hash: 86a609f5725506abc2109902b338cbaad85abf980f9d28c63e6e50a5c265dc10699372c4e91f5ee85d8d09fbcbb0dc17c19745b86d215b96bbf7d0a9270f0932
```

```bash
# how to fix
cd /home/cauchy/vcpkg/buildtrees/versioning_/versions/thrift/6855be1ce96497811d4eb0a9879baf6cf1b3610c
vi port.cmake
# change the expected hash to 86a609f5725506abc2109902b338cbaad85abf980f9d28c63e6e50a5c265dc10699372c4e91f5ee85d8d09fbcbb0dc17c19745b86d215b96bbf7d0a9270f0932
```

#### Export vcpkg

> export vcpkg libraries to a zip file, then other people can use libraries without install vcpkg

当使用vcpkg导出得库得时候，debug对应debug, release对应release
> 如果配置了`set(VCPKG_BUILD_TYPE release)`, debug使用release的版本，会报错

```bash
# vcpkg.json in a project
.
└── vcpkg.json
```

```json
// vcpkg.json
{
    "dependencies": [
        "spdlog",
        "boost-circular-buffer",
        {
            "name": "arrow",
            "default-features": false
        }
    ],
    "builtin-baseline": "f7423ee180c4b7f40d43402c2feb3859161ef625",
    "overrides": [
        {
            "name": "spdlog",
            "version": "1.13.0"
        },
        {
            "name": "boost-circular-buffer",
            "version": "1.85.0"
        },
        {
            "name": "arrow",
            "version": "16.1.0"
        }
    ]
}
```

```bash
# install libraries
vcpkg install
vcpkg export --zip --output-dir=.\exports

# Zip archive exported at: proj2/.exports/vcpkg-export-20240716-222831.zip
# To use exported libraries in CMake projects, add -DCMAKE_TOOLCHAIN_FILE=[...]/scripts/buildsystems/vcpkg.cmake to your CMake command line.
```

```bash
.
├── .exports
│   └── vcpkg-export-20240716-222831.zip
├── vcpkg_installed
│   ├── vcpkg
│   │   ├── info
│   │   ├── status
│   │   └── updates
│   └── x64-linux
│       ├── include
│       ├── lib
│       └── share
└── vcpkg.json
```

prohibit from pdb genenration in windows is not recommended
- in windows, debug project need *.dll built in debug mode
- in windows, release project need *.dll built in release mode
- debug project cannot use *.dll built in release mode, release project cannot use *.dll built in debug mode, 

```cmake
# vcpkg/triplets/x64-windows.cmake
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)
set(VCPKG_BUILD_TYPE release)

# Disable PDB generation
set(VCPKG_C_FLAGS_RELEASE "/Zi-")
set(VCPKG_CXX_FLAGS_RELEASE "/Zi-")
set(VCPKG_LINKER_FLAGS_RELEASE "/DEBUG:NONE")
```

## check hash

```bash
# in windows
certutil -hashfile .\thrift-0.20.0.tar.gz SHA512

# in linux
sha512sum apache-thrift-0.20.0.tar.gz
```

## cmake

```bash
# 1. add CMakeLists.txt file
# 2. cmake config
cmake -S . -B build     # reads your manifest and toolchain
# 3. build
cmake --build build
```

### use pthread in cmake

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.24.0)
project(proj1 VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
add_executable(server server.cpp)
add_executable(client client.cpp)

find_package(Threads REQUIRED)

include(FetchContent)
FetchContent_Declare(
    yalantinglibs
    GIT_REPOSITORY https://github.com/alibaba/yalantinglibs.git
    GIT_TAG main # optional ( default master / main )
    GIT_SHALLOW 1 # optional ( --depth=1 )
)
FetchContent_MakeAvailable(yalantinglibs)

target_link_libraries(server PRIVATE yalantinglibs::yalantinglibs)
target_link_libraries(client PRIVATE yalantinglibs::yalantinglibs)

# link pthread in cmake
target_link_libraries(server PRIVATE Threads::Threads)
target_link_libraries(client PRIVATE Threads::Threads)
```

### linux library usage

for shared library, the output files will be
- `libmylib.so`
- `proj1`

```cmake
cmake_minimum_required(VERSION 3.20.0)
project(proj VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
# shared library here
add_library(mylib SHARED mylib.cpp)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(mylib PRIVATE fmt::fmt)

add_executable(proj1 main.cpp)
target_link_libraries(proj1 PRIVATE mylib)
```

for static libray, the output files will be
- `libmylib.a`
- `proj1`

```cmake
cmake_minimum_required(VERSION 3.20.0)
project(proj VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
add_library(mylib mylib.cpp)
# default is static library
# add_library(mylib STATIC mylib.cpp)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(mylib PRIVATE fmt::fmt)

add_executable(proj1 main.cpp)
target_link_libraries(proj1 PRIVATE mylib)
```

### windows library usage

for shared library, must set `__declspec(dllexport)` manually or `WINDOWS_EXPORT_ALL_SYMBOLS` in cmake, the output files will be
- `mylib.dll`: **Dynamic Link Library**, *necessary*.
- `mylib.lib`: **import library**, used by other project when linking. *necessary*
- `mylib.exp`: **export file**. It contains information about the symbols (functions, variables) that are exported from the DLL. This file is used by the linker to create the **import library** (.lib) and to resolve references to the exported symbols. *not necessary*
- `proj1.exe`

```cmake
cmake_minimum_required(VERSION 3.20.0)
project(proj VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)

add_library(mylib SHARED mylib.cpp)
# in windows, this command genenrate "import library" like mylib.lib for the usage in other project
set_target_properties(mylib PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(mylib PRIVATE fmt::fmt)

add_executable(proj1 main.cpp)
target_link_libraries(proj1 PRIVATE mylib)
```

for static library, there is no need set `WINDOWS_EXPORT_ALL_SYMBOLS`, the output files will be
- `mylib.lib`: **static library**, used by other project when linking. *necessary*
- `proj1.exe`

```cmake
cmake_minimum_required(VERSION 3.20.0)
project(proj VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)

add_library(mylib mylib.cpp)
# add_library(mylib STATIC mylib.cpp)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(mylib PRIVATE fmt::fmt)

add_executable(proj1 main.cpp)
target_link_libraries(proj1 PRIVATE mylib)
```

## build clangd from source

[tutorial](https://jdhao.github.io/2021/07/03/install_clangd_on_linux/)

`clangd` is a tool in `clang-tools-extra`, and the build of `clang-tools-extra` dependends on `clang`

download source like `llvm-project-15.0.7.src.tar.xz` from [release](https://github.com/llvm/llvm-project/releases)
> redhat7 devtoolset-11支持编译到clang19

```bash
wget https://github.com/llvm/llvm-project/releases/download/llvmorg-15.0.7/llvm-project-15.0.7.src.tar.xz

tar -xvf *.xz
cd llvm-project-15.0.7.src
mkdir build && cd build

# or -G "Unix Makefiles"
# DCMAKE_INSTALL_PREFIX 可以最后指定, 默认/usr/local
# LLVM_BUILD_LLVM_DYLIB，LLVM_LINK_LLVM_DYLIB 选用动态链接节省空间，默认静态链接
cmake -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" \
    -DLLVM_TARGETS_TO_BUILD="X86" \
    -DLLVM_INCLUDE_TESTS=OFF \
    -DLLVM_BUILD_LLVM_DYLIB=ON \
    -DLLVM_LINK_LLVM_DYLIB=ON \
    -DCMAKE_CXX_STANDARD=20 \
    ../llvm

cmake --build . -j 40
# install alll
cmake --install .
# 安装时才指定路径
cmake --install build --prefix "/your/custom/path"
# # install clang,clang++,clangd
# cmake --build . --target install-clang install-clang-resource-headers install-clangd
# 安装文件为build/install_manifest.txt, 可以通过 sudo xargs rm < install_manifest.txt 卸载

# check
~/tools/bin/clangd --version

vi .bashrc
export PATH=$HOME/tools/bin:$PATH
```

如果想分发给其他人使用，使用静态链接

```bash
cmake -G Ninja \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" \
    -DLLVM_TARGETS_TO_BUILD="X86" \
    -DLLVM_INCLUDE_TESTS=OFF \
    -DCMAKE_CXX_STANDARD=20 \
    -DCMAKE_INSTALL_PREFIX=../llvm_dist \
    ../llvm
cmake --build . -j 8
cmake --install .
# distribution
zip llvm_dist.zip llvm_dist/ -r
```

## use git submodule

```bash
mkdir proj && cd proj
git init # add .git directory

git submodule add https://github.com/MengRao/fmtlog
git submodule update --init --recursive
# use vscode panel to change version to wanted

# add CMakeLists.txt
```

```cmake
cmake_minimum_required(VERSION 3.20)
project(proj1 VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
add_executable(proj1 main.cpp)

add_subdirectory(fmtlog)
target_link_libraries(proj1 PRIVATE fmtlog-shared)
target_link_libraries(proj1 PRIVATE fmt::fmt)
```