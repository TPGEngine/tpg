# Source files for the TPG library
file(GLOB TPG_SOURCES
    *.cc
    *.h
)

# Create the TPG library
add_library(engine STATIC ${TPG_SOURCES})

# Include directories
target_include_directories(engine
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../experiments
    ${CMAKE_CURRENT_SOURCE_DIR}/../environments
    ${CMAKE_CURRENT_SOURCE_DIR}/../logging
    ${CMAKE_CURRENT_SOURCE_DIR}/../logging/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../logging/loggers
    ${CMAKE_CURRENT_SOURCE_DIR}/../logging/metrics
    ${CMAKE_CURRENT_SOURCE_DIR}/../logging/storage
    /usr/include/eigen3
    ${Boost_INCLUDE_DIRS}
    /usr/include
)

# Link libraries
target_link_libraries(engine
    PUBLIC
    Eigen3::Eigen
    Boost::boost
    Boost::iostreams
    MPI::MPI_CXX
    logging
    yaml-cpp
)

# Set output directory for the library
set_target_properties(engine PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
) 