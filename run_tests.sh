#!/bin/bash

echo "Running server"
# ./build/bin/CppServer 2> server_log &
./build/server &
SERVER_PID=$!
sleep 1

echo "Running tests"
java -cp build/java/storage-1.0-SNAPSHOT.jar:/usr/lib/libthrift-0.9.1.jar:slf4j-1.7.7/slf4j-api-1.7.7.jar:slf4j-1.7.7/slf4j-simple-1.7.7.jar mipt.distsys.storage.proxy.TestCoordinator

echo "Killing server"
kill $SERVER_PID
