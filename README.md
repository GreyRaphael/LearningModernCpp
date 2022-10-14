# Learning Modern C++

>  Learning Notes for Modern C++ 

## Installation on Ubuntu

```bash
sudo apt update
sudo apt-get install build-essential
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