# Ubuntu 20.04

## Mandatory dependencies
On Ubuntu 20.04, run the following command to install required dependencies:
```bash
sudo apt install meson libreadline-dev gcc git
```

## Install Meson from Github releases page
1. Open https://github.com/mesonbuild/meson/releases
2. Download the latest release int the `.tar.gz` format
3. Decompress the archive somewhere in your computer
4. Create a symlink in a directory of your path to the `meson.py` file

## Install Criterion on Ubuntu
If you have Ubuntu 18.04 or Ubuntu 20.04, you may not have a `libcriterion` package in your distribution package manager. As a workaround, you may download and install manually these two packages:

* http://ppa.launchpad.net/snaipewastaken/ppa/ubuntu/pool/main/c/criterion/criterion-dev_2.3.2-6-ubuntu1~cosmic1_amd64.deb
* http://ppa.launchpad.net/snaipewastaken/ppa/ubuntu/pool/main/c/criterion/criterion_2.3.2-6-ubuntu1~cosmic1_amd64.deb