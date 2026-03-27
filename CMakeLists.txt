cmake_minimum_required(VERSION 3.16)

project(QuaternionSLERP)

set(CMAKE_CXX_STANDARD 17)

# Header-only library
add_library(quaternion INTERFACE)

target_include_directories(quaternion INTERFACE include)

# Demo program
add_executable(main src/main.cpp)

find_package(raylib REQUIRED)

target_link_libraries(main PRIVATE quaternion raylib)

enable_testing()

add_executable(test_quat tests/test_quat.cpp)
target_link_libraries(test_quat PRIVATE quaternion)

add_executable(test_vec3 tests/test_Vec3.cpp)
target_link_libraries(test_vec3 PRIVATE quaternion)

add_test(NAME QuaternionTests COMMAND test_quat)
add_test(NAME Vec3Tests COMMAND test_vec3)

find_package(raylib QUIET)
if(raylib_FOUND)
	add_executable(demo_interpolation demos/demo_interpolation.cpp)
	target_link_libraries(demo_interpolation PRIVATE quaternion raylib)
endif()

