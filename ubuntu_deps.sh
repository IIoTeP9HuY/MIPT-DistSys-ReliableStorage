#!/bin/bash

PROJECT_DIR=/tmp/hw

mkdir -p $PROJECT_DIR
cd $PROJECT_DIR
cp /tmp/hw.tar.bz2 $PROJECT_DIR
tar -xf hw.tar.bz2

sudo apt-get update
sudo apt-get install cmake maven libthrift-java
sudo apt-get install libboost-dev libboost-test-dev libboost-program-options-dev libboost-system-dev libboost-filesystem-dev libevent-dev automak
e libtool flex bison pkg-config g++ libssl-dev
sudo apt-get install python-dev
sudo apt-get install openjdk-7-jdk

cd /tmp
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

cd $PROJECT_DIR
./build.sh
./run_tests.sh