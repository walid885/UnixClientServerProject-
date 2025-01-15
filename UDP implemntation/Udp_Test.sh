#!/bin/bash

# Terminal Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color
BOLD='\033[1m'

# Function to print section headers
print_header() {
    echo -e "\n${BOLD}${BLUE}=== $1 ===${NC}\n"
}

# Function to print success messages
print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

# Function to print error messages
print_error() {
    echo -e "${RED}✗ $1${NC}"
}

# Function to check and set execute permissions
set_permissions() {
    local file=$1
    if [ -f "$file" ]; then
        chmod +x "$file"
        print_success "Set execute permissions for $file"
    else
        print_error "File $file not found"
        return 1
    fi
}

# Function to run client with timeout
run_client() {
    local client_num=$1
    local server_ip=$2
    local port=$3
    
    echo -e "${YELLOW}Starting Client $client_num...${NC}"
    timeout 5s ./UdpClient $server_ip $port
    
    if [ $? -eq 124 ]; then
        print_error "Client $client_num timed out"
        return 1

    else
        print_success "Client $client_num completed successfully"
        return 0
    fi
}

# Clear the terminal
clear

# Print welcome message
print_header "UDP Client-Server Test Suite"
echo -e "${BOLD}This script will test the UDP server with multiple clients${NC}"

# Check if executables exist and compile if necessary
print_header "Checking Prerequisites"
if [ ! -f "./UdpServer" ] || [ ! -f "./UdpClient" ]; then
    print_error "Server or client executable not found!"
    echo -e "${YELLOW}Running compilation script...${NC}"
    
    # Compile server
    gcc -Wall UdpServer.c -o UdpServer
    if [ $? -ne 0 ]; then
        print_error "Server compilation failed! Exiting..."
        exit 1
    fi
    
    # Compile client
    gcc -Wall UdpClient.c -o UdpClient
    if [ $? -ne 0 ]; then
        print_error "Client compilation failed! Exiting..."
        exit 1
    fi
fi

# Set execute permissions
set_permissions "./UdpServer" || exit 1
set_permissions "./UdpClient" || exit 1

print_success "Prerequisites check passed"

# Configuration
SERVER_PORT=8080
SERVER_IP="127.0.0.1"

# Start the server
print_header "Starting UDP Server"

# Kill any existing UDP server processes
pkill -f "./UdpServer" 2>/dev/null

# Start the server with proper error handling
./UdpServer $SERVER_PORT > server_output.log 2>&1 &
SERVER_PID=$!

# Wait for server to start and verify it's running
sleep 2

if ps -p $SERVER_PID > /dev/null; then
    print_success "Server started successfully on port $SERVER_PORT (PID: $SERVER_PID)"
else
    print_error "Server failed to start!"

fi

# Run test cases
print_header "Running Test Cases"

echo -e "${BOLD}Test Case 1:${NC} Basic Communication Test"
run_client 1 $SERVER_IP $SERVER_PORT
echo

echo -e "${BOLD}Test Case 2:${NC} Multiple Number Request"
run_client 2 $SERVER_IP $SERVER_PORT
echo

echo -e "${BOLD}Test Case 3:${NC} Final Verification"
run_client 3 $SERVER_IP $SERVER_PORT
echo

# Clean up
print_header "Cleaning Up"
echo -e "${YELLOW}Stopping server (PID: $SERVER_PID)...${NC}"
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null
print_success "Server stopped successfully"



# Final summary
print_header "Test Summary"
echo -e "${BOLD}Testing completed!${NC}"

exit 0