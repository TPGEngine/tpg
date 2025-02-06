cmake_minimum_required(VERSION 3.15)
project(TPGProject)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wall -Wno-error -Wextra -Wno-unused-parameter")

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


#Unit testing - add Catch 2 library
Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0 # or a later release
)

FetchContent_MakeAvailable(Catch2)

add_executable(tests 
    tests/TPG_test.cc
)



file(GLOB TPG_SOURCES 
  "src/cpp/logging/metrics/mta/*.h"
  "src/cpp/logging/metrics/mta/*.cc"
  "src/cpp/TPG/*.h"
  "src/cpp/TPG/*.cc"
  "src/cpp/experiments/api_client.cc"
  "src/cpp/experiments/api_client.h"

  
)

list(REMOVE_ITEM TPG_SOURCES "src/cpp/experiments/TPGExperimentMPI.cc")


add_library(tpg ${TPG_SOURCES})

# Add this line to find the Eigen package
find_package(Eigen3 3.3 REQUIRED NO_MODULE)
find_package(CURL REQUIRED)

include_directories(${EIGEN3_INCLUDE_DIR})
target_include_directories(tpg PUBLIC 
                src/cpp/TPG
                src/cpp/environments
                src/cpp/experiments
                src/cpp/logging
                )
target_include_directories(tpg PUBLIC ${EIGEN3_INCLUDE_DIR})
target_link_libraries(tpg PRIVATE CURL::libcurl)



target_link_libraries(tests 
  PRIVATE
      tpg
      Eigen3::Eigen
      Catch2::Catch2WithMain
      logging
)

include(CTest)
include(Catch)
enable_testing()
catch_discover_tests(tests)


