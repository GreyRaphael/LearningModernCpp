 Learning Modern C++

>  Learning Notes for Modern C++, examples from [Modern C++ Programming Cookbook - 2nd Edition](https://github.com/PacktPublishing/Modern-CPP-Programming-Cookbook-Second-Edition)

- [Development Environment in WSL](#development-environment-in-wsl)
  - [wsl basic cmds](#wsl-basic-cmds)
  - [wsl with proxy](#wsl-with-proxy)
  - [wsl with git](#wsl-with-git)
- [Development Environment in Debian](#development-environment-in-debian)
  - [GCC \& Clang in VSCode](#gcc--clang-in-vscode)
  - [clangd for C++](#clangd-for-c)
- [Development Environment in CentOS7](#development-environment-in-centos7)
- [Development Environment in Fedora38](#development-environment-in-fedora38)
- [Development Environment in Fedora Rawhide](#development-environment-in-fedora-rawhide)
- [Development Environment Online](#development-environment-online)
- [Othre configuration](#othre-configuration)
  - [linux locale config](#linux-locale-config)
- [How to debug program with arguments in CMakeTools](#how-to-debug-program-with-arguments-in-cmaketools)
- [How to use vcpkg in vscode](#how-to-use-vcpkg-in-vscode)
  - [msvc](#msvc)
  - [mingw](#mingw)
- [linux](#linux)

## Development Environment in WSL

### wsl basic cmds

```bash
# update wsl program itself, not distro
wsl --update

# set to wsl1
wsl --set-default-version 1
# set to wsl2
wsl --set-default-version 2
```

wsl uninstall distro

```powershell
# help info
wslconfig /h

# list all linux
wslconfig /l

# uninstall linux
wslconfig /u Debian
wslconfig /u Ubuntu
```

wsl export & import

```bash
# help info
wsl -h 

# show all WSL image names
wsl -l -v

# export WSL image to local file
wsl --export Fedora38 "D:\BackUp\Fedora38.tar"

# import local file to WSL image
wsl --import Fedora38 D:\IDE\Fedora38 "D:\BackUp\Fedora38.tar"

# shutdown
wsl --shutdown
```

### wsl with proxy

```bash
# wsl open proxy in current shell
vi ~/set_proxy.txt

# ~/set_proxy.txt
export http_proxy='http://192.168.0.108:7890'
export https_proxy='http://192.168.0.108:7890'
export all_proxy='socks5://192.168.0.108:7890'
export ALL_PROXY='socks5://192.168.0.108:7890'


source ~/set_proxy.txt
```

### wsl with git

config git proxy

```bash
# the config is saved in ~/.gitconfig
git config --global user.name "yourname"
git config --global user.email "yourmail"
git config --global http.proxy http://192.168.0.108:2080
```

or just change file `vi ~/.gitconfig`, then `git config --global --list`

```bash
[user]
        name = csc_fedora_gewei
        email = grey@pku.edu.cn
[http]
        proxy = http://192.168.0.108:2080
```

for csc to use `git clone ssh@xxxx.git`, change `vi ~/.ssh/config`, [solution](https://github.com/orgs/community/discussions/55269#discussioncomment-5901262)
- problem1: *ssh - port 22: Resource temporarily unavailable*
- problem2: *kex_exchange_identification: Connection closed by remote host*

```bash
# ~/.ssh/config
Host github.com
  HostName 20.200.245.248
  Port 443
```

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

Install vscode extenstion:
1. install [ms-cpp-tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools), then `"C_Cpp.intelliSenseEngine": "disabled",`
2. just install vscode extension [clangd](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd)

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
    "[cpp]": {
        "editor.defaultFormatter": "llvm-vs-code-extensions.vscode-clangd"
    },
    "C_Cpp.intelliSenseEngine": "disabled",
    "clangd.path": "clangd-16",
    "clangd.arguments": [
        "--clang-tidy",
    ],
    "[python]": {
        "editor.formatOnType": true,
        "editor.defaultFormatter": "ms-python.black-formatter"
    },
}
```

windows下的MinGw可以使用如下配置(不推荐MinGW)

```json
// settings.json in windows of MingW by gcc and llvm
{
  "cmake.cmakePath": "D:/Dev/cmake/bin/cmake.exe",
  "cmake.generator": "Ninja",
  "cmake.configureSettings": {
      "CMAKE_MAKE_PROGRAM": "D:/Dev/Ninja/ninja.exe"
  },
  "cmake.debugConfig": {
      "MIMode": "gdb",
      "miDebuggerPath": "D:/Dev/w64devkit/bin/gdb.exe"
  },
  // 给vscode提供其他compiler列表
  "cmake.additionalCompilerSearchDirs": [
      "D:/Dev/w64devkit/bin",
      "D:/Dev/llvm-mingw/bin",
      // "D:/Dev/winlibs-mingw64/bin",
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

in Windows:
1. download [llvm-mingw](https://github.com/mstorsjo/llvm-mingw/releases) or MSVC
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
BasedOnStyle: Google
IndentWidth: 4
ColumnLimit: 0
```

## Development Environment in CentOS7

1. download centos wsl [image](https://github.com/wsldl-pg/CentWSL/releases)
2. execute CentOS7.exe to install CentOS7

```bash
# user config: https://www.digitalocean.com/community/tutorials/how-to-add-and-delete-users-on-a-centos-7-server
# create user & set password
sudo adduser cauchy
sudo passwd cauchy
# delte user from /home
sudo userdel -r cauchy

# Granting Sudo Privileges to a User
sudo usermod -aG wheel cauchy
# Check sudo privilege, & uid
sudo lid -g wheel
# cauchy(uid=1000) 

# update repo
sudo yum update

# install devtoolset
sudo yum install centos-release-scl-rh
sudo yum install devtoolset-11

# temporary usage
scl enable devtoolset-11 bash
gcc --version
g++ --version

# permenantly usage
mv /usr/bin/gcc /usr/bin/gcc-4.8.5
ln -s /opt/rh/devtoolset-11/root/bin/gcc /usr/bin/gcc
mv /usr/bin/g++ /usr/bin/g++-4.8.5
ln -s /opt/rh/devtoolset-11/root/bin/g++ /usr/bin/g++
gcc --version
g++ --version
```

```bash
sudo yum install epel-release
sudo yum install cmake3
sudo ln -s /usr/bin/cmake3 /usr/bin/cmake
cmake --version

sudo yum install https://packages.endpointdev.com/rhel/7/os/x86_64/endpoint-repo.x86_64.rpm
sudo yum install git
git --version

sudo yum install openssl-devel
sudo yum install curl-devel
sudo yum install expat-devel
sudo yum install zlib-devel
```

wsl进入CentOS默认账号是root,需要切换成创建的账号, [method](https://github.com/microsoft/WSL/issues/4276#issuecomment-509364493)
1. get uid: `id -u cauchy`
2. 找到对应CentOS的注册表: `Computer\HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Lxss\{xxxx}`
3. 创建`REG_DWORD`,值为decimal `1000`


## Development Environment in Fedora38

update Fedora38 to Fedora Rawhide
- backup old wsl: `wsl --export Fedora38 "D:\BackUp\Fedora38.tar"`
- run below commands to update

```bash
#Be sure that systemd is disabled
cat /etc/wsl.conf

# sudo dnf install dnf-plugin-system-upgrade
sudo dnf -y system-upgrade --allowerasing --skip-broken download --releasever=rawhide  
sudo dnf -y system-upgrade reboot
# At this point, it will raise an error, ignore it, and execute the following commands:

sudo dnf -y system-upgrade upgrade   

sudo dnf -y autoremove   
sudo dnf -y clean all   
sudo mandb  
update.sh  
sudo dnf upgrade --refresh
cat /etc/fedora-release
exit
```

```bash
# 1. download and install Fedora 38, https://github.com/WhitewaterFoundry/Fedora-Remix-for-WSL
# 2. update
sudo dnf update

# cpp
sudo dnf install clang gdb git cmake ninja-build
# clangd
sudo dnf install clang-tools-extra

# rust
sudo dnf install rust cargo rust-src rustfmt

# golang
sudo dnf install golang

vi ~/.bashrc

PS1='\[\033[01;32m\]\u@\h\[\033[00m\]:\[\033[01;34m\]\w\[\033[00m\]\$ '
alias ll='ls -la'
```

```bash
# vi ~/.gitconfig
[user]
        name = BeFedora38
        email = grey@pku.edu.cn
[http]
        proxy = http://192.168.0.108:7890
```

change vscode settings

```json
{
    "cmake.cmakePath": "/usr/bin/cmake",
    "cmake.generator": "Ninja",
    "cmake.configureSettings": {
        "CMAKE_MAKE_PROGRAM": "/usr/bin/ninja"
    },
    "C_Cpp.intelliSenseEngine": "disabled",
    "C_Cpp.clang_format_fallbackStyle": "{ BasedOnStyle: Google, IndentWidth: 4, ColumnLimit: 0}",
    "C_Cpp.inlayHints.parameterNames.enabled": true,
    "C_Cpp.inlayHints.referenceOperator.enabled": true,
    "C_Cpp.inlayHints.autoDeclarationTypes.enabled": true,
    "[cpp]": {
        "editor.defaultFormatter": "llvm-vs-code-extensions.vscode-clangd"
    },
    "clangd.path": "clangd",
    "clangd.arguments": [
        "--clang-tidy",
    ],
}
```

## Development Environment in Fedora Rawhide

download image from [Fedora-WSL-RootFS](https://github.com/VSWSL/Fedora-WSL-RootFS)

```bash
# in powershell
wsl --import Rawhide D:\Dev\Rawhide D:\Downloads\rootfs.amd64.tar.gz

# into Rawhide bash
wsl -l -v
wsl -d Rawhide

# following tutorial to rawhide: https://docs.fedoraproject.org/en-US/quick-docs/upgrading-fedora-offline/
sudo dnf upgrade --refresh
sudo dnf install dnf-plugin-system-upgrade

# wsl1, replace gpgcheck=1 with gpgcheck=0
/etc/yum.repos.d/
vi fedora-cisco-openh264.repo
vi fedora-updates-testing.repo
vi fedora-updates.repo
vi fedora.repo

# change to rawhide channel
sudo dnf system-upgrade download --releasever=rawhide
sudo dnf system-upgrade reboot
# At this point, it will raise an error, ignore it, and execute the following commands:

sudo dnf -y system-upgrade upgrade
exit
wsl --shutdown
wsl -d Rawhide

sudo dnf -y autoremove
sudo dnf -y clean all
sudo mandb
sudo dnf upgrade --refresh
cat /etc/fedora-release

# add user
adduser moris
passwd moris
usermod -aG wheel moris
su - moris
sudo dnf update
```

prepare develepment environment

```bash
# for cpp
sudo dnf install clang gdb git cmake ninja-build clang-tools-extra


# for golang
sudo dnf install golang

# for python
sudo dnf install python3.10

# install vcpkg
git clone https://github.com/microsoft/vcpkg
./vcpkg/bootstrap-vcpkg.sh
vi ~/.bashrc

source $HOME/vcpkg/vcpkg/scripts/vcpkg_completion.bash
export PATH=$PATH:$HOME/vcpkg

PS1='\[\033[01;32m\]\u@\h\[\033[00m\]:\[\033[01;34m\]\w\[\033[00m\]\$ '
alias ll='ls -la'

# make softlink
ln -s /mnt/c/Users/moris winhome

# genenrate ssh-key
ssh-keygen -t ed25519 -C "your_email@example.com"
cat .ssh/id_ed25519.pub
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

## How to debug program with arguments in CMakeTools

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
            "name": "(gdb) Launch",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/proj1",
            "args": ["10", "20", "30"],
            "stopAtEntry": false,
            "cwd": "${fileDirname}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                },
                {
                    "description": "Set Disassembly Flavor to Intel",
                    "text": "-gdb-set disassembly-flavor intel",
                    "ignoreFailures": true
                }
            ]
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

### mingw

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

## linux

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