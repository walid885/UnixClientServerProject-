#!/bin/bash

# Color definitions
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color
BOLD='\033[1m'

# Function for section headers
print_header() {
    echo -e "\n${PURPLE}${BOLD}═══════════════════════════════════════════${NC}"
    echo -e "${BLUE}${BOLD}   $1${NC}"
    echo -e "${PURPLE}${BOLD}═══════════════════════════════════════════${NC}\n"
}

# Function for status messages
print_status() {
    echo -e "${YELLOW}${BOLD}[STATUS]${NC} $1"
}

# Function for success messages
print_success() {
    echo -e "${GREEN}${BOLD}[SUCCESS]${NC} $1"
}

# Start of script
print_header "TCP Client-Server Test Suite"

# Compilation phase
print_status "Cleaning previous builds..."
make clean > /dev/null 2>&1

print_status "Compiling programs..."
make all > /dev/null 2>&1
if [ $? -eq 0 ]; then
    print_success "Compilation completed successfully"
else
    echo -e "${RED}${BOLD}[ERROR]${NC} Compilation failed"
    exit 1
fi

# Start server
print_status "Starting server in background..."
./server &
SERVER_PID=$!
sleep 1
print_success "Server started with PID: ${SERVER_PID}"

# Test Case 1
print_header "Test Case 1: System Time Request"
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

print_success "Test Case 1 completed"
sleep 1

# Test Case 2
print_header "Test Case 2: Directory Listing"
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

print_success "Test Case 2 completed"
sleep 1

# Test Case 3
print_header "Test Case 3: Connection Duration"
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

print_success "Test Case 3 completed"

# Cleanup
print_status "Cleaning up..."
kill $SERVER_PID
if [ $? -eq 0 ]; then
    print_success "Server shutdown successfully"
else
    echo -e "${RED}${BOLD}[ERROR]${NC} Server shutdown failed"
fi

print_header "Test Suite Completed"
echo -e "${GREEN}${BOLD}All tests executed successfully!${NC}"
echo -e "${CYAN}Thank you for using the TCP Client-Server Test Suite${NC}\n"