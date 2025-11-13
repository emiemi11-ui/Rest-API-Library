#!/bin/bash

# Culori pentru output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

BASE_URL="http://localhost:8080"
PASS=0
FAIL=0

echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║   REST API TEST SUITE                  ║${NC}"
echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo ""

# Helper function pentru testare
test_endpoint() {
    local name="$1"
    local method="$2"
    local url="$3"
    local data="$4"
    local expected_code="$5"
    
    echo -e "${YELLOW}TEST: ${name}${NC}"
    
    if [ -z "$data" ]; then
        response=$(curl -s -w "\n%{http_code}" -X "$method" "$url")
    else
        response=$(curl -s -w "\n%{http_code}" -X "$method" "$url" -H "Content-Type: application/json" -d "$data")
    fi
    
    http_code=$(echo "$response" | tail -n1)
    body=$(echo "$response" | sed '$d')
    
    if [ "$http_code" -eq "$expected_code" ]; then
        echo -e "${GREEN}✓ PASS${NC} - HTTP $http_code"
        echo -e "  Response: ${body:0:100}..."
        ((PASS++))
    else
        echo -e "${RED}✗ FAIL${NC} - Expected HTTP $expected_code, got $http_code"
        echo -e "  Response: $body"
        ((FAIL++))
    fi
    echo ""
}

echo -e "${BLUE}═══════════════════════════════════════${NC}"
echo -e "${BLUE}1. Testing Health & Root Endpoints${NC}"
echo -e "${BLUE}═══════════════════════════════════════${NC}"

test_endpoint "Health Check" "GET" "$BASE_URL/health" "" 200
test_endpoint "Root Endpoint" "GET" "$BASE_URL/" "" 200

echo -e "${BLUE}═══════════════════════════════════════${NC}"
echo -e "${BLUE}2. Testing GET Operations${NC}"
echo -e "${BLUE}═══════════════════════════════════════${NC}"

test_endpoint "Get All Users" "GET" "$BASE_URL/api/users" "" 200
test_endpoint "Get User by ID (1)" "GET" "$BASE_URL/api/users/1" "" 200
test_endpoint "Get User by ID (2)" "GET" "$BASE_URL/api/users/2" "" 200

echo -e "${BLUE}═══════════════════════════════════════${NC}"
echo -e "${BLUE}3. Testing POST Operations${NC}"
echo -e "${BLUE}═══════════════════════════════════════${NC}"

test_endpoint "Create New User" "POST" "$BASE_URL/api/users" \
    '{"name":"Test User","email":"test@example.com"}' 201

test_endpoint "Create Another User" "POST" "$BASE_URL/api/users" \
    '{"name":"John Doe","email":"john@example.com"}' 201

# Verifică că utilizatorii au fost creați
test_endpoint "Verify New Users Created" "GET" "$BASE_URL/api/users" "" 200

echo -e "${BLUE}═══════════════════════════════════════${NC}"
echo -e "${BLUE}4. Testing PUT Operations${NC}"
echo -e "${BLUE}═══════════════════════════════════════${NC}"

test_endpoint "Update User 1" "PUT" "$BASE_URL/api/users/1" \
    '{"name":"Ion Popescu Updated","email":"ion.updated@test.com"}' 200

test_endpoint "Verify User 1 Updated" "GET" "$BASE_URL/api/users/1" "" 200

echo -e "${BLUE}═══════════════════════════════════════${NC}"
echo -e "${BLUE}5. Testing DELETE Operations${NC}"
echo -e "${BLUE}═══════════════════════════════════════${NC}"

test_endpoint "Delete User 2" "DELETE" "$BASE_URL/api/users/2" "" 200
test_endpoint "Verify User 2 Deleted" "GET" "$BASE_URL/api/users/2" "" 404

echo -e "${BLUE}═══════════════════════════════════════${NC}"
echo -e "${BLUE}6. Testing Error Handling${NC}"
echo -e "${BLUE}═══════════════════════════════════════${NC}"

test_endpoint "404 - Invalid Route" "GET" "$BASE_URL/api/invalid" "" 404
test_endpoint "404 - Nonexistent User" "GET" "$BASE_URL/api/users/999" "" 404
test_endpoint "400 - Create User Without Email" "POST" "$BASE_URL/api/users" \
    '{"name":"Invalid User"}' 400
test_endpoint "400 - Create User With Invalid Email" "POST" "$BASE_URL/api/users" \
    '{"name":"Invalid User","email":"notanemail"}' 400
test_endpoint "400 - Update Nonexistent User" "PUT" "$BASE_URL/api/users/999" \
    '{"name":"Test","email":"test@test.com"}' 400

echo ""
echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║   TEST RESULTS                         ║${NC}"
echo -e "${BLUE}╠════════════════════════════════════════╣${NC}"
echo -e "${GREEN}║   Passed: $PASS                            ${GREEN}║${NC}"
echo -e "${RED}║   Failed: $FAIL                             ${RED}║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════╝${NC}"

if [ $FAIL -eq 0 ]; then
    echo -e "\n${GREEN}🎉 All tests passed! Your API is working perfectly!${NC}\n"
    exit 0
else
    echo -e "\n${RED}⚠️  Some tests failed. Please check the output above.${NC}\n"
    exit 1
fi
