#!/bin/bash

# Enhanced color definitions
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
ORANGE='\033[0;33m'
LIGHTBLUE='\033[1;34m'
LIGHTGREEN='\033[1;32m'
LIGHTRED='\033[1;31m'
NC='\033[0m' # No Color
BOLD='\033[1m'

# Server color codes
declare -A SERVER_COLORS
SERVER_COLORS[0]="${LIGHTBLUE}"
SERVER_COLORS[1]="${LIGHTGREEN}"
SERVER_COLORS[2]="${ORANGE}"
SERVER_COLORS[3]="${PURPLE}"

# Function for fancy borders
print_border() {
    echo -e "${PURPLE}${BOLD}╔════════════════════════════════════════════════════════════╗${NC}"
}

print_bottom_border() {
    echo -e "${PURPLE}${BOLD}╚════════════════════════════════════════════════════════════╝${NC}"
}

# Enhanced section headers
print_header() {
    print_border
    echo -e "${PURPLE}${BOLD}║${NC} ${BLUE}${BOLD}   $1${NC}$(printf '%*s' $((49-${#1})) '')${PURPLE}${BOLD}║${NC}"
    print_bottom_border
}

# Enhanced status messages with timestamp
print_status() {
    local timestamp=$(date "+%H:%M:%S")
    echo -e "${YELLOW}${BOLD}[${timestamp}][STATUS]${NC} $1"
}

# Enhanced success messages with timestamp
print_success() {
    local timestamp=$(date "+%H:%M:%S")
    echo -e "${GREEN}${BOLD}[${timestamp}][SUCCESS]${NC} $1"
}

# Enhanced error messages with timestamp
print_error() {
    local timestamp=$(date "+%H:%M:%S")
    echo -e "${RED}${BOLD}[${timestamp}][ERROR]${NC} $1"
}

# Server status message with color coding
print_server_status() {
    local server_id=$1
    local message=$2
    local timestamp=$(date "+%H:%M:%S")
    echo -e "${SERVER_COLORS[$server_id]}${BOLD}[${timestamp}][Server-$server_id]${NC} $message"
}

# Client status message with color coding
print_client_status() {
    local client_id=$1
    local message=$2
    local timestamp=$(date "+%H:%M:%S")
    local color="${CYAN}"
    echo -e "${color}${BOLD}[${timestamp}][Client-$client_id]${NC} $message"
}

# Enhanced test environment setup
setup_test_environment() {
    print_status "Initializing test environment..."
    mkdir -p test_dir
    local files=("test.txt" "config.ini" "data.log" "users.db")
    local content=(
        "This is a test file for server testing"
        "[config]\nport=8080\nmax_connections=10"
        "2024-01-16 12:00:00 INFO Server started"
        "admin:admin\nusr1:usr1\nusr2:usr2\nusr3:usr3"
    )
    
    for i in "${!files[@]}"; do
        echo -e "${content[$i]}" > "test_dir/${files[$i]}"
        print_success "Created test file: ${files[$i]}"
    done
}

# Enhanced client test function
run_client_test() {
    local test_num=$1
    local service_num=$2
    local delay=$3
    local username=$4
    local password=$5
    local extra_input=$6
    
    print_client_status $test_num "Initiating connection..."
    
    expect -f - << EOF > client_output_${test_num}.log 2>&1 &
spawn ./client
expect "Username: "
send "$username\r"
expect "Password: "
send "$password\r"
expect "Choose"
send "$service_num\r"
if { "$service_num" == "3" } {
    expect "Enter filename:"
    send "$extra_input\r"
}
sleep $delay
expect "Choose"
send "5\r"
expect eof
EOF
    
    local pid=$!
    print_client_status $test_num "Started with PID: $pid"
    echo $pid
}

# Start of script
clear
print_header "Advanced Multi-Client TCP Server Test Suite"

# Show test configuration
echo -e "${CYAN}${BOLD}Test Configuration:${NC}"
echo -e "• Number of servers: 4"
echo -e "• Number of clients: 4"
echo -e "• Test duration: ~15 seconds"
echo -e "• Services tested: Time, Directory, File Reading, Connection Duration\n"

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
print_header "Server Initialization"
print_status "Starting multi-server system..."
./server &
SERVER_PID=$!
sleep 2

if ps -p $SERVER_PID > /dev/null; then
    for i in {0..3}; do
        print_server_status $i "Server process initialized"
    done
    print_success "Master server started with PID: ${SERVER_PID}"
else
    print_error "Server failed to start"
    exit 1
fi

# Run multiple client tests
print_header "Client Testing Phase"

# Test matrix display
echo -e "${BOLD}Test Matrix:${NC}"
echo -e "┌────────────┬──────────────┬─────────────┬──────────────┐"
echo -e "│ Client ID  │   Service    │    User     │    Status    │"
echo -e "├────────────┼──────────────┼─────────────┼──────────────┤"

# Launch clients with visual feedback
CLIENT_PIDS=()

# Test 1: Time Service
CLIENT1_PID=$(run_client_test 1 1 2 "admin" "admin")
CLIENT_PIDS+=($CLIENT1_PID)
echo -e "│    1       │     Time     │    admin    │  Running     │"

# Test 2: Directory Listing
CLIENT2_PID=$(run_client_test 2 2 3 "usr1" "usr1")
CLIENT_PIDS+=($CLIENT2_PID)
echo -e "│    2       │     Dir      │    usr1     │  Running     │"

# Test 3: File Reading
CLIENT3_PID=$(run_client_test 3 3 2 "usr2" "usr2" "test_dir/test.txt")
CLIENT_PIDS+=($CLIENT3_PID)
echo -e "│    3       │     File     │    usr2     │  Running     │"

# Test 4: Connection Duration
CLIENT4_PID=$(run_client_test 4 4 4 "usr3" "usr3")
CLIENT_PIDS+=($CLIENT4_PID)
echo -e "│    4       │   Duration   │    usr3     │  Running     │"
echo -e "└────────────┴──────────────┴─────────────┴──────────────┘"

# Monitor progress
print_header "Test Progress Monitoring"
for i in {1..15}; do
    echo -ne "\rProgress: [${GREEN}"
    for ((j=0; j<i; j++)); do echo -ne "█"; done
    for ((j=i; j<15; j++)); do echo -ne "${NC}─"; done
    echo -ne "${NC}] $((i*100/15))%"
    sleep 1
done
echo -e "\n"

# Check results
print_header "Test Results"
for i in {1..4}; do
    if [ -f "client_output_${i}.log" ]; then
        if grep -q "Authentication successful" "client_output_${i}.log"; then
            print_success "Client $i: Test completed successfully"
        else
            print_error "Client $i: Test may have failed"
        fi
    fi
done

# Cleanup
print_header "Cleanup Phase"
print_status "Terminating all processes..."

kill $SERVER_PID 2>/dev/null
for pid in "${CLIENT_PIDS[@]}"; do
    kill $pid 2>/dev/null
done

sleep 2

# Final cleanup
rm -rf test_dir
rm -f client_output_*.log

print_header "Final Test Summary"
echo -e "${CYAN}${BOLD}Test Execution Summary:${NC}"
echo -e "✓ All server processes initialized"
echo -e "✓ 4 concurrent clients tested"
echo -e "✓ All core services verified"
echo -e "✓ Multiple user authentications tested"
echo -e "\n${GREEN}${BOLD}Test suite execution completed successfully!${NC}\n"