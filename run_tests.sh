#!/bin/bash

PORT=9090

echo "Running coordinator"
./build/coordinator $PORT &
SERVER_PID=$!
sleep 1

echo "Running tests"
java -cp build/java/storage-1.0-SNAPSHOT.jar mipt.distsys.storage.proxy.TestCoordinator $PORT

echo "Killing coordinator"
kill $SERVER_PID
