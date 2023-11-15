cmake_minimum_required(VERSION 3.12)
project(DynaReMa)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the library headers
install(DIRECTORY include/ DESTINATION include)

# Add the examples
add_subdirectory(examples)