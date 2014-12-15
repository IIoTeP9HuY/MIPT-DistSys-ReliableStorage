MIPT Distributed Systems Course programming assignments repository

Introduction
============

There are my solutions for Programming Exercises from MIPT Distributes Systems course. Written in C++/Java.

Code contains implementation of simple replicated data storage.

Collaboration Policy
==========

I opensourced this code because I believe it can help people learn something new and improve their skills.
You can use it on your conscience, but I encourage you not to copy-paste this sources and use
them only for educational purposes :)

Description
============================

System implements distributed replicated std::map<string, string>.

It consists of two components:
  - **Coordinator** - keeps track of data nodes, processes heartbeats, gives client information about current master node.
  - **Server** - data node, stores data, replicates it, processes client's requests to put and get data

During lifetime system has one Coordinator and several Servers.

To put/get data client talks to Coordinator to get address of current Master data node and then talks to corresponding data node with put/get requests.

Data nodes send heartbeats(pings) to Coordinator to get actual information about current Master and Backup data nodes.

Master node answers client requests and backups data to Backup node.

Backup node mirrors Master node's put activity but doesn't answer client's requests.

API
============================
  - **Coordinator**:
    - **ViewInfo ping(i32 viewNum, string name)** - *Called by Server nodes on heartbeat. Returns current state info.*
    - **string primary()** - *Returns current Master data node address.*
  
  - **Server**
    - **void put(string key, string value)** - *Called by client, puts pair (key, value) into data storage*
    - **void putBackup(string key, string value)** - *Called by Master on Backup, stores (key, value) pair on backup storage. Used for simplicity to distinguish put calls from client and put calls from Master.*
    - **string get(string key)** - *Called by client, returns value stored by key*

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
