#!/bin/bash

# Demo script pentru REST API Framework

echo "╔════════════════════════════════════════════════════╗"
echo "║    REST API Framework - DEMO COMPLET              ║"
echo "╚════════════════════════════════════════════════════╝"
echo ""

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Cleanup function
cleanup() {
    echo -e "\n${YELLOW}[CLEANUP]${NC} Stopping server..."
    pkill -f "./rest_api" 2>/dev/null
    sleep 2
    
    echo -e "${YELLOW}[CLEANUP]${NC} Cleaning IPC resources..."
    rm -f /dev/shm/sem.* /dev/shm/* 2>/dev/null
    
    echo -e "${GREEN}[CLEANUP]${NC} Done!"
}

trap cleanup EXIT

# Check if we're in build directory
if [ ! -f "./rest_api" ]; then
    echo -e "${RED}[ERROR]${NC} Executabilul rest_api nu a fost găsit!"
    echo -e "${YELLOW}[INFO]${NC} Asigură-te că ești în directorul build/"
    exit 1
fi

# Start server
echo -e "${BLUE}[STEP 1]${NC} Pornire server (port 8080, 4 workers)..."
./rest_api 8080 4 > /tmp/server.log 2>&1 &
SERVER_PID=$!
sleep 3

if ! ps -p $SERVER_PID > /dev/null; then
    echo -e "${RED}[ERROR]${NC} Server-ul nu a pornit!"
    cat /tmp/server.log
    exit 1
fi
echo -e "${GREEN}[OK]${NC} Server running (PID: $SERVER_PID)"

# Test 1: Health check
echo -e "\n${BLUE}[STEP 2]${NC} Testare health check..."
HEALTH=$(curl -s http://localhost:8080/health)
if [ $? -eq 0 ]; then
    echo -e "${GREEN}[OK]${NC} $HEALTH"
else
    echo -e "${RED}[FAIL]${NC} Health check failed"
fi

sleep 1

# Test 2: Create users
echo -e "\n${BLUE}[STEP 3]${NC} Creare utilizatori de test..."
for i in {1..5}; do
    RESPONSE=$(curl -s -X POST http://localhost:8080/api/users \
        -H "Content-Type: application/json" \
        -d "{\"name\":\"User$i\",\"email\":\"user$i@test.com\",\"password\":\"pass$i\"}")
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}[OK]${NC} Created User$i"
    else
        echo -e "${YELLOW}[WARN]${NC} User$i creation may have failed"
    fi
    sleep 0.5
done

# Test 3: Get all users
echo -e "\n${BLUE}[STEP 4]${NC} Obținere listă utilizatori..."
USERS=$(curl -s http://localhost:8080/api/users)
if [ $? -eq 0 ]; then
    echo -e "${GREEN}[OK]${NC} Users retrieved:"
    echo "$USERS" | head -10
else
    echo -e "${RED}[FAIL]${NC} Failed to get users"
fi

# Test 4: IPC Demo (if available)
if [ -f "./ipc_demo_client" ]; then
    echo -e "\n${BLUE}[STEP 5]${NC} Demonstrație IPC cu Priority Queue..."
    ./ipc_demo_client 2>&1 | head -20
    sleep 2
else
    echo -e "\n${YELLOW}[SKIP]${NC} IPC demo client not found"
fi

# Test 5: Load test (if available)
if [ -f "./load_test_client" ]; then
    echo -e "\n${BLUE}[STEP 6]${NC} Running load test (1000 requests)..."
    ./load_test_client 10 100 2>&1
else
    echo -e "\n${YELLOW}[SKIP]${NC} Load test client not found"
fi

# Test 6: Simple client (if available)
if [ -f "./simple_client" ]; then
    echo -e "\n${BLUE}[STEP 7]${NC} Testare simple client..."
    timeout 5 ./simple_client 127.0.0.1 8080 2>&1 | head -20
else
    echo -e "\n${YELLOW}[SKIP]${NC} Simple client not found"
fi

# Show server stats
echo -e "\n${BLUE}[STEP 8]${NC} Server statistics (last 20 lines)..."
tail -20 /tmp/server.log

echo -e "\n╔════════════════════════════════════════════════════╗"
echo -e "║           ${GREEN}DEMONSTRAȚIE COMPLETĂ!${NC}                 ║"
echo "╚════════════════════════════════════════════════════╝"

echo -e "\n${BLUE}[INFO]${NC} Server-ul va fi oprit în 5 secunde..."
sleep 5
