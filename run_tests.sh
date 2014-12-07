#!/bin/bash

PORT=9090

echo "Running tests on port" $PORT
java -cp build/java/storage-1.0-SNAPSHOT.jar mipt.distsys.storage.proxy.TestCoordinator $PORT
