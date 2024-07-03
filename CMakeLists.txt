cmake_minimum_required(VERSION 3.10)

project(emulator-lib-chip8 VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(EXECUTABLE_NAME chip8)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_library(${PROJECT_NAME} 
    src/chip8.cpp

    include/chip8.hpp
)

target_include_directories(${PROJECT_NAME} PUBLIC 
    ${PROJECT_SOURCE_DIR}/include
)

add_executable(${EXECUTABLE_NAME}
    src/main.cpp
    src/game.cpp

    include/game.hpp
)

target_include_directories(${EXECUTABLE_NAME} PUBLIC 
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(${EXECUTABLE_NAME} 
    ${PROJECT_NAME}
    ${SDL2_LIBRARIES}
)

target_compile_options(${EXECUTABLE_NAME} PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)

option(BUILD_TESTING "Building tests" ON)
if (BUILD_TESTING)
    add_subdirectory(tests)
endif() 

install(TARGETS ${PROJECT_NAME} DESTINATION ${PROJECT_SOURCE_DIR}/install/bin)
install(TARGETS ${EXECUTABLE_NAME} DESTINATION ${PROJECT_SOURCE_DIR}/install/bin)
