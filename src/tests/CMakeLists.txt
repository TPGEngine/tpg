# --- Unit Testing  ---
include(CTest)
include(Catch)
enable_testing()
find_package(TBB REQUIRED)

# --- Executable Targets ---
set(TEST_TARGETS
    register_machine_crossover_test
    team_crossover_test
    mujoco_env_test
    mujoco_inverted_pendulum_test 
    mujoco_inverted_double_pendulum_test
    mujoco_hopper_test
    mujoco_half_cheetah_test
    mujoco_humanoid_standup_test
    mujoco_ant_test
    mujoco_reacher_test
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

# Create test executables
foreach(test ${TEST_TARGETS})
    add_executable(${test} ${test}.cc)
endforeach()

# Set the runtime output directory for all the test executables so that
# Catch2 can locate them.
foreach(test ${TEST_TARGETS})
  set_target_properties(${test} PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
  )
endforeach()

# --- Library Target (tpg) ---
file(GLOB TPG_SOURCES 
  "../src/logging/metrics/mta/*.h"
  "../src/logging/metrics/mta/*.cc"
  "../src/engine/*.h"
  "../src/engine/*.cc"
  "../src/experiments/api_client.cc"
  "../src/experiments/api_client.h"
  "../src/experiments/evaluators/*.h"
  "../src/experiments/evaluators/*.cc"
)
list(REMOVE_ITEM TPG_SOURCES "../src/experiments/TPGExperimentMPI.cc")
add_library(tpg ${TPG_SOURCES})

# Combine include directories for tpg
target_include_directories(tpg PUBLIC 
  ../src
  ../src/engine
  ../src/environments
  ../src/experiments
  ../src/logging
  ../src/experiments/evaluators
  ${EIGEN3_INCLUDE_DIR}
)

target_link_libraries(tpg PRIVATE CURL::libcurl TBB::tbb)

# --- MuJoCo Setup ---
set(MUJOCO_DIR "$ENV{MUJOCO}")  # Ensure the MUJOCO environment variable is set
set(MUJOCO_LIB "${MUJOCO_DIR}/lib/libmujoco.so")
set(MUJOCO_INCLUDE "${MUJOCO_DIR}/include")

if(NOT EXISTS "${MUJOCO_LIB}")
    message(FATAL_ERROR "MuJoCo library not found at ${MUJOCO_LIB}. Ensure MuJoCo is installed and MUJOCO is set correctly.")
endif()

include_directories(${MUJOCO_INCLUDE})

# Additional include for MuJoCo environments
include_directories(../src/environments/mujoco)

# --- Link Libraries to Executables ---

# For the register machine crossover and team crossover test executable
target_link_libraries(register_machine_crossover_test 
  PRIVATE
    tpg
    Eigen3::Eigen
    Catch2::Catch2WithMain
    logging
)

target_link_libraries(team_crossover_test 
  PRIVATE
    tpg
    Eigen3::Eigen
    Catch2::Catch2WithMain
    logging
)


# Define common libraries for all MuJoCo tests
set(COMMON_LIBS
  PRIVATE
    Catch2::Catch2WithMain
    tpg
    ${MUJOCO_LIB}
)

# Loop over each MuJoCo test target to link common libraries
foreach(target IN LISTS MUJOCO_TEST_TARGETS)
  target_link_libraries(${target} ${COMMON_LIBS})
endforeach()

# --- Testing ---

# Create the directory for Catch2-generated include files.
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

# Discover tests for all targets
foreach(test ${TEST_TARGETS})
    catch_discover_tests(${test})
endforeach()

# Custom target to run all tests automatically after build
add_custom_target(run_tests ALL
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests
    COMMENT "Running all tests..."
)