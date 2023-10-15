 Learning Modern C++

>  Learning Notes for Modern C++, examples from [Modern C++ Programming Cookbook - 2nd Edition](https://github.com/PacktPublishing/Modern-CPP-Programming-Cookbook-Second-Edition)

- [Development Environment in WSL](#development-environment-in-wsl)
  - [wslconfig in powershell](#wslconfig-in-powershell)
  - [wsl use windows proxy](#wsl-use-windows-proxy)
  - [wsl use conda](#wsl-use-conda)
- [Development Environment in Debian](#development-environment-in-debian)
  - [GCC \& Clang in VSCode](#gcc--clang-in-vscode)
  - [clangd for C++](#clangd-for-c)
- [Development Environment in CentOS7](#development-environment-in-centos7)
- [Development Environment in Fedora38](#development-environment-in-fedora38)
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

a proxy is for git, so just set git proxy is ok
> `git config --global http.proxy 10.101.253.101:7890`

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
sudo apt install clang-16
sudo apt install gdb
sudo apt install build-essential
sudo apt install git cmake ninja-build -y
# rust
sudo apt install rustc rust-src rustfmt
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

Debian11->testing problems
- [solution](https://github.com/microsoft/WSL/issues/10397#issuecomment-1682139166) for problem: `/etc/passwd lock: Invalid argument`

### GCC & Clang in VSCode

```bash
# 安装gcc, g++; testing默认是最高版本gcc
sudo apt install build-essential
# 安装clang-16
sudo apt install clang-16
# 安装gdb
sudo apt install gdb
```

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

vi ~/.bashrc
# export PS1='[\033[01;32m\]\u@\h\[\033[00m\]:\[\033[01;34m\]\w\[\033[00m\]\$'
# alias ll='ls -la'

vi ~/.gitconfig
# [user]
#         name = BeFedora38
#         email = grey@pku.edu.cn
# [http]
#         proxy = http://192.168.0.108:7890
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