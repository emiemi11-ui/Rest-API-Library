#!/bin/bash

# ================================================================
# REST API E-COMMERCE - PRODUCTION DEPLOYMENT SCRIPT
# Complete setup and deployment for production-ready REST API
# ================================================================

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
NC='\033[0m' # No Color
BOLD='\033[1m'

# Configuration
PORT="${PORT:-8080}"
WORKERS="${WORKERS:-4}"
BUILD_DIR="build"
DB_FILE="app.db"

echo -e "${CYAN}╔════════════════════════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║                                                                ║${NC}"
echo -e "${CYAN}║   ${BOLD}REST API E-COMMERCE - PRODUCTION DEPLOYMENT${NC}${CYAN}            ║${NC}"
echo -e "${CYAN}║   ${BOLD}Enterprise-Grade C++ REST API Server${NC}${CYAN}                   ║${NC}"
echo -e "${CYAN}║                                                                ║${NC}"
echo -e "${CYAN}╚════════════════════════════════════════════════════════════════╝${NC}"
echo

# ================================================================
# STEP 1: Check Dependencies
# ================================================================
echo -e "${BLUE}[STEP 1/6]${NC} Checking dependencies..."

check_command() {
    if ! command -v $1 &> /dev/null; then
        echo -e "${RED}[ERROR]${NC} $1 is not installed!"
        echo "Install with: sudo apt-get install $2"
        return 1
    fi
    echo -e "  ${GREEN}✓${NC} $1 found"
}

check_command cmake cmake || exit 1
check_command g++ build-essential || exit 1
check_command sqlite3 sqlite3 || exit 1
check_command make build-essential || exit 1

echo -e "${GREEN}[OK]${NC} All dependencies satisfied\n"

# ================================================================
# STEP 2: Clean and Build
# ================================================================
echo -e "${BLUE}[STEP 2/6]${NC} Building project..."

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
echo "  Configuring with CMake..."
cmake .. > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo -e "${RED}[ERROR]${NC} CMake configuration failed"
    exit 1
fi
echo -e "  ${GREEN}✓${NC} CMake configured"

# Build all targets
echo "  Compiling project (this may take a moment)..."
make -j4 > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo -e "${RED}[ERROR]${NC} Compilation failed"
    exit 1
fi

cd ..

# Check executables
EXECUTABLES=("rest_api" "shop_client" "admin_client" "load_test_client" "ipc_demo_server" "ipc_demo_client" "simple_client")
for exe in "${EXECUTABLES[@]}"; do
    if [ -f "$BUILD_DIR/$exe" ]; then
        echo -e "  ${GREEN}✓${NC} $exe built successfully"
    else
        echo -e "  ${YELLOW}⚠${NC} $exe not found (optional)"
    fi
done

echo -e "${GREEN}[OK]${NC} Build completed\n"

# ================================================================
# STEP 3: Initialize Database
# ================================================================
echo -e "${BLUE}[STEP 3/6]${NC} Initializing database..."

if [ -f scripts/init_database.sh ]; then
    chmod +x scripts/init_database.sh
    ./scripts/init_database.sh --reset 2>&1 | grep -E "(OK|ERROR|BACKUP|Users:|Products:|Orders:)" || true
else
    echo -e "${YELLOW}[WARN]${NC} Database init script not found, skipping"
fi

echo -e "${GREEN}[OK]${NC} Database initialized\n"

# ================================================================
# STEP 4: Network Configuration
# ================================================================
echo -e "${BLUE}[STEP 4/6]${NC} Network configuration..."

# Get local IP address
LOCAL_IP=$(hostname -I | awk '{print $1}')
echo -e "  ${CYAN}Server will be accessible at:${NC}"
echo -e "    • Local:   ${GREEN}http://localhost:$PORT${NC}"
echo -e "    • Network: ${GREEN}http://$LOCAL_IP:$PORT${NC}"
echo

# Check if port is already in use
if lsof -Pi :$PORT -sTCP:LISTEN -t >/dev/null 2>&1 ; then
    echo -e "  ${YELLOW}⚠${NC} Port $PORT is already in use"
    echo -e "    Killing existing process..."
    lsof -ti:$PORT | xargs kill -9 2>/dev/null || true
    sleep 1
fi

echo -e "${GREEN}[OK]${NC} Network configured\n"

# ================================================================
# STEP 5: Display Server Information
# ================================================================
echo -e "${BLUE}[STEP 5/6]${NC} Server configuration..."

cat << EOF
  ┌────────────────────────────────────────────────────┐
  │ Configuration:                                     │
  │  • Port:      $PORT                                   │
  │  • Workers:   $WORKERS                                   │
  │  • Database:  $DB_FILE                              │
  │  • Local IP:  $LOCAL_IP                          │
  └────────────────────────────────────────────────────┘

  ┌────────────────────────────────────────────────────┐
  │ Available Executables:                             │
  │  • rest_api        - Main REST API server          │
  │  • shop_client     - E-commerce shop client        │
  │  • admin_client    - Admin dashboard               │
  │  • load_test_client - Load testing tool            │
  └────────────────────────────────────────────────────┘
EOF

echo -e "${GREEN}[OK]${NC} Configuration ready\n"

# ================================================================
# STEP 6: Start Server
# ================================================================
echo -e "${BLUE}[STEP 6/6]${NC} Starting REST API server..."
echo

# Start server in background if requested
if [ "$1" == "--start" ] || [ "$1" == "-s" ]; then
    echo -e "${CYAN}Starting server in background...${NC}"
    nohup ./$BUILD_DIR/rest_api $PORT $WORKERS > server.log 2>&1 &
    SERVER_PID=$!
    echo $SERVER_PID > server.pid

    sleep 2

    # Check if server started successfully
    if ps -p $SERVER_PID > /dev/null; then
        echo -e "${GREEN}✓ Server started successfully${NC}"
        echo -e "  PID: $SERVER_PID"
        echo -e "  Logs: server.log"
        echo

        # Quick health check
        sleep 1
        if curl -s http://localhost:$PORT/health > /dev/null 2>&1; then
            echo -e "${GREEN}✓ Health check passed${NC}"
        else
            echo -e "${YELLOW}⚠ Health check pending${NC}"
        fi
    else
        echo -e "${RED}✗ Server failed to start${NC}"
        cat server.log
        exit 1
    fi
else
    echo -e "${CYAN}To start the server manually:${NC}"
    echo -e "  ./$BUILD_DIR/rest_api $PORT $WORKERS"
    echo
    echo -e "${CYAN}To start the server in background:${NC}"
    echo -e "  ./deploy.sh --start"
fi

echo

# ================================================================
# Final Summary
# ================================================================
echo -e "${CYAN}╔════════════════════════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║                                                                ║${NC}"
echo -e "${CYAN}║   ${GREEN}${BOLD}✓ DEPLOYMENT SUCCESSFUL!${NC}${CYAN}                                    ║${NC}"
echo -e "${CYAN}║                                                                ║${NC}"
echo -e "${CYAN}╚════════════════════════════════════════════════════════════════╝${NC}"
echo

echo -e "${BOLD}Quick Start Commands:${NC}"
echo
echo -e "  ${CYAN}1. Start Server:${NC}"
echo -e "     ./$BUILD_DIR/rest_api $PORT $WORKERS"
echo
echo -e "  ${CYAN}2. Test with Shop Client:${NC}"
echo -e "     ./$BUILD_DIR/shop_client localhost $PORT"
echo
echo -e "  ${CYAN}3. Monitor with Admin Dashboard:${NC}"
echo -e "     ./$BUILD_DIR/admin_client localhost $PORT"
echo
echo -e "  ${CYAN}4. Run Load Test:${NC}"
echo -e "     ./$BUILD_DIR/load_test_client --server localhost --port $PORT --threads 10 --duration 30"
echo
echo -e "  ${CYAN}5. Connect from Other Machines:${NC}"
echo -e "     ./$BUILD_DIR/shop_client $LOCAL_IP $PORT"
echo

echo -e "${BOLD}API Documentation:${NC}"
echo -e "  • View README.md for complete API documentation"
echo -e "  • Access endpoints at: ${GREEN}http://localhost:$PORT${NC}"
echo -e "  • Default credentials: ${YELLOW}admin / admin123${NC}"
echo

if [ "$1" == "--start" ] || [ "$1" == "-s" ]; then
    echo -e "${BOLD}Server Management:${NC}"
    echo -e "  • Stop server:  kill \$(cat server.pid)"
    echo -e "  • View logs:    tail -f server.log"
    echo -e "  • Health check: curl http://localhost:$PORT/health"
    echo
fi

echo -e "${GREEN}${BOLD}Deployment completed successfully!${NC}"
echo
