#!/bin/bash

thrift -o build --gen cpp src/main/thrift/ReplicatedStorage.thrift
thrift -o build --gen java src/main/thrift/ReplicatedStorage.thrift
make
mvn clean install
