project(TPG LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
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



# add_library(tpg 
#  src/cpp/TPG/TPG.h
#  src/cpp/TPG/TPG.cc
#  src/cpp/TPG/RegisterMachine.h
#  src/cpp/TPG/RegisterMachine.cc
#  src/cpp/TPG/Instruction.h 
#  src/cpp/TPG/Instruction.cc
#  src/cpp/TPG/misc.cc
#  src/cpp/TPG/misc.h
#  src/cpp/TPG/team.cc
#  src/cpp/TPG/team.h
#)


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


# Find MuJoCo Library
set(MUJOCO_DIR "$ENV{MUJOCO}")  # Make sure MUJOCO is set
set(MUJOCO_LIB "${MUJOCO_DIR}/lib/libmujoco.so")
set(MUJOCO_INCLUDE "${MUJOCO_DIR}/include")

# Ensure MuJoCo is found
if(NOT EXISTS "${MUJOCO_LIB}")
    message(FATAL_ERROR "MuJoCo library not found at ${MUJOCO_LIB}. Ensure MuJoCo is installed and MUJOCO_PATH is set.")
endif()

include_directories(${MUJOCO_INCLUDE})

add_executable(mujoco_env_test tests/mujoco_env_test.cc)
add_executable(mujoco_inverted_pendulum_test tests/mujoco_inverted_pendulum_test.cc)

# Add this line to find the Eigen package
find_package(Eigen3 3.3 REQUIRED NO_MODULE)
find_package(CURL REQUIRED)

# Include Directories

include_directories(${EIGEN3_INCLUDE_DIR})
target_include_directories(tpg PUBLIC ${EIGEN3_INCLUDE_DIR})


# Link libraries to testing files
target_link_libraries(tpg PRIVATE CURL::libcurl)

target_link_libraries(tests 
  PRIVATE
      tpg
      Eigen3::Eigen
      Catch2::Catch2WithMain
)

target_link_libraries(tests PRIVATE Eigen3::Eigen)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain) 
target_include_directories(mujoco_inverted_pendulum_test PRIVATE src/cpp/environments/mujoco)

# Link MuJoCo with mujoco_tests
target_link_libraries(mujoco_env_test 
  PRIVATE
      ${MUJOCO_LIB}
      Catch2::Catch2WithMain
      tpg
)

target_link_libraries(mujoco_inverted_pendulum_test 
  PRIVATE
      Catch2::Catch2WithMain
      tpg
      ${MUJOCO_LIB}
)

include(CTest)
include(Catch)
enable_testing()

# Discover tests

catch_discover_tests(tests)
catch_discover_tests(mujoco_env_test)
catch_discover_tests(mujoco_inverted_pendulum_test)