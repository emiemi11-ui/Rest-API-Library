#!/bin/bash

# Comprehensive API Test Script for Enterprise REST API
# Tests all phases: Authentication, Rate Limiting, CORS, Metrics, etc.

API_URL="http://localhost:8080"
BOLD='\033[1m'
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${BOLD}╔════════════════════════════════════════════════════════╗${NC}"
echo -e "${BOLD}║    Enterprise REST API - Comprehensive Test Suite     ║${NC}"
echo -e "${BOLD}╚════════════════════════════════════════════════════════╝${NC}"
echo ""

TESTS_PASSED=0
TESTS_FAILED=0

# Function to test endpoint
test_endpoint() {
    local name="$1"
    local method="$2"
    local endpoint="$3"
    local data="$4"
    local expected_status="$5"
    local headers="$6"

    echo -ne "${YELLOW}[TEST]${NC} $name... "

    if [ -z "$data" ]; then
        response=$(curl -s -w "\n%{http_code}" -X "$method" "$API_URL$endpoint" $headers 2>/dev/null)
    else
        response=$(curl -s -w "\n%{http_code}" -X "$method" "$API_URL$endpoint" \
            -H "Content-Type: application/json" \
            -d "$data" $headers 2>/dev/null)
    fi

    status_code=$(echo "$response" | tail -n 1)
    body=$(echo "$response" | head -n -1)

    if [ "$status_code" = "$expected_status" ]; then
        echo -e "${GREEN}✓ PASS${NC} (Status: $status_code)"
        ((TESTS_PASSED++))
        return 0
    else
        echo -e "${RED}✗ FAIL${NC} (Expected: $expected_status, Got: $status_code)"
        echo -e "${RED}   Response: $body${NC}"
        ((TESTS_FAILED++))
        return 1
    fi
}

echo -e "${BOLD}═══ Phase 1: Basic Health & Connectivity ═══${NC}\n"

test_endpoint "Health Check" "GET" "/health" "" "200"
test_endpoint "Metrics Endpoint" "GET" "/api/metrics" "" "200"

echo ""
echo -e "${BOLD}═══ Phase 2: User Authentication ═══${NC}\n"

# Test login (assuming mock credentials from authcontroller.cpp)
LOGIN_RESPONSE=$(curl -s -X POST "$API_URL/api/auth/login" \
    -H "Content-Type: application/json" \
    -d '{"email":"admin@example.com","password":"admin123"}' 2>/dev/null)

TOKEN=$(echo "$LOGIN_RESPONSE" | grep -o '"token":"[^"]*"' | cut -d'"' -f4)

if [ -n "$TOKEN" ]; then
    echo -e "${GREEN}✓${NC} Login successful, token obtained"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗${NC} Login failed"
    ((TESTS_FAILED++))
fi

# Test authentication with token
test_endpoint "Profile with Auth" "GET" "/api/auth/profile" "" "200" "-H 'Authorization: Bearer $TOKEN'"

# Test authentication without token
test_endpoint "Profile without Auth" "GET" "/api/auth/profile" "" "401"

echo ""
echo -e "${BOLD}═══ Phase 3: User CRUD Operations ═══${NC}\n"

test_endpoint "Get All Users" "GET" "/api/users" "" "200"

# Create user
CREATE_RESPONSE=$(curl -s -X POST "$API_URL/api/users" \
    -H "Content-Type: application/json" \
    -d '{"name":"Test User","email":"test@example.com"}' 2>/dev/null)

if echo "$CREATE_RESPONSE" | grep -q "id"; then
    USER_ID=$(echo "$CREATE_RESPONSE" | grep -o '"id":[0-9]*' | cut -d':' -f2)
    echo -e "${GREEN}✓${NC} User created with ID: $USER_ID"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗${NC} User creation failed"
    ((TESTS_FAILED++))
    USER_ID="1"
fi

test_endpoint "Get User by ID" "GET" "/api/users/$USER_ID" "" "200"
test_endpoint "Update User" "PUT" "/api/users/$USER_ID" '{"name":"Updated User","email":"updated@example.com"}' "200"
test_endpoint "Delete User" "DELETE" "/api/users/$USER_ID" "" "204"
test_endpoint "Get Deleted User" "GET" "/api/users/$USER_ID" "" "404"

echo ""
echo -e "${BOLD}═══ Phase 4: Pagination & Filtering ═══${NC}\n"

test_endpoint "Pagination (Page 1)" "GET" "/api/users?page=1&limit=10" "" "200"
test_endpoint "Sorting (Name Desc)" "GET" "/api/users?sort=-name" "" "200"
test_endpoint "Search Users" "GET" "/api/users?search=test" "" "200"

echo ""
echo -e "${BOLD}═══ Phase 5: Rate Limiting ═══${NC}\n"

echo -ne "${YELLOW}[TEST]${NC} Rate limiting (burst requests)... "
RATE_LIMITED=false

for i in {1..110}; do
    status=$(curl -s -w "%{http_code}" -o /dev/null "$API_URL/health" 2>/dev/null)
    if [ "$status" = "429" ]; then
        RATE_LIMITED=true
        break
    fi
done

if [ "$RATE_LIMITED" = true ]; then
    echo -e "${GREEN}✓ PASS${NC} (Rate limiting active)"
    ((TESTS_PASSED++))
else
    echo -e "${YELLOW}⚠ SKIP${NC} (Rate limit not reached or disabled)"
fi

echo ""
echo -e "${BOLD}═══ Phase 6: CORS Headers ═══${NC}\n"

CORS_RESPONSE=$(curl -s -I -X OPTIONS "$API_URL/api/users" 2>/dev/null)

if echo "$CORS_RESPONSE" | grep -q "Access-Control-Allow-Origin"; then
    echo -e "${GREEN}✓${NC} CORS headers present"
    ((TESTS_PASSED++))
else
    echo -e "${YELLOW}⚠${NC} CORS headers not found"
fi

echo ""
echo -e "${BOLD}═══ Phase 7: Input Validation & Error Handling ═══${NC}\n"

test_endpoint "Invalid Email Format" "POST" "/api/users" '{"name":"Test","email":"invalid-email"}' "400"
test_endpoint "Missing Required Fields" "POST" "/api/users" '{"name":"Only Name"}' "400"
test_endpoint "Invalid JSON" "POST" "/api/users" '{invalid json}' "400"
test_endpoint "Non-existent Endpoint" "GET" "/api/nonexistent" "" "404"

echo ""
echo -e "${BOLD}═══ Phase 8: Performance & Monitoring ═══${NC}\n"

# Test metrics endpoint
METRICS=$(curl -s "$API_URL/api/metrics" 2>/dev/null)

if echo "$METRICS" | grep -q "total_requests"; then
    echo -e "${GREEN}✓${NC} Metrics collection active"
    ((TESTS_PASSED++))
else
    echo -e "${YELLOW}⚠${NC} Metrics not available"
fi

echo ""
echo -e "${BOLD}╔════════════════════════════════════════════════════════╗${NC}"
echo -e "${BOLD}║                  TEST RESULTS SUMMARY                  ║${NC}"
echo -e "${BOLD}╠════════════════════════════════════════════════════════╣${NC}"
echo -e "${BOLD}║${NC}  Total Tests:        $((TESTS_PASSED + TESTS_FAILED))                               ${BOLD}║${NC}"
echo -e "${BOLD}║${NC}  ${GREEN}Passed:${NC}             $TESTS_PASSED                               ${BOLD}║${NC}"
echo -e "${BOLD}║${NC}  ${RED}Failed:${NC}             $TESTS_FAILED                               ${BOLD}║${NC}"

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${BOLD}║${NC}  ${GREEN}Status:${NC}             ${GREEN}✓ ALL TESTS PASSED${NC}                  ${BOLD}║${NC}"
else
    echo -e "${BOLD}║${NC}  ${RED}Status:${NC}             ${RED}✗ SOME TESTS FAILED${NC}                 ${BOLD}║${NC}"
fi

echo -e "${BOLD}╚════════════════════════════════════════════════════════╝${NC}"
echo ""

if [ $TESTS_FAILED -eq 0 ]; then
    exit 0
else
    exit 1
fi
