project(TPG LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
cmake_minimum_required(VERSION 3.16)



Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0 # or a later release
)

FetchContent_MakeAvailable(Catch2)


include_directories(src/cpp/experiments/)
include_directories(src/cpp/environments/)
include_directories(src/cpp/TPG/)


 # src/cpp/TPG/TPG.h src/cpp/TPG/TPG.cc src/cpp/environments/TaskEnv.h src/cpp/TPG/RegisterMachine.h src/cpp/TPG/RegisterMachine.cc  src/cpp/TPG/Instruction.h src/cpp/TPG/Instruction.cc

add_executable(tests 
    src/cpp/TPG/tests/TPG_test.cc
)
# Add this line to find the Eigen package
find_package(Eigen3 3.3 REQUIRED NO_MODULE)

target_link_libraries(tests PRIVATE Eigen3::Eigen)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain) 



include(CTest)
include(Catch)
catch_discover_tests(tests)