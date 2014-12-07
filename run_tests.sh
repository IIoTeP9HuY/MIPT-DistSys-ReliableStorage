#!/bin/bash

echo "Running coordinator"
# ./build/bin/CppServer 2> server_log &
./build/coordinator &
SERVER_PID=$!
sleep 1

echo "Running tests"
java -cp build/java/storage-1.0-SNAPSHOT.jar mipt.distsys.storage.proxy.TestCoordinator

echo "Killing coordinator"
kill $SERVER_PID
