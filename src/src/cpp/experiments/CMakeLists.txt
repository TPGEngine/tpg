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

# Source files for experiments
file(GLOB EXPERIMENTS_SOURCES
    *.cc
    *.h
    ../environments/*.cc
    ../environments/*.h
    ../environments/mujoco/MujocoEnv.h
    runners/*.cc
    runners/*.h
)

# Create the TPGExperimentMPI executable
add_executable(TPGExperimentMPI ${EXPERIMENTS_SOURCES})

# Include directories
target_include_directories(TPGExperimentMPI
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/runners
    ${CMAKE_CURRENT_SOURCE_DIR}/evaluators
    ${CMAKE_CURRENT_SOURCE_DIR}/../TPG
    ${CMAKE_CURRENT_SOURCE_DIR}/../environments
    ${CMAKE_CURRENT_SOURCE_DIR}/../environments/classic_control
    ${CMAKE_CURRENT_SOURCE_DIR}/../environments/mujoco
    ${CMAKE_CURRENT_SOURCE_DIR}/../environments/sequence
    ${EIGEN3_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${MUJOCOPATH}/include
)

# Link libraries
target_link_libraries(TPGExperimentMPI
    PRIVATE
    TPG
    MPI::MPI_CXX
    Boost::thread
    Boost::system
    Boost::mpi
    Boost::serialization
    Boost::iostreams
    ${OpenCV_LIBS}
    CURL::libcurl
    ${CMAKE_THREAD_LIBS_INIT}
    glfw
    GLEW::GLEW
    OpenGL::GL
    ${MUJOCOLIBRARY}
    TBB::tbb
    glut
    GLU
    OSMesa
)

# Set output directory for the executable to match the expected SCons path
set_target_properties(TPGExperimentMPI PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/release/cpp/experiments") 