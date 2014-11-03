#!/bin/bash

mkdir build
cd build
cmake ..
make
cd ..
mvn -q package
