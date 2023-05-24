cmake_minimum_required(VERSION 3.0.0)
project(test_code VERSION 0.1.0)

include(CTest)
enable_testing()

# Add the following lines to specify the OpenGL and GLUT libraries
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

include_directories(${PROJECT_SOURCE_DIR})
add_executable(test_code main.cpp)

# Link the required OpenGL and GLUT libraries
target_link_libraries(test_code PRIVATE OpenGL::GLU OpenGL::GL ${GLUT_LIBRARIES})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
