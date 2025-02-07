cmake_minimum_required(VERSION 3.15)
project(logging LANGUAGES CXX)

# Use C++23 as in the rest of the project
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# List the source files that comprise the logging library.
# (Note that we include core/event_dispatcher.tpp as in the SCons script.)
set(LOGGING_SOURCES
    storage/selection/selection_storage.cc
    storage/timing/timing_storage.cc
    storage/replacement/replacement_storage.cc
    storage/removal/removal_storage.cc
    loggers/selection/selection_logger.cc
    loggers/timing/timing_logger.cc
    loggers/replacement/replacement_logger.cc
    loggers/removal/removal_logger.cc
    metrics/selection/selection_metrics.cc
    metrics/selection/selection_metrics_builder.cc
    metrics/timing/timing_metrics.cc
    metrics/timing/timing_metrics_builder.cc
    metrics/replacement/replacement_metrics_builder.cc
    metrics/removal/removal_metrics_builder.cc
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
