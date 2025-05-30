 Learning Modern C++

>  Learning Notes for Modern C++, examples from [Modern C++ Programming Cookbook - 2nd Edition](https://github.com/PacktPublishing/Modern-CPP-Programming-Cookbook-Second-Edition)

- [Development Environment in WSL](#development-environment-in-wsl)
  - [wsl basic cmds](#wsl-basic-cmds)
  - [wsl with proxy](#wsl-with-proxy)
  - [wsl with git](#wsl-with-git)
  - [disable windows path](#disable-windows-path)
- [Development Environment in Debian](#development-environment-in-debian)
  - [GCC \& Clang in VSCode](#gcc--clang-in-vscode)
  - [clangd for C++](#clangd-for-c)
- [Development Environment in CentOS7](#development-environment-in-centos7)
  - [Build GCC-13 from source](#build-gcc-13-from-source)
- [Development Environment in CentOS9 stream](#development-environment-in-centos9-stream)
- [Development Environment in CentOS10 stream](#development-environment-in-centos10-stream)
- [Development Environment in Fedora Rawhide](#development-environment-in-fedora-rawhide)
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

## Development Environment in WSL

### wsl basic cmds

```bash
# update wsl itself, not distro
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

# uninstall linux
wslconfig /u Debian
wslconfig /u Ubuntu
```

wsl export & import

```bash
# help info
wsl --help

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

config git proxy, just change file `vi ~/.gitconfig`, then `git config --global --list`

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
# in Fedora 40
sudo dnf install netcat

# ~/.ssh/config
Host github.com
  HostName 20.200.245.248
  Port 443
  #ProxyCommand nc -X connect -x 127.0.0.1:2080 %h %p  
```

```bash
# in CentOS9 Stream
sudo dnf install nmap-ncat

# ~/.ssh/config
Host github.com
  HostName 20.200.245.248
  Port 443
  #ProxyCommand ncat --proxy 127.0.0.1:2080 --proxy-type socks5 %h %p
```

change dnf proxy

```conf
# /etc/dnf/dnf.conf
[main]
gpgcheck=True
installonly_limit=3
clean_requirements_on_remove=True
best=False
skip_if_unavailable=True
max_parallel_downloads=10
fastestmirror=True
defaultyes=True
# add this line
proxy=http://127.0.0.1:2080
```

### disable windows path

```bash
sudo vi /etc/wsl.conf

# /etc/wsl.conf
[interop]
appendWindowsPath = false

# restart wsl
wsl --shutdown
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
# or vi .bashrc
export PATH=/opt/rh/devtoolset-11/root/bin/:$PATH
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

# config python3.8
sudo yum install rh-python38
scl enable rh-python38 bash
python3 --version
python3 -m venv jupy8
source jupy8/bin/activate
```

wsl进入CentOS默认账号是root,需要切换成创建的账号, [method](https://github.com/microsoft/WSL/issues/4276#issuecomment-509364493)
1. get uid: `id -u cauchy`
2. 找到对应CentOS的注册表: `Computer\HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Lxss\{xxxx}`
3. 创建`REG_DWORD`,值为decimal `1000`


### Build GCC-13 from source

> not recommended, there are many problems

```bash
# GMP (GNU Multiple Precision Arithmetic Library)
# MPFR (Multiple Precision Floating-Point Reliable Library)
# MPC (Multiple Precision Complex Library)
# ISL (Integer Set Library)
sudo yum install gmp-devel mpfr-devel libmpc-devel

sudo yum install wget
wget https://ftp.gnu.org/gnu/gcc/gcc-13.3.0/gcc-13.3.0.tar.xz
tar -xJf gcc-13.3.0.tar.xz
cd gcc-13.3.0

mkdir build
cd build

../configure --prefix=/usr/local/gcc-13 --enable-languages=c,c++ --disable-multilib
# --disable-multilib: This option is often necessary on a 64-bit system to avoid building both 32-bit and 64-bit files.

# Compiles using all available cores
make -j $(nproc)
sudo make install

# post-build: vi .bashrc
export PATH=/usr/local/gcc-13/bin:$PATH
export LD_LIBRARY_PATH=/usr/local/gcc-13/lib64:$LD_LIBRARY_PATH

# check version
gcc --version
```

## Development Environment in CentOS9 stream

download image from [CentOS 9-stream](https://github.com/mishamosher/CentOS-WSL/releases/)

```bash
dnf update
# default python3.9

# extra install gcc 11
dnf install clang clang-tools-extra cmake git gdb
dnf --enablerepo=crb install ninja-build

sudo dnf install gcc-toolset-13-gcc
scl -l # list all
scl enable gcc-toolset-13 bash

dnf install golang

# vi /etc/wsl.conf
[interop]
appendWindowsPath = false
# restart wsl in windows
wsl --terminate CentOS9

# add sudo user
adduser frank
passwd frank
# userdel -r cauchy
usermod -aG wheel frank
lid -g wheel 
# change Computer\HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Lxss\{} to decimal 1000

su - frank
vi .bashrc
PS1='\[\033[01;32m\]\u@\h\[\033[00m\]:\[\033[01;34m\]\w\[\033[00m\]\$ '
alias ll='ls -la'

export VCPKG_FORCE_SYSTEM_BINARIES=1
export VCPKG_ROOT=$HOME/vcpkg
export CAPNPROTO_TOOLS=$VCPKG_ROOT/installed/x64-linux/tools/capnproto
export PATH=$VCPKG_ROOT:$CAPNPROTO_TOOLS:$PATH

git clone https://github.com/microsoft/vcpkg
./vcpkg/bootstrap-vcpkg.sh
```

```bash
# vi vcpkg/scripts/buildsystems/vcpkg.cmake add to last line
set(CMAKE_CXX_STANDARD 20)
# vi vcpkg/triplets/x64-linux.cmake add to last line
set(VCPKG_BUILD_TYPE release)
```

```json
// vscode setting.json
{
    "cmake.configureSettings": {
        "CMAKE_TOOLCHAIN_FILE": "${env:HOME}/vcpkg/scripts/buildsystems/vcpkg.cmake",
    },
    "python.venvPath": "~/pyenvs",
    "clangd.path": "clangd",
    "clangd.arguments": [
        "--clang-tidy"
    ],
}
```

## Development Environment in CentOS10 stream

How to make your own centos10 rootfs.tar.gz for wsl
- download [boot.iso](https://mirror.stream.centos.org/10-stream/BaseOS/x86_64/iso/CentOS-Stream-10-latest-x86_64-boot.iso)
- install centos in VMWare17 by boot.iso(**Minimal**), and create a *root* user, connect to wsl by ssh in vmware17 meanu: `ssh root@xxx.xxx.xxx.xxx`
- `cd /` and `tar --exclude=/dev --exclude=/proc --exclude=/sys --exclude=/mnt --exclude=/media --exclude=/run --exclude=/tmp --exclude=/rootfs.tar.gz  -czvf rootfs.tar.gz /`
- transfer rootfs.tar.gz out of vmware17: `scp root@xxx.xxx.xxx.xxx:/rootfs.tar.gz .`
- in windows, wsl import rootfs.tar.gz: `wsl --import <DistroName> <InstallLocation> rootfs.tar.gz`

```bash
# login as root
cd /
mkdir tmp
chmod 1777 /tmp

dnf update
# default python3.12

# install gcc automatically
dnf install clang clang-tools-extra cmake git gdb
dnf --enablerepo=crb install ninja-build

# vi /etc/wsl.conf
[interop]
appendWindowsPath = false

# restart wsl in windows
wsl --terminate CentOS10

# create user
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

```bash
# /etc/yum.repo.d/fedora.repo for wsl
[fedora]
name=Fedora $releasever - $basearch - aliyun
failovermethod=priority
baseurl=http://mirrors.aliyun.com/fedora/development/rawhide/Everything/$basearch/os/
#mirrorlist=https://mirrors.fedoraproject.org/metalink?repo=fedora-$releasever&arch=$basearch
enabled=1
metadata_expire=7d
gpgcheck=0
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-fedora-$basearch

[fedora-debuginfo]
name=Fedora $releasever - $basearch - Debug - aliyun
failovermethod=priority
baseurl=http://mirrors.aliyun.com/fedora/development/rawhide/Everything/$basearch/debug/
#mirrorlist=https://mirrors.fedoraproject.org/metalink?repo=fedora-debug-$releasever&arch=$basearch
enabled=0
metadata_expire=7d
gpgcheck=0
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-fedora-$basearch

[fedora-source]
name=Fedora $releasever - Source - aliyun
failovermethod=priority
baseurl=http://mirrors.aliyun.com/fedora/development/rawhide/Everything/source/SRPMS/
#mirrorlist=https://mirrors.fedoraproject.org/metalink?repo=fedora-source-$releasever&arch=$basearch
enabled=0
metadata_expire=7d
gpgcheck=0
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-fedora-$basearch
```

```bash
# /etc/yum.repo.d/fedora.repo for aliyun
[fedora]
name=Fedora $releasever - $basearch - aliyun
failovermethod=priority
baseurl=http://mirrors.cloud.aliyuncs.com/fedora/development/rawhide/Everything/$basearch/os/
#mirrorlist=https://mirrors.fedoraproject.org/metalink?repo=fedora-$releasever&arch=$basearch
enabled=1
metadata_expire=7d
gpgcheck=0
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-fedora-$basearch

[fedora-debuginfo]
name=Fedora $releasever - $basearch - Debug - aliyun
failovermethod=priority
baseurl=http://mirrors.cloud.aliyuncs.com/fedora/development/rawhide/Everything/$basearch/debug/
#mirrorlist=https://mirrors.fedoraproject.org/metalink?repo=fedora-debug-$releasever&arch=$basearch
enabled=0
metadata_expire=7d
gpgcheck=0
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-fedora-$basearch

[fedora-source]
name=Fedora $releasever - Source - aliyun
failovermethod=priority
baseurl=http://mirrors.cloud.aliyuncs.com/fedora/development/rawhide/Everything/source/SRPMS/
#mirrorlist=https://mirrors.fedoraproject.org/metalink?repo=fedora-source-$releasever&arch=$basearch
enabled=0
metadata_expire=7d
gpgcheck=0
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-fedora-$basearch
```

prepare develepment environment

```bash
# for cpp
sudo dnf install clang gdb git cmake ninja-build clang-tools-extra

# for golang
sudo dnf install golang

# rust
sudo dnf install rust cargo rust-src rustfmt

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

vscode **settings.json** for rawhide

```json
{
    "cmake.cmakePath": "/usr/bin/cmake",
    "cmake.generator": "Ninja",
    "cmake.configureSettings": {
        "CMAKE_MAKE_PROGRAM": "/usr/bin/ninja",
        "CMAKE_TOOLCHAIN_FILE": "${env:HOME}/vcpkg/scripts/buildsystems/vcpkg.cmake",
    },
    "python.venvPath": "~/pyenvs",
    "clangd.path": "clangd",
    "clangd.arguments": [
        "--clang-tidy"
    ],
}
```

backup wsl: `wsl --export Rawhide "D:\BackUp\Rawhide.tar"`

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

```bash
wget https://github.com/llvm/llvm-project/releases/download/llvmorg-15.0.7/llvm-project-15.0.7.src.tar.xz
tar -xvf *.xz

cd llvm-project-15.0.7.src
mkdir build && cd build
cmake -G "Unix Makefiles" -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" -DCMAKE_INSTALL_PREFIX=~/tools -DCMAKE_BUILD_TYPE=Release ../llvm
make -j 16
make install

# check
~/tools/bin/clangd --version

vi .bashrc
export PATH=$HOME/tools/bin:$PATH
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