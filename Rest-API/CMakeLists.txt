cmake_minimum_required(VERSION 3.10)
project(RestAPI CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES
    src/*.cpp
)

add_executable(rest_api ${SOURCES})
target_include_directories(rest_api PRIVATE include)

# Căutare și linkare SQLite3
find_package(SQLite3 REQUIRED)
if(SQLite3_FOUND)
    target_include_directories(rest_api PRIVATE ${SQLite3_INCLUDE_DIRS})
    target_link_libraries(rest_api PRIVATE ${SQLite3_LIBRARIES})
    message(STATUS "SQLite3 găsit: ${SQLite3_LIBRARIES}")
else()
    message(FATAL_ERROR "SQLite3 nu a fost găsit! Instalează cu: sudo apt-get install libsqlite3-dev")
endif()

# Căutare și linkare OpenSSL (pentru SHA256 hash)
find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    target_include_directories(rest_api PRIVATE ${OPENSSL_INCLUDE_DIR})
    target_link_libraries(rest_api PRIVATE OpenSSL::Crypto)
    message(STATUS "OpenSSL găsit: ${OPENSSL_LIBRARIES}")
else()
    message(FATAL_ERROR "OpenSSL nu a fost găsit! Instalează cu: sudo apt-get install libssl-dev")
endif()

# Linkare pthread pentru multi-threading
target_link_libraries(rest_api PRIVATE pthread)
