MIPT Distributed Systems Course programming assignments repository

Introduction
============

There are my solutions for Programming Exercises from MIPT Distributes Systems course. Written in C++/Java.

Collaboration Policy
==========

I opensourced this code because I believe it can help people learn something new and improve their skills.
You can use it on your conscience, but I encourage you not to copy-paste this sources and use
them only for educational purposes :)

Build Instructions
============================

Code succesfully builds on Archlinux and Ubuntu14.04

Dependencies:
  - g++4.8
  - boost
  - JDK-7
  - Apache Thrift
  - Maven
  
For quick test you can use my snapshot on terminal.com: https://www.terminal.com/tiny/TdAS98ytvc

To build on Ubuntu14.04:
```bash
  ./ubuntu_deps.sh
  ./build.sh
```

To run tests:
```bash
  ./run_coordinator_tests.sh
  ./run_server_tests.sh
```

To run Coordinator:
```bash
  ./build/coordinator 9090
```

To run Data Server:
```bash
  ./build/server 9091
```
