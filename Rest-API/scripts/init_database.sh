#!/bin/bash

# ================================================================
# Database Initialization Script
# Creates and seeds the e-commerce database
# ================================================================

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Database path
DB_PATH="${DB_PATH:-app.db}"
SCHEMA_FILE="schema.sql"
SEED_FILE="seed_data.sql"

echo -e "${GREEN}╔════════════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║     E-Commerce Database Initialization        ║${NC}"
echo -e "${GREEN}╚════════════════════════════════════════════════╝${NC}"
echo

# Check if SQLite3 is installed
if ! command -v sqlite3 &> /dev/null; then
    echo -e "${RED}[ERROR] sqlite3 is not installed!${NC}"
    echo "Install with: sudo apt-get install sqlite3"
    exit 1
fi

# Check if schema file exists
if [ ! -f "$SCHEMA_FILE" ]; then
    echo -e "${RED}[ERROR] Schema file not found: $SCHEMA_FILE${NC}"
    exit 1
fi

# Backup existing database if it exists
if [ -f "$DB_PATH" ]; then
    BACKUP_PATH="${DB_PATH}.backup.$(date +%Y%m%d_%H%M%S)"
    echo -e "${YELLOW}[BACKUP] Creating backup: $BACKUP_PATH${NC}"
    cp "$DB_PATH" "$BACKUP_PATH"
    echo -e "${GREEN}[OK] Backup created${NC}"
    echo
fi

# Remove existing database if reset flag is provided
if [ "$1" == "--reset" ] || [ "$1" == "-r" ]; then
    echo -e "${YELLOW}[RESET] Removing existing database...${NC}"
    rm -f "$DB_PATH"
    echo -e "${GREEN}[OK] Database removed${NC}"
    echo
fi

# Create database and apply schema
echo "[1/3] Creating database and applying schema..."
sqlite3 "$DB_PATH" < "$SCHEMA_FILE"
if [ $? -eq 0 ]; then
    echo -e "${GREEN}[OK] Schema applied successfully${NC}"
else
    echo -e "${RED}[ERROR] Failed to apply schema${NC}"
    exit 1
fi
echo

# Seed data (optional)
if [ -f "$SEED_FILE" ]; then
    echo "[2/3] Seeding database with demo data..."
    sqlite3 "$DB_PATH" < "$SEED_FILE"
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}[OK] Data seeded successfully${NC}"
    else
        echo -e "${RED}[ERROR] Failed to seed data${NC}"
        exit 1
    fi
else
    echo -e "${YELLOW}[SKIP] Seed file not found: $SEED_FILE${NC}"
fi
echo

# Verify database
echo "[3/3] Verifying database..."
TABLE_COUNT=$(sqlite3 "$DB_PATH" "SELECT COUNT(*) FROM sqlite_master WHERE type='table';")
echo "  - Tables created: $TABLE_COUNT"

USER_COUNT=$(sqlite3 "$DB_PATH" "SELECT COUNT(*) FROM users;")
PRODUCT_COUNT=$(sqlite3 "$DB_PATH" "SELECT COUNT(*) FROM products;")
ORDER_COUNT=$(sqlite3 "$DB_PATH" "SELECT COUNT(*) FROM orders;")

echo "  - Users: $USER_COUNT"
echo "  - Products: $PRODUCT_COUNT"
echo "  - Orders: $ORDER_COUNT"
echo

echo -e "${GREEN}╔════════════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║     ✓ Database initialized successfully!      ║${NC}"
echo -e "${GREEN}╚════════════════════════════════════════════════╝${NC}"
echo
echo "Database location: $(pwd)/$DB_PATH"
echo "Database size: $(du -h "$DB_PATH" | cut -f1)"
echo
echo "Default credentials:"
echo "  Username: admin"
echo "  Password: admin123"
echo -e "${YELLOW}  ⚠️  CHANGE PASSWORD IN PRODUCTION!${NC}"
echo
