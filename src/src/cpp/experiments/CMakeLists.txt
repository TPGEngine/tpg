# Source files for experiments
file(GLOB EXPERIMENTS_SOURCES
    *.cc
    *.h
)

# Create the TPGExperimentMPI executable
add_executable(TPGExperimentMPI ${EXPERIMENTS_SOURCES})

# Include directories
target_include_directories(TPGExperimentMPI
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../TPG
    ${CMAKE_CURRENT_SOURCE_DIR}/../environments
    ${CMAKE_CURRENT_SOURCE_DIR}/../environments/classic_control
    ${CMAKE_CURRENT_SOURCE_DIR}/../environments/mujoco
    ${CMAKE_CURRENT_SOURCE_DIR}/../environments/sequence
    ${EIGEN3_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${MUJOCO}/include
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
)

# Set output directory for the executable
set_target_properties(TPGExperimentMPI PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin") 