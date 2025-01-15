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

# Function for error messages
print_error() {
    echo -e "${RED}${BOLD}[ERROR]${NC} $1"
}

# Create test files and directories
setup_test_environment() {
    print_status "Setting up test environment..."
    mkdir -p test_dir
    echo "This is a test file" > test_dir/test.txt
    echo "Another test file" > test_dir/another.txt
    print_success "Test environment created"
}

# Function to run a client test
run_client_test() {
    local test_num=$1
    local service_num=$2
    local delay=$3
    local username=$4
    local password=$5
    local extra_input=$6
    
    expect -f - << EOF > client_output_${test_num}.log 2>&1 &
spawn ./client
expect "Username: "
send "$username\r"
expect "Password: "
send "$password\r"
expect "Choose"
sleep $delay
send "$service_num\r"
if { "$service_num" == "3" } {
    expect "Enter filename:"
    send "$extra_input\r"
}
sleep 1
expect "Choose"
send "5\r"
expect eof
EOF
    echo $!
}

# Start of script
print_header "Multi-Client TCP Server Test Suite"

# Compilation phase
print_status "Cleaning previous builds..."
rm -f server client *.o *.log
rm -rf test_dir

print_status "Compiling programs..."
gcc -o server MultiServer.c -pthread
gcc -o client Multiclient.c

if [ $? -eq 0 ]; then
    print_success "Compilation completed successfully"
else
    print_error "Compilation failed"
    exit 1
fi

# Setup test environment
setup_test_environment

# Start server
print_status "Starting server in background..."
./server &
SERVER_PID=$!
sleep 2

if ps -p $SERVER_PID > /dev/null; then
    print_success "Server started with PID: ${SERVER_PID}"
else
    print_error "Server failed to start"
    exit 1
fi

# Run multiple client tests simultaneously
print_status "Starting multi-client tests..."

# Test 1: Get system time (admin user)
CLIENT1_PID=$(run_client_test 1 1 2 "admin" "admin")
print_status "Client 1 (Time Service) launched - PID: $CLIENT1_PID"

# Test 2: List directory (user1)
CLIENT2_PID=$(run_client_test 2 2 3 "usr1" "usr1")
print_status "Client 2 (Directory Listing) launched - PID: $CLIENT2_PID"

# Test 3: Read file (user2)
CLIENT3_PID=$(run_client_test 3 3 2 "usr2" "usr2" "test_dir/test.txt")
print_status "Client 3 (File Reading) launched - PID: $CLIENT3_PID"

# Test 4: Connection duration (user3)
CLIENT4_PID=$(run_client_test 4 4 4 "usr3" "usr3")
print_status "Client 4 (Connection Duration) launched - PID: $CLIENT4_PID"

# Wait for client tests to complete
sleep 15

# Check client outputs
for i in {1..4}; do
    if [ -f "client_output_${i}.log" ]; then
        if grep -q "Authentication successful" "client_output_${i}.log"; then
            print_success "Client $i completed successfully"
        else
            print_error "Client $i may have failed"
        fi
    else
        print_error "No output log found for client $i"
    fi
done

# Cleanup
print_status "Cleaning up..."

# Kill server and any remaining clients
kill $SERVER_PID 2>/dev/null
for pid in $CLIENT1_PID $CLIENT2_PID $CLIENT3_PID $CLIENT4_PID; do
    kill $pid 2>/dev/null
done

# Wait for processes to terminate
sleep 2

# Check if server was terminated
if ! ps -p $SERVER_PID > /dev/null; then
    print_success "Server shutdown successfully"
else
    print_error "Server shutdown failed"
    kill -9 $SERVER_PID 2>/dev/null
fi

# Clean up test files
rm -rf test_dir
rm -f client_output_*.log

print_header "Test Suite Completed"
echo -e "${CYAN}Test Summary:${NC}"
echo -e "- Tested 4 concurrent clients"
echo -e "- Tested all available services"
echo -e "- Tested different user authentications"
echo -e "\n${GREEN}${BOLD}Test suite execution completed!${NC}\n"