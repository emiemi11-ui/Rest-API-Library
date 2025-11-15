cmake_minimum_required(VERSION 3.10)
project(RestAPIFramework CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ===== FRAMEWORK LIBRARY =====

# Collect all infrastructure source files
file(GLOB_RECURSE INFRASTRUCTURE_SOURCES
    infrastructure/src/core/*.cpp
    infrastructure/src/http/*.cpp
    infrastructure/src/sync/*.cpp
    infrastructure/src/ipc/*.cpp
    infrastructure/src/data/*.cpp
    infrastructure/src/utils/*.cpp
)

# Collect framework wrapper sources
file(GLOB FRAMEWORK_SOURCES
    framework/src/*.cpp
)

# Create the framework library
add_library(restapi STATIC
    ${INFRASTRUCTURE_SOURCES}
    ${FRAMEWORK_SOURCES}
)

# Include directories for the framework
target_include_directories(restapi PUBLIC
    ${CMAKE_SOURCE_DIR}/framework/include
    ${CMAKE_SOURCE_DIR}/infrastructure/include
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

# Example 2: E-Commerce
file(GLOB_RECURSE ECOMMERCE_SOURCES
    examples/example2_ecommerce/src/**/*.cpp
)
add_executable(example2_ecommerce
    examples/example2_ecommerce/main.cpp
    ${ECOMMERCE_SOURCES}
)
target_link_libraries(example2_ecommerce PRIVATE restapi)
target_include_directories(example2_ecommerce PRIVATE
    ${CMAKE_SOURCE_DIR}/framework/include
    ${CMAKE_SOURCE_DIR}/infrastructure/include
    ${CMAKE_SOURCE_DIR}/examples/example2_ecommerce/include
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

message(STATUS "")
message(STATUS "╔════════════════════════════════════════════════════════════════╗")
message(STATUS "║  REST API FRAMEWORK - Build Configuration                     ║")
message(STATUS "╠════════════════════════════════════════════════════════════════╣")
message(STATUS "║  Framework Library:  librestapi.a                              ║")
message(STATUS "║  Examples:                                                     ║")
message(STATUS "║    - example1_simple     (port 8080)                           ║")
message(STATUS "║    - example2_ecommerce  (port 8080)                           ║")
message(STATUS "║    - example3_iot        (port 8082)                           ║")
message(STATUS "║    - example4_banking    (port 8083)                           ║")
message(STATUS "║    - example5_medical    (port 8084)                           ║")
message(STATUS "╚════════════════════════════════════════════════════════════════╝")
message(STATUS "")
