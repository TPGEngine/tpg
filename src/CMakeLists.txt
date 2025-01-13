cmake_minimum_required(VERSION 3.14)
project(TPG_Project)

# Enable testing
enable_testing()

# Find Catch2
find_package(Catch2 2.13.8 REQUIRED)

# Add your main TPG library
add_library(tpg_lib
    src/cpp/TPG/TPG.cc
    src/cpp/TPG/TPG.h
    # Add other source files here
)

# Add the test executable
add_executable(tpg_tests
    tests/TPG_test.cc
)

# Link the test executable with Catch2 and your library
target_link_libraries(tpg_tests
    PRIVATE
    Catch2::Catch2WithMain
    tpg_lib
)

# Register the test with CTest
add_test(NAME tpg_tests COMMAND tpg_tests)