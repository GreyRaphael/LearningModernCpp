# WSL

- [WSL](#wsl)
  - [prepare wsl1 images](#prepare-wsl1-images)
    - [prepare centos10 stream](#prepare-centos10-stream)
    - [prepare centos7](#prepare-centos7)
  - [wsl basic cmds](#wsl-basic-cmds)

## prepare wsl1 images

how to prepare `rootfs.tar.gz`:
1. docker: most recommended
2. VMWare: not recommended
3. `dnf --installroot`
4. download from github

### prepare centos10 stream

generate centos10-stream `rootfs.tar.gz` in docker

```bash
# 1. pull image
docker pull dokken/centos-stream-10:latest
# 2. run image
docker run -it --name centos10_wsl_build dokken/centos-stream-10:latest /bin/bash

# 3. update
dnf update -y
# 4. install necessary tools
dnf install -y passwd sudo vim iputils ncurses
# 5. clean cache
dnf clean all
# 6.exit
exit

# 7.export rootfs.tar.gz
docker ps -a
docker export centos10_wsl_build > centos10-stream-rootfs.tar
gzip centos10-stream-rootfs.tar

# 8. import rootfs.tar.gz in windows
wsl --import CentOS10 D:\WSL\CentOS10 D:\WSL_Images\centos10-stream-rootfs.tar.gz
wsl -d CentOS10
```

docker cmds

```bash
# docker进入已经存在的容器(attach & interactivie)
docker start -ai centos10_wsl_build
# 如果容器已经在后台运行
docker exec -it centos10_wsl_build /bin/bash
```

prepare centos10-stream development environment

```bash
# enter with root
wsl -d CentOS10

# add user
adduser tom
# # delte old user
# userdel -r cauchy
passwd tom
# add to sudo
usermod -aG wheel tom
# check user
su - tom

wsl --terminate CentOS10
# change default user in windows Registry
# Computer\HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Lxss\{} to decimal 1000

wsl -d CentOS10
# update in new user
sudo dnf update
# default python3.12

# add bash color
vi .bashrc
PS1='\[\033[01;32m\]\u@\h\[\033[00m\]:\[\033[01;34m\]\w\[\033[00m\]\$ '
alias ll='ls -la'

# make softlink
ln -s /mnt/c/Users/moris winhome
```

cpp dev environment

```bash
# install cpp dev tools
dnf install clang clang-tools-extra cmake git
dnf --enablerepo=crb install ninja-build

# optional: lastest gcc
dnf search gcc-toolset
dnf install gcc-toolset-15-gcc
scl -l # list all
scl enable gcc-toolset-15 bash
```

vcpkg config

```bash
vi .bashrc

export VCPKG_FORCE_SYSTEM_BINARIES=1
export VCPKG_ROOT=$HOME/vcpkg
export PATH=$VCPKG_ROOT:$PATH

git clone https://github.com/microsoft/vcpkg
./vcpkg/bootstrap-vcpkg.sh

# vi vcpkg/scripts/buildsystems/vcpkg.cmake add to last line
set(CMAKE_CXX_STANDARD 20)
# vi vcpkg/triplets/x64-linux.cmake add to last line
set(VCPKG_BUILD_TYPE release)
```

```json
// vscode setting.json
{
    "cmake.cmakePath": "/usr/bin/cmake",
    "cmake.generator": "Ninja",
    "cmake.configureSettings": {
        "CMAKE_TOOLCHAIN_FILE": "${env:HOME}/vcpkg/scripts/buildsystems/vcpkg.cmake",
        "CMAKE_MAKE_PROGRAM": "/usr/bin/ninja",
    },
    "python.venvPath": "~/envs",
    "clangd.path": "clangd",
    "clangd.arguments": [
        "--clang-tidy"
    ],
}
```

go, rust, python dev environment

```bash
# golang
sudo dnf install golang
# rust
sudo dnf install rust cargo rust-src rustfmt
# for python with uv
curl -LsSf https://astral.sh/uv/install.sh | sh
uv python list
```

git proxy, just change file `vi ~/.gitconfig`, then `git config --global --list`

```bash
[user]
        name = csc_fedora_gewei
        email = grey@pku.edu.cn
[http]
        proxy = http://127.0.0.1:2080
```

ssh public key

```bash
# genenrate ssh-key
ssh-keygen -t ed25519 -C "your_email@example.com"
cat .ssh/id_ed25519.pub
```

for csc to use `git clone ssh@xxxx.git`, change `vi ~/.ssh/config`, [solution](https://github.com/orgs/community/discussions/55269#discussioncomment-5901262)
- problem1: *ssh - port 22: Resource temporarily unavailable*
- problem2: *kex_exchange_identification: Connection closed by remote host*

```bash
sudo dnf install ncat
which ncat

vi ~/.ssh/config
Host github.com
  HostName 20.200.245.248
  Port 443
  #ProxyCommand nc -X connect -x 127.0.0.1:2080 %h %p
  ProxyCommand ncat --proxy 127.0.0.1:2080 --proxy-type socks5 %h %p
```

对于win10的git使用代理

```bash
# %USERPROFILE%/.ssh/config
# connect在git目录，如果是MinGit, 直接从github release重新下载一份放到环境变量即可
Host Tencent
  HostName xxx.xxx.xxx.xxx
  User gewei
  ProxyCommand connect -S 127.0.0.1:2080 %h %p
```

optional: add dnf proxy

```bash
cat /etc/dnf/dnf.conf

[main]
gpgcheck=1
installonly_limit=3
clean_requirements_on_remove=True
best=True
skip_if_unavailable=False
# add this line
proxy=http://127.0.0.1:2080
```

bash proxy

```bash
# wsl open proxy in current shell
vi ~/set_proxy.txt

# ~/set_proxy.txt
export http_proxy='http://127.0.0.1:2080'
export https_proxy='http://127.0.0.1:2080'
export all_proxy='socks5://127.0.0.1:2080'
export ALL_PROXY='socks5://127.0.0.1:2080'

source ~/set_proxy.txt
```

### prepare centos7

generate centos7 `rootfs.tar.gz`

```bash
# clear proxy in host
export http_proxy=""
export https_proxy=""
export HTTP_PROXY=""
export HTTPS_PROXY=""

docker pull centos:7
docker run -it --name centos7_wsl_build centos:7 /bin/bash

cd /etc/yum.repos.d/
rm *repo
# change repo to aliyun
curl -o /etc/yum.repos.d/CentOS-Base.repo HTTPS://mirrors.aliyun.com/repo/Centos-7.repo

yum update
# install epel
yum instal epel-release
yum update
yum install -y net-tools iproute vi sudo passwd

yum update
yum clean all
exit

docker export centos7_wsl_build > centos7-rootfs.tar
gzip centos7-rootfs.tar
```

```bash
# install scl source for devtoolset-11
yum install centos-release-scl
# edit /etc/yum.repos.d/CentOS-SCLo-scl-rh.repo
[centos-sclo-rh]
name=CentOS-7 - SCLo rh
# edit here
baseurl=https://mirrors.aliyun.com/centos/7/sclo/x86_64/rh/

# edit /etc/yum.repos.d/CentOS-SCLo-scl.repo
[centos-sclo-sclo]
name=CentOS-7 - SCLo sclo
# edit here
baseurl=https://mirrors.aliyun.com/centos/7/sclo/x86_64/rh/
```

prepare user

```bash
wsl --import CentOS7 D:\WSL\CentOS7 D:\WSL_Images\centos7-rootfs.tar
# enter as root
wsl -d CentOS7

adduser tom
passwd tom
# delete old user
userdel -r cauchy
# add to sudoer
usermod -aG wheel tom
lid -g wheel
# tom(uid=1000) 
```

cpp dev

```bash
yum update

yum install git ninja-build
yum install rh-python38

# install devtoolset
yum install centos-release-scl-rh
yum install devtoolset-11

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

# centos7最大可以编译clang-19
```

## wsl basic cmds

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