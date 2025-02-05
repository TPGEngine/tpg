project(TPG LANGUAGES CXX)

cmake_minimum_required(VERSION 3.15)
project(TPGProject)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wall -Werror -Wextra -Wno-unused-parameter")

# Find required packages
find_package(MPI REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread mpi serialization iostreams)
find_package(Eigen3 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(CURL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(TBB REQUIRED)

# Add subdirectory for the src directory
add_subdirectory(src) 



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





file(GLOB TPG_SOURCES 
  "src/cpp/TPG/*.h"
  "src/cpp/TPG/*.cc"
  "src/cpp/experiments/api_client.cc"
  "src/cpp/experiments/api_client.h"

  
)

list(REMOVE_ITEM TPG_SOURCES "src/cpp/experiments/TPGExperimentMPI.cc")


add_library(tpg ${TPG_SOURCES})




target_include_directories(tpg
  PUBLIC 
    src
)

 # src/cpp/TPG/TPG.h src/cpp/TPG/TPG.cc src/cpp/environments/TaskEnv.h src/cpp/TPG/RegisterMachine.h src/cpp/TPG/RegisterMachine.cc  src/cpp/TPG/Instruction.h src/cpp/TPG/Instruction.cc

add_executable(tests 
    tests/TPG_test.cc
)
# Add this line to find the Eigen package
find_package(Eigen3 3.3 REQUIRED NO_MODULE)
find_package(CURL REQUIRED)

include_directories(${EIGEN3_INCLUDE_DIR})
target_include_directories(tpg PUBLIC ${EIGEN3_INCLUDE_DIR})
target_link_libraries(tpg PRIVATE CURL::libcurl)
# Link libraries to testing files

target_link_libraries(tests 
  PRIVATE
      tpg
      Eigen3::Eigen
      Catch2::Catch2WithMain
)
target_link_libraries(tests PRIVATE Eigen3::Eigen)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain) 



include(CTest)
include(Catch)
enable_testing()
catch_discover_tests(tests)



