# Production-Ready Load Test Client - Examples & Documentation

## Overview
Enhanced load test client with production-ready features for testing REST API servers on local or remote networks.

## Key Features

### 1. Network Connectivity
- ✅ Accept server IP and port via command line arguments
- ✅ Test servers on different machines in the network
- ✅ Configurable connection timeouts (5 seconds)
- ✅ Proper error handling for network failures

### 2. Realistic E-Commerce Test Scenarios
The client randomly selects from these realistic endpoints:
- `GET /api/products` - Browse all products
- `GET /api/products/search?q=laptop` - Search products
- `GET /api/products/1` - Get product details (ID 1)
- `GET /api/products/2` - Get product details (ID 2)
- `GET /api/products/5` - Get product details (ID 5)
- `POST /api/orders` - Create new order with JSON payload
- `GET /api/orders` - View all orders
- `GET /health` - Health check

### 3. Comprehensive Metrics
- **Request Statistics:**
  - Total requests sent
  - Successful vs failed requests
  - Success rate percentage
  - Test duration
  - Throughput (requests per second)

- **Response Time Analysis:**
  - Average response time
  - Minimum response time
  - Maximum response time
  - **p50 (median)** - 50% of requests faster than this
  - **p95** - 95% of requests faster than this
  - **p99** - 99% of requests faster than this

- **Status Code Distribution:**
  - Breakdown of HTTP status codes (200, 404, 500, etc.)
  - Percentage of each status code
  - Color-coded (green for 2xx, yellow for 4xx, red for 5xx)

- **Error Breakdown:**
  - Connection failed
  - Send failed
  - Receive timeout
  - Socket creation failed
  - Invalid address

### 4. Professional Output
- ✅ Real-time progress bar with percentage and counters
- ✅ Colored output (ANSI colors)
  - 🟢 Green: Success messages and 2xx status codes
  - 🔴 Red: Errors and 5xx status codes
  - 🟡 Yellow: Warnings and 4xx status codes
  - 🔵 Blue: Headers and banners
  - 🟣 Magenta: Percentile metrics
  - 🔵 Cyan: Section headers and progress
- ✅ Table-formatted results
- ✅ Optional CSV export for further analysis

### 5. Configuration Options
- Configurable test duration or total requests
- Configurable concurrency (threads)
- Realistic think time between requests (randomized)
- Multiple test modes

## Command Line Options

```bash
--server <ip>        Server IP address (default: 127.0.0.1)
--port <port>        Server port (default: 8080)
--threads <n>        Number of concurrent threads (default: 10)
--concurrency <n>    Alias for --threads
--requests <n>       Total number of requests (conflicts with --duration)
--duration <sec>     Test duration in seconds (conflicts with --requests)
--think-time <ms>    Think time between requests in ms (default: 100)
--csv <filename>     Save results to CSV file
--help               Show this help message
```

## Usage Examples

### Example 1: Test Remote Server with Duration
```bash
./load_test_client --server 192.168.1.100 --port 8080 --threads 10 --duration 60
```
This runs a 60-second test against server 192.168.1.100 with 10 concurrent threads.

### Example 2: Test with Specific Request Count
```bash
./load_test_client --server 10.0.0.5 --port 8080 --requests 10000 --concurrency 50
```
This sends exactly 10,000 requests using 50 concurrent threads.

### Example 3: Test Local Server with CSV Export
```bash
./load_test_client --server localhost --port 8080 --threads 20 --duration 120 --csv results.csv
```
This runs a 2-minute test with 20 threads and saves detailed results to CSV.

### Example 4: Quick Smoke Test
```bash
./load_test_client --server 127.0.0.1 --port 8080 --threads 5 --requests 100 --think-time 50
```
Quick test with 100 requests, 5 threads, and minimal think time.

### Example 5: High Concurrency Stress Test
```bash
./load_test_client --server 192.168.1.50 --port 8080 --concurrency 100 --duration 300
```
5-minute stress test with 100 concurrent connections.

## Example Output

### Successful Test Output
```
╔══════════════════════════════════════════════════════════════╗
║          PRODUCTION LOAD TEST CLIENT v2.0                    ║
╠══════════════════════════════════════════════════════════════╣
  Target:          192.168.1.100:8080
  Concurrency:     10 threads
  Total Requests:  1000
  Requests/Thread: 100
  Think Time:      100 ms
╚══════════════════════════════════════════════════════════════╝

Starting load test...

[==================================================] 100.0% (1000/1000)

╔══════════════════════════════════════════════════════════════╗
║              LOAD TEST RESULTS - SUMMARY                     ║
╠══════════════════════════════════════════════════════════════╣
  Test Configuration:
    Server:          192.168.1.100:8080
    Concurrency:     10 threads
    Think Time:      100 ms

  Request Statistics:
    Total Requests:  1000
    Successful:      950 (95.00%)
    Failed:          50 (5.00%)
    Duration:        25.3 seconds
    Throughput:      39.53 req/sec

  Response Time (ms):
    Average:         45.23 ms
    Min:             12 ms
    Max:             234 ms
    p50 (median):    42.00 ms
    p95:             89.00 ms
    p99:             156.00 ms

  Status Code Distribution:
    200:    850 (85.0%)
    201:     50 (5.0%)
    404:     30 (3.0%)
    500:     20 (2.0%)
    Network Error:     50 (5.0%)

  Error Breakdown:
    Connection failed: 30
    Receive timeout: 20
╚══════════════════════════════════════════════════════════════╝

  Results saved to: results.csv
```

### Duration-Based Test Output
```
╔══════════════════════════════════════════════════════════════╗
║          PRODUCTION LOAD TEST CLIENT v2.0                    ║
╠══════════════════════════════════════════════════════════════╣
  Target:          10.0.0.5:8080
  Concurrency:     20 threads
  Duration:        60 seconds
  Think Time:      100 ms
╚══════════════════════════════════════════════════════════════╝

Starting load test...

[===================================>              ] 70.0% (42/60s) | Requests: 8543

╔══════════════════════════════════════════════════════════════╗
║              LOAD TEST RESULTS - SUMMARY                     ║
╠══════════════════════════════════════════════════════════════╣
  Test Configuration:
    Server:          10.0.0.5:8080
    Concurrency:     20 threads
    Think Time:      100 ms

  Request Statistics:
    Total Requests:  12245
    Successful:      12150 (99.22%)
    Failed:          95 (0.78%)
    Duration:        60.1 seconds
    Throughput:      203.91 req/sec

  Response Time (ms):
    Average:         28.45 ms
    Min:             8 ms
    Max:             567 ms
    p50 (median):    25.00 ms
    p95:             52.00 ms
    p99:             98.00 ms

  Status Code Distribution:
    200:   11500 (93.9%)
    201:     450 (3.7%)
    404:     200 (1.6%)
    Network Error:     95 (0.8%)
╚══════════════════════════════════════════════════════════════╝
```

## CSV Export Format

When using `--csv`, results are saved in the following format:

```csv
Endpoint,Status Code,Latency (ms),Success,Error
/api/products,200,45,true,
/api/products/1,200,38,true,
/api/orders,201,67,true,
/api/products/5,404,23,false,
/api/products,0,0,false,Connection failed
/api/orders,200,52,true,
```

This can be imported into Excel, Google Sheets, or analysis tools for deeper insights.

## Performance Metrics Explained

### Percentiles
- **p50 (Median)**: Half of all requests completed faster than this time
- **p95**: 95% of requests completed faster (only 5% slower) - good for identifying outliers
- **p99**: 99% of requests completed faster (only 1% slower) - shows worst-case scenarios

### Throughput
Requests per second the server handled. Higher is better.

### Success Rate
Percentage of requests that received a 2xx HTTP status code.

## Building

### With CMake (recommended):
```bash
cd build
cmake ..
make load_test_client
./load_test_client --help
```

### Manual compilation:
```bash
g++ -std=c++17 -pthread -O2 -o load_test_client clients/load_test_client.cpp
```

## Technical Details

### Multi-threading
- Uses C++17 threads for true concurrency
- Thread-safe metrics collection with mutexes
- Atomic counters for real-time progress

### Network Implementation
- Raw TCP sockets (AF_INET, SOCK_STREAM)
- 5-second connection and receive timeouts
- Proper error handling for all network operations
- Socket reuse (new socket per request for connection pool simulation)

### Think Time
- Randomized between 50% and 200% of configured think time
- Simulates realistic user behavior
- Prevents thundering herd issues

### Measurements
- High-precision timing using `std::chrono::steady_clock`
- Millisecond accuracy for latency measurements
- Includes full request-response cycle

## Common Use Cases

### 1. Capacity Planning
Run duration-based tests with increasing concurrency to find server limits:
```bash
./load_test_client --server prod.example.com --port 8080 --threads 50 --duration 300
./load_test_client --server prod.example.com --port 8080 --threads 100 --duration 300
./load_test_client --server prod.example.com --port 8080 --threads 200 --duration 300
```

### 2. Performance Regression Testing
Run fixed request counts before/after code changes:
```bash
./load_test_client --server test.example.com --port 8080 --requests 5000 --threads 25 --csv baseline.csv
# Deploy changes
./load_test_client --server test.example.com --port 8080 --requests 5000 --threads 25 --csv after_changes.csv
```

### 3. Network Testing
Test servers on different network segments:
```bash
./load_test_client --server 192.168.1.10 --port 8080 --threads 10 --duration 60  # Same subnet
./load_test_client --server 10.0.5.20 --port 8080 --threads 10 --duration 60     # Different VLAN
./load_test_client --server 172.16.0.30 --port 8080 --threads 10 --duration 60   # VPN
```

### 4. Smoke Testing
Quick validation after deployment:
```bash
./load_test_client --server newly-deployed.example.com --port 8080 --requests 100 --threads 5
```

## Troubleshooting

### Connection Failed Errors
- Verify server is running: `telnet <server_ip> <port>`
- Check firewall rules
- Verify IP address and port
- Test with simple_client first

### Low Throughput
- Increase concurrency (`--threads`)
- Reduce think time (`--think-time`)
- Check network bandwidth
- Verify server isn't overloaded

### High p99 Latency
- Server may be experiencing resource contention
- Database queries might need optimization
- Check for disk I/O issues
- Review server logs for errors

## Enhancements Over Original

| Feature | Original | Enhanced |
|---------|----------|----------|
| Network Support | Localhost only | Any IP address |
| Command Line Args | Basic (2 args) | Full parser (8+ options) |
| Test Scenarios | Single endpoint | 8 realistic endpoints |
| Metrics | Basic (avg latency) | Comprehensive (15+ metrics) |
| Percentiles | None | p50, p95, p99 |
| Status Codes | Not tracked | Full distribution |
| Error Breakdown | Generic count | Detailed categorization |
| Progress Display | None | Real-time progress bar |
| Output Format | Basic text | Colored, formatted tables |
| CSV Export | No | Yes, optional |
| Test Modes | Requests only | Requests or duration |
| Think Time | None | Configurable, randomized |
| Documentation | Minimal | Comprehensive |

## License
Part of the Enterprise REST API project.
