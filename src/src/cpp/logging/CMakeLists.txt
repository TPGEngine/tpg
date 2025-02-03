cmake_minimum_required(VERSION 3.15)
project(logging LANGUAGES CXX)

# Use C++23 as in the rest of the project
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# List the source files that comprise the logging library.
# (Note that we include core/event_dispatcher.tpp as in the SCons script.)
set(LOGGING_SOURCES
    storage/mta/mta_storage.cc
    storage/tms/timing_storage.cc
    loggers/mta/mta_logger.cc
    loggers/tms/timing_logger.cc
    metrics/mta/mta_metrics.cc
    metrics/mta/mta_metrics_builder.cc
    metrics/tms/timing_metrics.cc
    metrics/tms/timing_metrics_builder.cc
    core/event_dispatcher.tpp
)

# Create the logging library target.
# Choose STATIC (or SHARED, if preferred) to match your desired linkage.
add_library(logging STATIC ${LOGGING_SOURCES})

# Specify include directories relative to the logging folder.
# This mirrors the SCons "env.Append(CPPPATH = [...])"
target_include_directories(logging
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}                 # logging/
        ${CMAKE_CURRENT_SOURCE_DIR}/core            # logging/core
        ${CMAKE_CURRENT_SOURCE_DIR}/loggers         # logging/loggers
        ${CMAKE_CURRENT_SOURCE_DIR}/metrics         # logging/metrics
        ${CMAKE_CURRENT_SOURCE_DIR}/storage         # logging/storage
)
