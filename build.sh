#!/bin/bash

cd build
cmake ..
make
cd ..
mvn -q package
