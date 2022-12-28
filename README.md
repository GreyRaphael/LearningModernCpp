# Learning Modern C++

>  Learning Notes for Modern C++ 

- [Learning Modern C++](#learning-modern-c)
  - [Installation on Ubuntu](#installation-on-ubuntu)
  - [WSL Ubuntu](#wsl-ubuntu)
  - [Online Complier](#online-complier)
  - [Books](#books)

## Installation on Ubuntu

```bash
sudo apt update
sudo apt-get install build-essential gdb

gcc --version
g++ --version
gdb --version

sudo apt install cmake
sudo apt install ninja-build

cmake --version
ninja --version
```

```bash
# install locale
locale -a
# C
# C.utf8
# POSIX
# en_US.utf8
dpkg-reconfigure locales
# 空格键选择zh_CN.gbk
# 然后选择main display language: en_US.utf8

locale -a
# C
# C.utf8
# POSIX
# en_US.utf8
# zh_CN.gbk
```

## WSL Ubuntu

```bash
# wsl use proxy
# https://cloud.tencent.com/developer/article/1913248
export http_proxy='http://10.101.253.101:7890'
export https_proxy='http://10.101.253.101:7890'
export all_proxy='socks5://10.101.253.101:7890'
export ALL_PROXY='socks5://10.101.253.101:7890'

wget youtube.com
```

```bash
# wsl with conda
conda update --all
# WSL换源后使用conda出现CondaHTTPError: HTTP 000 CONNECTION FAILED for url <https://mirrors.tuna.tsinghua.edu.cn/anaconda/pkgs/main/linux-64/current_repodata.json>
# 但是 wget正常
wget https://mirrors.tuna.tsinghua.edu.cn/anaconda/pkgs/main/linux-64/current_repodata.json
# in cmd
wsl --shutdown
wsl -l -v
# 重启wsl
```

wsl with Clash: `source set_proxy.txt`

```bash
# set_proxy.txt
export http_proxy='http://10.101.253.101:7890'
export https_proxy='http://10.101.253.101:7890'
export all_proxy='socks5://10.101.253.101:7890'
export ALL_PROXY='socks5://10.101.253.101:7890'
```

## Online Complier

- [Wandbox](https://wandbox.org/)
- [Compiler Explorer](https://godbolt.org/)

## Books

- [Modern C++ Programming Cookbook - 2nd Edition](https://github.com/PacktPublishing/Modern-CPP-Programming-Cookbook-Second-Edition)
