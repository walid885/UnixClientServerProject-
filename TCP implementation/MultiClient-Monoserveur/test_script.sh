#!/bin/bash

# Compile the programs
make clean
make all

# Start the server in background
./server &
SERVER_PID=$!

# Wait for server to start
sleep 1

# Test Case 1: Get current date and time
echo "Test Case 1: Get current date and time"
echo -e "user1\npass1\n1\n4" | ./client

# Test Case 2: List directory contents
echo "Test Case 2: List directory contents"
echo -e "user1\npass1\n2\n4" | ./client

# Test Case 3: Read file content
echo "Test Case 3: Read file content"
echo -e "user1\npass1\n3\nserveurTCP.c\n4" | ./client

# Kill the server
kill $SERVER_PID

echo "All tests completed"