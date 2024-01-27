# Add each example
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.cpp)
add_executable(test_inheritance ${SOURCES})
target_include_directories(test_inheritance PUBLIC ${CMAKE_SOURCE_DIR}/include)