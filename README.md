 Learning Modern C++

>  Learning Notes for Modern C++, examples from [Modern C++ Programming Cookbook - 2nd Edition](https://github.com/PacktPublishing/Modern-CPP-Programming-Cookbook-Second-Edition)

- [Development Environment in WSL](#development-environment-in-wsl)
  - [wslconfig in powershell](#wslconfig-in-powershell)
  - [wsl use windows proxy](#wsl-use-windows-proxy)
  - [wsl use conda](#wsl-use-conda)
- [Development Environment in Debian](#development-environment-in-debian)
  - [GCC \& Clang in VSCode](#gcc--clang-in-vscode)
  - [LLDB in VSCode](#lldb-in-vscode)
    - [in linux](#in-linux)
    - [in Windows](#in-windows)
- [Development Environment Online](#development-environment-online)
- [Othre configuration](#othre-configuration)
  - [linux locale config](#linux-locale-config)

## Development Environment in WSL

### wslconfig in powershell

```powershell
# help info
wslconfig /h

# list all linux
wslconfig /l

# uninstall linux
wslconfig /u Debian
wslconfig /u Ubuntu
```

### wsl use windows proxy

```bash
# wsl open proxy in current shell
vi ~/set_proxy.txt


# ~/set_proxy.txt
export http_proxy='http://10.101.253.101:7890'
export https_proxy='http://10.101.253.101:7890'
export all_proxy='socks5://10.101.253.101:7890'
export ALL_PROXY='socks5://10.101.253.101:7890'


source ~/set_proxy.txt
```

### wsl use conda

```bash
# 1. Download Miniconda3-latest-Linux-x86_64.sh & Install
wget https://mirrors.tuna.tsinghua.edu.cn/anaconda/miniconda/Miniconda3-latest-Linux-x86_64.sh

chmod +x Miniconda3-latest-Linux-x86_64.sh

./Miniconda3-latest-Linux-x86_64.sh

# 2.change conda source
# visit https://mirrors.tuna.tsinghua.edu.cn/help/anaconda/

conda update --all
# WSL换源后使用conda出现CondaHTTPError: HTTP 000 CONNECTION FAILED for url <https://mirrors.tuna.tsinghua.edu.cn/anaconda/pkgs/main/linux-64/current_repodata.json>
# 但是 wget正常
wget https://mirrors.tuna.tsinghua.edu.cn/anaconda/pkgs/main/linux-64/current_repodata.json
# in cmd
wsl --shutdown
wsl -l -v
# 重启wsl
```

## Development Environment in Debian

> 实机和wsl均可这么配置，下文以WSL为例

1. Download [wsl debian11 image](https://learn.microsoft.com/en-us/windows/wsl/install-manual), and install(直接解压，然后以Administrator权限运行debian.exe)
2. update [Debian11 to Debian12](https://www.makeuseof.com/upgrade-to-debian-12-bookworm-from-debian-11/)
3. Install development environment

```bash
# 1. Download image & Install
# 2. Debain11 -> Debian12
sudo apt update
sudo apt install apt-transport-https ca-certificates

# 使用https://mirrors.tuna.tsinghua.edu.cn/help/debian/的Debian12源
sudo vi /etc/apt/sources.list
sudo apt update && sudo apt upgrade -y

# 在powershell中wslconfig /t Debian,然后重新进入wsl

# 检查版本是否是12
cat /etc/debian_version

sudo apt --purge autoremove -y

# 3. Install development environment
sudo apt install clang-15
sudo apt install gdb
sudo apt install build-essential
sudo apt install git cmake ninja-build -y
```

config git proxy

```bash
# the config is saved in ~/.gitconfig
git config --global user.name "yourname"
git config --global user.email "yourmail"
git config --global http.proxy http://192.168.0.108:7890
```

Debian11->12 problems
- [solution](https://github.com/microsoft/WSL/issues/4279#issuecomment-1639165782) for problem: `mv: cannot move '/lib/x86_64-linux-gnu/security' to '/usr/lib/x86_64-linux-gnu/security': Permission denied`

### GCC & Clang in VSCode

```bash
# 安装gcc, g++, Debian12默认gcc是version 12
sudo apt install build-essential
# 安装clang-15
sudo apt install clang-15
# 安装gdb
sudo apt install gdb

# 如果安装更高版本gcc13, 先add source，然后使用proxy安装
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo http_proxy=http://192.168.0.108:7890 apt install gcc-13
```

Install vscode extenstion:
- method1: install **C/C++ Extension Pack** in vscode extensions(not recommended), 
- method2: just install vscode extension [clangd](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd)

config `setttings.json` in linux
> gcc-12和clang-15都能使用gdb来调试

```json
// setings.json in linux
{
    "cmake.cmakePath": "/usr/bin/cmake",
    "cmake.generator": "Ninja",
    "cmake.configureSettings": {
        "CMAKE_MAKE_PROGRAM": "/usr/bin/ninja"
    },
    "cmake.debugConfig": {
        "MIMode": "gdb",
        "miDebuggerPath": "/usr/bin/gdb"
    }
}
```

windows下的MinGw可以使用如下配置

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

### LLDB in VSCode

> 在Linux下需要提前编译lldb-mi，在windows下使用llvm-mingw自带的lldb-mi.exe即可

#### in linux

```bash
sudo apt install liblldb-15-dev

git clone https://github.com/lldb-tools/lldb-mi.git
cd lldb-mi
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE:STRING=Release
cmake --build .
cmake --install .
whereis lldb-mi
# lldb-mi: /usr/bin/lldb-mi /usr/local/bin/lldb-mi
```

```json
// settings.json
{
    "cmake.debugConfig": {
        "MIMode": "lldb",
        "miDebuggerPath": "/usr/bin/lldb-mi"
    },
}
```

1. Install [CodeLLDB](https://marketplace.visualstudio.com/items?itemName=vadimcn.vscode-lldb) extensions
2. Add `launch.json`
3. Add **breakpoints** and debug

```json
// launch.json
{
    "version": "0.2.0",
    "configurations": [
        {
            "type": "lldb",
            "request": "launch",
            "name": "Launch",
            "program": "${workspaceFolder}/build/proj1",
            "args": [],
            "cwd": "${workspaceFolder}/build"
        }

    ]
}
```

#### in Windows


1. Install [CodeLLDB](https://marketplace.visualstudio.com/items?itemName=vadimcn.vscode-lldb) extensions
2. Add `launch.json`
3. Add **breakpoints** and debug

```json
// launch.json in windows
{
    "version": "0.2.0",
    "configurations": [
        {
            "type": "lldb",
            "request": "launch",
            "name": "Win-Debug",
            "program": "${workspaceFolder}/build/bin/proj1",
            "args": [],
            "cwd": "${workspaceFolder}/build/bin",
            "env": {
                // lldb-mi.exe必须在下面PATH
                "PATH": "D:/Dev/llvm-mingw/bin"
            }
        }
    ]
}
```

```json
// 或者直接修改settings.json
{
    "cmake.cmakePath": "/usr/bin/cmake",
    "cmake.generator": "Ninja",
    "cmake.configureSettings": {
        "CMAKE_MAKE_PROGRAM": "/usr/bin/ninja"
    },
    "cmake.debugConfig": {
        "MIMode": "lldb",
        "miDebuggerPath": "D:/Dev/llvm-mingw/bin/lldb-mi.exe"
    }
}
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