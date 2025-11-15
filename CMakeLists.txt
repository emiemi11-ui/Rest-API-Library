cmake_minimum_required(VERSION 3.10)
project(RestAPIFramework CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ===== FRAMEWORK LIBRARY =====

# Collect all source files from Rest-API/src (the existing codebase)
file(GLOB_RECURSE REST_API_SOURCES
    Rest-API/src/core/*.cpp
    Rest-API/src/http/*.cpp
    Rest-API/src/sync/*.cpp
    Rest-API/src/ipc/*.cpp
    Rest-API/src/data/*.cpp
    Rest-API/src/utils/*.cpp
)

# Collect framework wrapper sources
file(GLOB FRAMEWORK_SOURCES
    framework/src/*.cpp
)

# Create the framework library
add_library(restapi STATIC
    ${REST_API_SOURCES}
    ${FRAMEWORK_SOURCES}
)

# Include directories for the framework
target_include_directories(restapi PUBLIC
    ${CMAKE_SOURCE_DIR}/framework/include
    ${CMAKE_SOURCE_DIR}/Rest-API/include
)

# Find and link SQLite3
find_package(SQLite3 REQUIRED)
if(SQLite3_FOUND)
    target_include_directories(restapi PRIVATE ${SQLite3_INCLUDE_DIRS})
    target_link_libraries(restapi PUBLIC ${SQLite3_LIBRARIES})
    message(STATUS "SQLite3 found: ${SQLite3_LIBRARIES}")
else()
    message(FATAL_ERROR "SQLite3 not found! Install with: sudo apt-get install libsqlite3-dev")
endif()

# Find and link OpenSSL
find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    target_include_directories(restapi PRIVATE ${OPENSSL_INCLUDE_DIR})
    target_link_libraries(restapi PUBLIC OpenSSL::Crypto)
    message(STATUS "OpenSSL found: ${OPENSSL_LIBRARIES}")
else()
    message(FATAL_ERROR "OpenSSL not found! Install with: sudo apt-get install libssl-dev")
endif()

# Link pthread and rt
target_link_libraries(restapi PUBLIC pthread rt)

message(STATUS "Framework library 'librestapi.a' configured")

# ===== EXAMPLES =====

# Example 1: Simple API
add_executable(example1_simple
    examples/example1_simple/main.cpp
)
target_link_libraries(example1_simple PRIVATE restapi)
target_include_directories(example1_simple PRIVATE
    ${CMAKE_SOURCE_DIR}/framework/include
)

# Example 3: IoT Sensors
add_executable(example3_iot
    examples/example3_iot/main.cpp
)
target_link_libraries(example3_iot PRIVATE restapi)
target_include_directories(example3_iot PRIVATE
    ${CMAKE_SOURCE_DIR}/framework/include
)

# Example 4: Banking
add_executable(example4_banking
    examples/example4_banking/main.cpp
)
target_link_libraries(example4_banking PRIVATE restapi)
target_include_directories(example4_banking PRIVATE
    ${CMAKE_SOURCE_DIR}/framework/include
)

# Example 5: Medical
add_executable(example5_medical
    examples/example5_medical/main.cpp
)
target_link_libraries(example5_medical PRIVATE restapi)
target_include_directories(example5_medical PRIVATE
    ${CMAKE_SOURCE_DIR}/framework/include
)

# Note: Example 2 (E-Commerce) will be added later when we migrate the existing code

# ===== LEGACY REST API (optional) =====
# Keep the original rest_api executable for backward compatibility
file(GLOB_RECURSE LEGACY_SOURCES
    Rest-API/src/*.cpp
)
add_executable(rest_api ${LEGACY_SOURCES})
target_include_directories(rest_api PRIVATE Rest-API/include)
target_link_libraries(rest_api PRIVATE ${SQLite3_LIBRARIES} OpenSSL::Crypto pthread rt)

message(STATUS "")
message(STATUS "╔════════════════════════════════════════════════════════════════╗")
message(STATUS "║  REST API FRAMEWORK - Build Configuration                     ║")
message(STATUS "╠════════════════════════════════════════════════════════════════╣")
message(STATUS "║  Framework Library:  librestapi.a                              ║")
message(STATUS "║  Examples:                                                     ║")
message(STATUS "║    - example1_simple     (port 8080)                           ║")
message(STATUS "║    - example3_iot        (port 8082)                           ║")
message(STATUS "║    - example4_banking    (port 8083)                           ║")
message(STATUS "║    - example5_medical    (port 8084)                           ║")
message(STATUS "║  Legacy:                                                       ║")
message(STATUS "║    - rest_api            (original E-Commerce API)             ║")
message(STATUS "╚════════════════════════════════════════════════════════════════╝")
message(STATUS "")
