cmake_minimum_required(VERSION 3.18)
project(FreeTreeDrawing)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(glad STATIC glad/glad.c)

target_include_directories(glad PRIVATE
    glad/include)

find_package(glfw3 REQUIRED)
find_package(GLUT REQUIRED)
find_package(OpenGL REQUIRED)


add_executable(${PROJECT_NAME} main.cpp)



target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    
    ${CMAKE_CURRENT_SOURCE_DIR}/glad/include

    ${GLFW_INCLUDE_DIRS}
)


target_link_libraries(${PROJECT_NAME} PRIVATE
    glad
    glfw
    OpenGL::GL
    OpenGL::GLU
    GLUT::GLUT
)