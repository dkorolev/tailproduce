## Setup, checkout, build and run tests

In Ubuntu 14.04.

```
sudo apt-get update
sudo apt-get install -y git binutils make cmake libgtest-dev libgoogle-glog-dev libboost1.55-dev libboost-system1.55-dev libboost-filesystem1.55-dev clang-3.5
(cd /usr/src/gtest; sudo cmake . ; sudo make)
git clone https://github.com/dkorolev/tailproduce.git
cd tailproduce
make love
```
