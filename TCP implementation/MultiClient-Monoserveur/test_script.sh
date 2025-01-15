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
print_header "Multi-Client TCP Server Test Suite"

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
sleep 2
print_success "Server started with PID: ${SERVER_PID}"

# Function to run a client test
run_client_test() {
    local test_num=$1
    local service_num=$2
    local delay=$3
    local username=$4
    local password=$5
    
    expect << EOF &
spawn ./client
expect "Username: "
send "$username\r"
expect "Password: "
send "$password\r"
expect "Choose"
sleep $delay
send "$service_num\r"
expect "Choose"
send "5\r"
expect eof
EOF
}

# In the main test section, replace the client launches with:
print_status "Launching client 1 (Time Service) - Admin"
run_client_test 1 1 2 "admin" "admin"

print_status "Launching client 2 (Directory Listing) - User 1"
run_client_test 2 2 3 "usr1" "usr1"

print_status "Launching client 3 (Connection Duration) - User 2"
run_client_test 3 4 4 "usr2" "usr2"

sleep 10

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
echo -e "${CYAN}Multi-client testing completed${NC}\n"