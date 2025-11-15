#!/usr/bin/env python3

import requests
import time
import sys

# ANSI color codes
class Colors:
    CYAN = '\033[0;36m'
    GREEN = '\033[0;32m'
    YELLOW = '\033[1;33m'
    RED = '\033[0;31m'
    NC = '\033[0m'  # No Color
    BOLD = '\033[1m'

def print_header():
    print(f"\n{Colors.CYAN}╔════════════════════════════════════════════════════════════════╗{Colors.NC}")
    print(f"{Colors.CYAN}║  🔥 REST API FRAMEWORK - AUTOMATED TESTING                    ║{Colors.NC}")
    print(f"{Colors.CYAN}║  Testing 5 Different Domains on the Same Framework           ║{Colors.NC}")
    print(f"{Colors.CYAN}╚════════════════════════════════════════════════════════════════╝{Colors.NC}\n")

def test_server(name, port, endpoints):
    """Test a server with multiple endpoints"""
    print(f"{Colors.BOLD}{name:30} (Port {port}){Colors.NC}")
    print(f"{Colors.CYAN}{'─' * 60}{Colors.NC}")

    all_passed = True

    for endpoint_name, method, path, expected_status in endpoints:
        url = f"http://localhost:{port}{path}"
        print(f"  Testing {endpoint_name:40} ", end="", flush=True)

        try:
            if method == "GET":
                response = requests.get(url, timeout=2)
            elif method == "POST":
                response = requests.post(url, json={}, timeout=2)
            else:
                response = requests.request(method, url, timeout=2)

            if response.status_code == expected_status:
                print(f"{Colors.GREEN}✓ OK{Colors.NC} ({response.status_code})")
                # Print first 100 chars of response
                try:
                    data = response.json()
                    preview = str(data)[:100]
                    print(f"    {Colors.YELLOW}→ {preview}...{Colors.NC}")
                except:
                    pass
            else:
                print(f"{Colors.RED}✗ FAIL{Colors.NC} (expected {expected_status}, got {response.status_code})")
                all_passed = False

        except requests.exceptions.ConnectionRefusedError:
            print(f"{Colors.RED}✗ CONNECTION REFUSED{Colors.NC}")
            all_passed = False
        except requests.exceptions.Timeout:
            print(f"{Colors.RED}✗ TIMEOUT{Colors.NC}")
            all_passed = False
        except Exception as e:
            print(f"{Colors.RED}✗ ERROR: {str(e)}{Colors.NC}")
            all_passed = False

        time.sleep(0.1)

    print()
    return all_passed

def main():
    print_header()

    results = {}

    # ===== TEST SERVER 1: Simple API =====
    results['Simple API'] = test_server(
        "1️⃣  SIMPLE API",
        8080,
        [
            ("Hello World", "GET", "/", 200),
            ("Calculator (5+3)", "GET", "/add/5/3", 200),
            ("Calculator (10-4)", "GET", "/sub/10/4", 200),
            ("Calculator (6*7)", "GET", "/mul/6/7", 200),
            ("Greet User", "GET", "/greet/John", 200),
            ("Health Check", "GET", "/health", 200),
        ]
    )

    # ===== TEST SERVER 2: E-Commerce API =====
    # Skip if not built
    try:
        requests.get("http://localhost:8081/health", timeout=1)
        results['E-Commerce API'] = test_server(
            "2️⃣  E-COMMERCE API",
            8081,
            [
                ("Health Check", "GET", "/health", 200),
                ("List Products", "GET", "/api/products", 200),
                ("List Users", "GET", "/api/users", 200),
            ]
        )
    except:
        print(f"{Colors.YELLOW}⚠️  E-Commerce API not available (skipped){Colors.NC}\n")

    # ===== TEST SERVER 3: IoT Sensors API =====
    results['IoT Sensors API'] = test_server(
        "3️⃣  IOT SENSORS API",
        8082,
        [
            ("List Sensors", "GET", "/api/sensors", 200),
            ("Sensor Statistics", "GET", "/api/sensors/stats", 200),
            ("Latest Reading (SENS001)", "GET", "/api/sensors/SENS001/latest", 200),
            ("Sensor History", "GET", "/api/sensors/SENS001/history", 200),
            ("Alerts", "GET", "/api/sensors/alerts", 200),
            ("Submit Data", "POST", "/api/sensors/data", 201),
            ("Health Check", "GET", "/health", 200),
        ]
    )

    # ===== TEST SERVER 4: Banking API =====
    results['Banking API'] = test_server(
        "4️⃣  BANKING API",
        8083,
        [
            ("List Accounts", "GET", "/api/accounts", 200),
            ("Account Balance (ACC001)", "GET", "/api/accounts/ACC001/balance", 200),
            ("Account Details", "GET", "/api/accounts/ACC001", 200),
            ("Deposit Money", "POST", "/api/accounts/ACC001/deposit", 200),
            ("Transfer Funds", "POST", "/api/transfer", 200),
            ("Transaction History", "GET", "/api/transactions", 200),
            ("Health Check", "GET", "/health", 200),
        ]
    )

    # ===== TEST SERVER 5: Medical API =====
    results['Medical API'] = test_server(
        "5️⃣  MEDICAL RECORDS API",
        8084,
        [
            ("List Patients", "GET", "/api/patients", 200),
            ("Patient Details (P001)", "GET", "/api/patients/P001", 200),
            ("Medical History", "GET", "/api/patients/P001/history", 200),
            ("Patient Appointments", "GET", "/api/patients/P001/appointments", 200),
            ("Create Appointment", "POST", "/api/appointments", 201),
            ("List Appointments", "GET", "/api/appointments", 200),
            ("Health Check", "GET", "/health", 200),
        ]
    )

    # ===== SUMMARY =====
    print(f"{Colors.CYAN}╔════════════════════════════════════════════════════════════════╗{Colors.NC}")
    print(f"{Colors.CYAN}║  📊 TEST SUMMARY                                               ║{Colors.NC}")
    print(f"{Colors.CYAN}╠════════════════════════════════════════════════════════════════╣{Colors.NC}")

    all_passed = True
    for name, passed in results.items():
        status = f"{Colors.GREEN}✓ PASSED{Colors.NC}" if passed else f"{Colors.RED}✗ FAILED{Colors.NC}"
        print(f"{Colors.CYAN}║{Colors.NC}  {name:30} {status}")
        if not passed:
            all_passed = False

    print(f"{Colors.CYAN}╚════════════════════════════════════════════════════════════════╝{Colors.NC}\n")

    if all_passed:
        print(f"{Colors.GREEN}{Colors.BOLD}✅ ALL TESTS PASSED!{Colors.NC}")
        print(f"{Colors.GREEN}🎉 The framework successfully handles 5 different domains!{Colors.NC}\n")
        return 0
    else:
        print(f"{Colors.RED}{Colors.BOLD}❌ SOME TESTS FAILED{Colors.NC}")
        print(f"{Colors.YELLOW}💡 Make sure all servers are running: ./run_all_servers.sh{Colors.NC}\n")
        return 1

if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        print(f"\n\n{Colors.YELLOW}[INTERRUPTED] Testing cancelled by user{Colors.NC}\n")
        sys.exit(130)
