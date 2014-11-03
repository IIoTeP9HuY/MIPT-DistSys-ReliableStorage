#!/bin/bash

# Install deps
sudo apt-get update
sudo apt-get install cmake maven libthrift-java libboost-dev libboost-test-dev libboost-program-options-dev libboost-system-dev libboost-filesystem-dev libevent-dev automake libtool flex bison pkg-config g++ libssl-dev python-dev openjdk-7-jdk

# Install thrift
wget http://archive.apache.org/dist/thrift/0.9.1/thrift-0.9.1.tar.gz
tar -xf thrift-0.9.1.tar.gz
cd thrift-0.9.1
./configure
make
cd test/cpp/.libs
for i in ../*.o; do echo $i; ln -s $i .; done
cd -
make
sudo make install
