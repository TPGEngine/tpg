project(ClassicControlHeaders)

file(GLOB CLASSIC_CONTROL_SOURCES
    *.h
)

include_directories(
    PUBLIC # Make these includes PUBLIC for dependent projects
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../TPG
    /usr/include/eigen3
    /usr/include  # General include path
)