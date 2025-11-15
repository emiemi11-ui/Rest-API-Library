cmake_minimum_required(VERSION 3.10)

# Enable testing
enable_testing()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/infrastructure/include
    ${CMAKE_SOURCE_DIR}/framework/include
)

# ============================================================================
# UNIT TESTS
# ============================================================================

# Test Router
add_executable(test_router
    unit/test_router.cpp
    ${CMAKE_SOURCE_DIR}/infrastructure/src/http/router.cpp
    ${CMAKE_SOURCE_DIR}/infrastructure/src/http/request.cpp
)
target_link_libraries(test_router pthread)
add_test(NAME RouterTest COMMAND test_router)

# Test SharedQueue
add_executable(test_sharedqueue
    unit/test_sharedqueue.cpp
    ${CMAKE_SOURCE_DIR}/infrastructure/src/ipc/sharedqueue.cpp
    ${CMAKE_SOURCE_DIR}/infrastructure/src/ipc/sharedmemory.cpp
    ${CMAKE_SOURCE_DIR}/infrastructure/src/sync/semaphore.cpp
)
target_link_libraries(test_sharedqueue pthread rt)
add_test(NAME SharedQueueTest COMMAND test_sharedqueue)

# ============================================================================
# PERFORMANCE BENCHMARKS
# ============================================================================

# Throughput benchmark
add_executable(benchmark_throughput
    performance/benchmark_throughput.cpp
)
target_link_libraries(benchmark_throughput pthread)

# ============================================================================
# INTEGRATION TESTS
# ============================================================================

# (Integration tests will be added here)

# ============================================================================
# INSTALL TARGETS
# ============================================================================

install(TARGETS
    test_router
    test_sharedqueue
    benchmark_throughput
    DESTINATION ${CMAKE_BINARY_DIR}/tests
)

# ============================================================================
# CUSTOM TARGETS
# ============================================================================

# Run all unit tests
add_custom_target(run_unit_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS test_router test_sharedqueue
    COMMENT "Running all unit tests..."
)

# Run benchmarks
add_custom_target(run_benchmarks
    COMMAND benchmark_throughput
    DEPENDS benchmark_throughput
    COMMENT "Running performance benchmarks..."
)
