cmake_minimum_required(VERSION 3.21)
project(SUDUC C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -fsanitize=address")

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED True)

file(GLOB HEADERS "${PROJECT_SOURCE_DIR}/include/*.h")
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*.c")

add_executable(sudu ${SOURCES} ${HEADERS})
target_include_directories(sudu PRIVATE ${PROJECT_SOURCE_DIR}/include)