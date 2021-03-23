cmake_minimum_required(VERSION 3.5)

set(FLASHER_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)
set(PORT RASPBERRY_PI)

project(raspberry_flasher)
    
add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/Src/main.c ${CMAKE_CURRENT_LIST_DIR}/../common/example_common.c)

target_include_directories(${PROJECT_NAME} PUBLIC ../common/)

add_subdirectory(${FLASHER_DIR} ${CMAKE_BINARY_DIR}/flasher)

target_link_libraries(${PROJECT_NAME} PUBLIC flasher)

# This example only supports one target (ESP32) and uses specific binary to be flashed.
target_compile_definitions(${PROJECT_NAME} PUBLIC -DSINGLE_TARGET_SUPPORT=1)