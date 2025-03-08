# Get the MUJOCO path from the environment
if(DEFINED ENV{MUJOCO})
    set(MUJOCOPATH $ENV{MUJOCO})
else()
    message(FATAL_ERROR "MUJOCO environment variable is not set!")
endif()

# Locate the Mujoco library file in ${MUJOCOPATH}/lib
find_library(MUJOCOLIBRARY mujoco HINTS "${MUJOCOPATH}/lib")
if(NOT MUJOCOLIBRARY)
  message(FATAL_ERROR "Could not find the Mujoco library in ${MUJOCOPATH}/lib")
endif()

# Include directories
include_directories(
    ${MUJOCOPATH}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../engine
)

# Create a library for MuJoCo environments
file(GLOB MUJOCO_ENV_SOURCES *.cc *.h)
add_library(mujoco_environments STATIC ${MUJOCO_ENV_SOURCES})

# Link against MuJoCo
target_link_libraries(mujoco_environments
    ${MUJOCOLIBRARY}
    OpenGL::GL
    GLEW::GLEW
    glfw
)

# If coverage is enabled, add coverage flags to this target
if(ENABLE_COVERAGE)
    append_coverage_compiler_flags_to_target(mujoco_environments)
endif() 