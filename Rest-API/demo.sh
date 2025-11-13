#!/bin/bash

# Script simplu de demonstrație pentru REST API

echo "🚀 REST API Demo - Quick Test"
echo "================================"
echo ""

BASE="http://localhost:8080"

echo "1️⃣  Health Check"
echo "   curl $BASE/health"
curl -s $BASE/health | jq . 2>/dev/null || curl -s $BASE/health
echo -e "\n"

echo "2️⃣  Get All Users"
echo "   curl $BASE/api/users"
curl -s $BASE/api/users | jq . 2>/dev/null || curl -s $BASE/api/users
echo -e "\n"

echo "3️⃣  Get User by ID"
echo "   curl $BASE/api/users/1"
curl -s $BASE/api/users/1 | jq . 2>/dev/null || curl -s $BASE/api/users/1
echo -e "\n"

echo "4️⃣  Create New User"
echo "   curl -X POST $BASE/api/users -d '{\"name\":\"Demo User\",\"email\":\"demo@test.com\"}'"
curl -s -X POST $BASE/api/users \
  -H "Content-Type: application/json" \
  -d '{"name":"Demo User","email":"demo@test.com"}' | jq . 2>/dev/null || \
curl -s -X POST $BASE/api/users \
  -H "Content-Type: application/json" \
  -d '{"name":"Demo User","email":"demo@test.com"}'
echo -e "\n"

echo "5️⃣  Get All Users Again (to see the new user)"
curl -s $BASE/api/users | jq . 2>/dev/null || curl -s $BASE/api/users
echo -e "\n"

echo "✅ Demo Complete!"
echo ""
echo "Pentru testare completă, rulează: ./tests/test_api.sh"
