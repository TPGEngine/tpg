cmake_minimum_required(VERSION 3.15)
project(TPGProject)

# Set C++ standard and compile flags
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wall -Wno-error -Wextra -Wno-unused-parameter")

# Find required packages
find_package(MPI REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread mpi serialization iostreams)
find_package(OpenCV REQUIRED)
find_package(CURL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(TBB REQUIRED)
find_package(Eigen3 3.3 REQUIRED NO_MODULE)

# Add subdirectory for src
add_subdirectory(src)

# --- Unit Testing and Catch2 Setup ---
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0 # or a later release
)
FetchContent_MakeAvailable(Catch2)

# --- Executable Targets ---
add_executable(tests tests/TPG_test.cc)                       # RegisterMachineCrossover
add_executable(mujoco_env_test tests/mujoco_env_test.cc)        # MuJoCoEnv
add_executable(mujoco_inverted_pendulum_test tests/mujoco_inverted_pendulum_test.cc)         # Inverted Pendulum
add_executable(mujoco_inverted_double_pendulum_test tests/mujoco_inverted_double_pendulum_test.cc)   # Inverted Double Pendulum
add_executable(mujoco_hopper_test tests/mujoco_hopper_test.cc)    # Hopper
add_executable(mujoco_half_cheetah_test tests/mujoco_half_cheetah_test.cc)  # Half Cheetah
add_executable(mujoco_humanoid_standup_test tests/mujoco_humanoid_standup_test.cc)  # Humanoid Standup
add_executable(mujoco_ant_test tests/mujoco_ant_test.cc)          # Ant
add_executable(mujoco_reacher_test tests/mujoco_reacher_test.cc)  # Reacher

# --- Library Target (tpg) ---
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

# Combine include directories for tpg
target_include_directories(tpg PUBLIC 
  src
  src/cpp/TPG
  src/cpp/environments
  src/cpp/experiments
  src/cpp/logging
  ${EIGEN3_INCLUDE_DIR}
)
target_link_libraries(tpg PRIVATE CURL::libcurl)

# --- MuJoCo Setup ---
set(MUJOCO_DIR "$ENV{MUJOCO}")  # Ensure the MUJOCO environment variable is set
set(MUJOCO_LIB "${MUJOCO_DIR}/lib/libmujoco.so")
set(MUJOCO_INCLUDE "${MUJOCO_DIR}/include")
if(NOT EXISTS "${MUJOCO_LIB}")
    message(FATAL_ERROR "MuJoCo library not found at ${MUJOCO_LIB}. Ensure MuJoCo is installed and MUJOCO is set correctly.")
endif()
include_directories(${MUJOCO_INCLUDE})

# Additional include for MuJoCo environments
include_directories(src/cpp/environments/mujoco)

# --- Link Libraries to Executables ---

# For the non-MuJoCo test executable
target_link_libraries(tests 
  PRIVATE
    tpg
    Eigen3::Eigen
    Catch2::Catch2WithMain
)

# Define common libraries for all MuJoCo tests
set(COMMON_LIBS
  PRIVATE
    Catch2::Catch2WithMain
    tpg
    ${MUJOCO_LIB}
)

# List all MuJoCo test targets
set(MUJOCO_TEST_TARGETS
  mujoco_env_test
  mujoco_inverted_pendulum_test
  mujoco_inverted_double_pendulum_test
  mujoco_hopper_test
  mujoco_half_cheetah_test
  mujoco_humanoid_standup_test
  mujoco_ant_test
  mujoco_reacher_test
)

# Loop over each MuJoCo test target to link common libraries
foreach(target IN LISTS MUJOCO_TEST_TARGETS)
  target_link_libraries(${target} ${COMMON_LIBS})
endforeach()

# --- Testing ---
include(CTest)
include(Catch)
enable_testing()

# Discover tests for all targets
catch_discover_tests(tests)
catch_discover_tests(mujoco_env_test)
catch_discover_tests(mujoco_inverted_pendulum_test)
catch_discover_tests(mujoco_inverted_double_pendulum_test)
catch_discover_tests(mujoco_hopper_test)
catch_discover_tests(mujoco_half_cheetah_test)
catch_discover_tests(mujoco_humanoid_standup_test)
catch_discover_tests(mujoco_ant_test)
catch_discover_tests(mujoco_reacher_test)

# Custom target to run all tests automatically after build
add_custom_target(run_tests ALL
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running all tests..."
)