# WSL

- [WSL](#wsl)
  - [prepare wsl1 images](#prepare-wsl1-images)
    - [prepare dev environment](#prepare-dev-environment)

## prepare wsl1 images

how to prepare:
1. docker: most recommended
2. VMWare: not recommended
3. `dnf --installroot`

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

### prepare dev environment

```bash
wsl -d CentOS10
# add bash color
vi .bashrc
PS1='\[\033[01;32m\]\u@\h\[\033[00m\]:\[\033[01;34m\]\w\[\033[00m\]\$ '
alias ll='ls -la'

# add user
adduser tom
passwd tom
# add to sudo
usermod -aG wheel tom
# change default user in windows Registry
# Computer\HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Lxss\{} to decimal 1000

# update in new user
sudo dnf update
# default python3.12

# install cpp dev tools
dnf install clang clang-tools-extra cmake git
dnf --enablerepo=crb install ninja-build
```