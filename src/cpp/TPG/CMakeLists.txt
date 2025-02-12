# Source files for the TPG library
file(GLOB TPG_SOURCES
    *.cc
    *.h
)

# Create the TPG library
add_library(TPG STATIC ${TPG_SOURCES})

# Include directories
target_include_directories(TPG
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
target_link_libraries(TPG
    PUBLIC
    Eigen3::Eigen
    Boost::boost
    Boost::iostreams
    MPI::MPI_CXX
    logging
)

# Set output directory for the library
set_target_properties(TPG PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
) 