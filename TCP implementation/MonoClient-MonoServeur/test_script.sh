#!/bin/bash

# Compile the programs
make clean
make all

# Start the server in background
./server &
SERVER_PID=$!

# Wait for server to start
sleep 1

# Test case 1: Get system time
echo "Test 1: Get system time"
expect << EOF
spawn ./client
expect "Username: "
send "admin\r"
expect "Password: "
send "admin\r"
expect "Choose service"
send "1\r"
expect "Choose service"
send "5\r"
expect eof
EOF

sleep 1

# Test case 2: List directory
echo "Test 2: List directory"
expect << EOF
spawn ./client
expect "Username: "
send "admin\r"
expect "Password: "
send "admin\r"
expect "Choose service"
send "2\r"
expect "Choose service"
send "5\r"
expect eof
EOF

sleep 1

# Test case 3: Get connection duration
echo "Test 3: Connection duration"
expect << EOF
spawn ./client
expect "Username: "
send "admin\r"
expect "Password: "
send "admin\r"
expect "Choose service"
sleep 3

send "4\r"


expect "Choose service"
send "5\r"
expect eof
EOF

# Kill the server
kill $SERVER_PID

echo "All tests completed"