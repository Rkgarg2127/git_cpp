cmake_minimum_required(VERSION 3.13)

# Define the project and set the C++ standard
project(git VERSION 1.0)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Collect source files
file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

# Define the executable
add_executable(git ${SOURCE_FILES})

# Find and link OpenSSL
find_package(OpenSSL REQUIRED)
target_link_libraries(git PRIVATE OpenSSL::SSL OpenSSL::Crypto)


# Add the libcurl library
find_package(CURL REQUIRED)
# Link libcurl to your project
target_link_libraries(git PRIVATE CURL::libcurl)

# Link zlib
target_link_libraries(git PRIVATE z)

