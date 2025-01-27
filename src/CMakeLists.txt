cmake_minimum_required(VERSION 3.15)
project(TPGProject)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wall -Werror -Wextra -Wno-unused-parameter")

# Find required packages
find_package(MPI REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread mpi serialization iostreams)
find_package(Eigen3 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(CURL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(TBB REQUIRED)

# Add subdirectory for the src directory
add_subdirectory(src) 