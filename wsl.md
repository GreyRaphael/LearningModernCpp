# WSL

- [WSL](#wsl)
  - [prepare wsl1 images](#prepare-wsl1-images)
    - [prepare centos10 stream](#prepare-centos10-stream)
  - [wsl in windows](#wsl-in-windows)

## prepare wsl1 images

how to prepare:
1. docker: most recommended
2. VMWare: not recommended
3. `dnf --installroot`
4. download from github

### prepare centos10 stream

generate centos10-stream rootfs.tar.gz

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

# install cpp dev tools
dnf install clang clang-tools-extra cmake git
dnf --enablerepo=crb install ninja-build

# optional: lastest gcc
dnf search gcc-toolset
dnf install gcc-toolset-15-gcc
scl -l # list all
scl enable gcc-toolset-15 bash

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
    "cmake.configureSettings": {
        "CMAKE_TOOLCHAIN_FILE": "${env:HOME}/vcpkg/scripts/buildsystems/vcpkg.cmake",
    },
    "python.venvPath": "~/envs",
    "clangd.path": "clangd",
    "clangd.arguments": [
        "--clang-tidy"
    ],
}
```

## wsl in windows