cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_COMPILER nvcc)

project(test)
add_executable(test main.cpp)

include_directories(${PROJECT_SOURCE_DIR})
include_directories("${PROJECT_SOURCE_DIR}/hwacp")

add_subdirectory(triangleRenderer)
add_subdirectory(debugger)
add_subdirectory(transformer)
add_subdirectory(triangleClipper)
add_subdirectory(objLoader)
add_subdirectory(cameraPlanes)

# libs
list(APPEND LIBS sfml-graphics)
list(APPEND LIBS sfml-window)
list(APPEND LIBS sfml-system)
list(APPEND LIBS triangleRenderer)
list(APPEND LIBS transformer)
list(APPEND LIBS triangleClipper)
list(APPEND LIBS debugger)
list(APPEND LIBS objLoader)
list(APPEND LIBS cameraPlanes)
list(APPEND LIBS "${PROJECT_SOURCE_DIR}/hwacp/hwacp.a")
target_link_libraries(test ${LIBS})

# includes
list(APPEND INC "${PROJECT_SOURCE_DIR}")
list(APPEND INC "${PROJECT_BINARY_DIR}")
target_include_directories(test PUBLIC "${INC}")
