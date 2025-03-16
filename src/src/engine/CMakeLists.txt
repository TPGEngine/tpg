find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0)

# Source files for the TPG library
file(GLOB TPG_SOURCES
    *.cc
    *.h
    streaming/pipeline.cc
    streaming/pipeline.h
)

# Create the TPG library
add_library(engine STATIC ${TPG_SOURCES})

# Include directories
target_include_directories(engine
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/streaming
    ${CMAKE_CURRENT_SOURCE_DIR}/../experiments
    ${CMAKE_CURRENT_SOURCE_DIR}/../experiments/evaluators
    ${CMAKE_CURRENT_SOURCE_DIR}/../environments
    ${CMAKE_CURRENT_SOURCE_DIR}/../logging
    ${CMAKE_CURRENT_SOURCE_DIR}/../logging/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../logging/loggers
    ${CMAKE_CURRENT_SOURCE_DIR}/../logging/metrics
    ${CMAKE_CURRENT_SOURCE_DIR}/../logging/storage
    /usr/include/eigen3
    ${Boost_INCLUDE_DIRS}
    /usr/include
    ${GST_INCLUDE_DIRS}
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
    ${GST_LIBRARIES}
)

# Set output directory for the library
set_target_properties(engine PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
) 

if (ENABLE_LIVE_STREAMING)
    target_compile_definitions(engine PUBLIC ENABLE_LIVE_STREAMING)
endif()

# Debugging: Print the GStreamer include directories
message(STATUS "GStreamer include directories: ${GST_INCLUDE_DIRS}")

# Debugging: Print the GStreamer libraries
message(STATUS "GStreamer libraries: ${GST_LIBRARIES}")
