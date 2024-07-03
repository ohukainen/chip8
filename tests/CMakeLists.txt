find_package(Catch2 3 REQUIRED)

add_executable(tests 
    tests.cpp
)

target_link_libraries(tests PRIVATE 
    Catch2::Catch2WithMain
    emulator-lib-chip8
)

install(TARGETS tests DESTINATION ${PROJECT_SOURCE_DIR}/install/bin)
