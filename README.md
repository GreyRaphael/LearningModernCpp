 Learning Modern C++

>  Learning Notes for Modern C++, examples from [Modern C++ Programming Cookbook - 2nd Edition](https://github.com/PacktPublishing/Modern-CPP-Programming-Cookbook-Second-Edition)

- [Development Environment in WSL](#development-environment-in-wsl)
  - [wslconfig in powershell](#wslconfig-in-powershell)
  - [wsl use windows proxy](#wsl-use-windows-proxy)
  - [wsl use conda](#wsl-use-conda)
- [Development Environment in Debian](#development-environment-in-debian)
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