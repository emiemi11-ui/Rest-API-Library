#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${CYAN}╔════════════════════════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║  🔥 REST API FRAMEWORK - MULTI-DOMAIN DEMO                    ║${NC}"
echo -e "${CYAN}║  Demonstrating 5 Different APIs on the Same Framework        ║${NC}"
echo -e "${CYAN}╚════════════════════════════════════════════════════════════════╝${NC}"
echo ""

# Check if build directory exists
if [ ! -d "../build" ]; then
    echo -e "${YELLOW}[INFO] Build directory not found. Creating...${NC}"
    mkdir -p ../build
fi

# Build all examples
echo -e "${CYAN}[BUILD] Building framework and examples...${NC}"
cd ../build

# Run CMake
cmake .. > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo -e "${RED}[ERROR] CMake failed! Running with verbose output:${NC}"
    cmake ..
    exit 1
fi

# Build
make -j4 > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo -e "${RED}[ERROR] Build failed! Running with verbose output:${NC}"
    make VERBOSE=1
    exit 1
fi

echo -e "${GREEN}[OK] Build completed successfully!${NC}"
echo ""

# Return to demo directory
cd - > /dev/null

# Kill any existing servers on these ports
echo -e "${YELLOW}[CLEANUP] Checking for existing servers...${NC}"
for port in 8080 8081 8082 8083 8084; do
    pid=$(lsof -ti:$port 2>/dev/null)
    if [ ! -z "$pid" ]; then
        echo -e "${YELLOW}  Killing process on port $port (PID: $pid)${NC}"
        kill -9 $pid 2>/dev/null
        sleep 0.5
    fi
done
echo ""

# Start servers
echo -e "${CYAN}[START] Starting 5 different REST APIs...${NC}"
echo ""

# Server 1: Simple API
echo -e "${GREEN}🚀 Starting Simple API (port 8080)...${NC}"
../build/example1_simple > /dev/null 2>&1 &
PID1=$!
sleep 1

# Server 2: E-Commerce API
echo -e "${GREEN}🛍️  Starting E-Commerce API (port 8081)...${NC}"
if [ -f "../build/example2_ecommerce" ]; then
    ../build/example2_ecommerce > /dev/null 2>&1 &
    PID2=$!
    sleep 1
else
    echo -e "${YELLOW}  ⚠️  E-Commerce example not built yet (skipping)${NC}"
    PID2=""
fi

# Server 3: IoT Sensors API
echo -e "${GREEN}📡 Starting IoT Sensors API (port 8082)...${NC}"
../build/example3_iot > /dev/null 2>&1 &
PID3=$!
sleep 1

# Server 4: Banking API
echo -e "${GREEN}💰 Starting Banking API (port 8083)...${NC}"
../build/example4_banking > /dev/null 2>&1 &
PID4=$!
sleep 1

# Server 5: Medical API
echo -e "${GREEN}🏥 Starting Medical Records API (port 8084)...${NC}"
../build/example5_medical > /dev/null 2>&1 &
PID5=$!
sleep 1

echo ""
echo -e "${CYAN}╔════════════════════════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║  ✅ ALL SERVERS RUNNING!                                       ║${NC}"
echo -e "${CYAN}╠════════════════════════════════════════════════════════════════╣${NC}"
echo -e "${CYAN}║  📍 Simple API:        http://localhost:8080                  ║${NC}"
if [ ! -z "$PID2" ]; then
echo -e "${CYAN}║  📍 E-Commerce API:    http://localhost:8081                  ║${NC}"
fi
echo -e "${CYAN}║  📍 IoT Sensors API:   http://localhost:8082                  ║${NC}"
echo -e "${CYAN}║  📍 Banking API:       http://localhost:8083                  ║${NC}"
echo -e "${CYAN}║  📍 Medical API:       http://localhost:8084                  ║${NC}"
echo -e "${CYAN}╚════════════════════════════════════════════════════════════════╝${NC}"
echo ""
echo -e "${YELLOW}💡 Test the servers with:${NC}"
echo -e "   ${GREEN}python3 client_tester.py${NC}"
echo ""
echo -e "${YELLOW}💡 Or manually:${NC}"
echo -e "   ${GREEN}curl http://localhost:8080/${NC}"
echo -e "   ${GREEN}curl http://localhost:8082/api/sensors/stats${NC}"
echo -e "   ${GREEN}curl http://localhost:8083/api/accounts${NC}"
echo ""
echo -e "${RED}Press Ctrl+C to stop all servers...${NC}"
echo ""

# Cleanup function
cleanup() {
    echo ""
    echo -e "${YELLOW}[SHUTDOWN] Stopping all servers...${NC}"

    if [ ! -z "$PID1" ]; then kill $PID1 2>/dev/null; fi
    if [ ! -z "$PID2" ]; then kill $PID2 2>/dev/null; fi
    if [ ! -z "$PID3" ]; then kill $PID3 2>/dev/null; fi
    if [ ! -z "$PID4" ]; then kill $PID4 2>/dev/null; fi
    if [ ! -z "$PID5" ]; then kill $PID5 2>/dev/null; fi

    sleep 1
    echo -e "${GREEN}[OK] All servers stopped.${NC}"
    exit 0
}

# Trap Ctrl+C
trap cleanup INT TERM

# Wait for all processes
wait
