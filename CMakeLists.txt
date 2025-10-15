cmake_minimum_required(VERSION 3.18)
project(FreeTreeDrawing)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_library(glad STATIC glad/glad.c)
target_include_directories(glad PRIVATE glad/include)

find_package(glfw3 REQUIRED)
find_package(GLUT REQUIRED)
find_package(OpenGL REQUIRED)

file(GLOB APP_SOURCES
    "main.cpp"
    "tree/*.cpp"
    "imgui/*.cpp"
    "imgui/misc/cpp/imgui_stdlib.cpp"
    "imgui/backends/imgui_impl_glfw.cpp"
    "imgui/backends/imgui_impl_opengl3.cpp"
)

add_executable(${PROJECT_NAME} ${APP_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
    ${GLFW_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/tree/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glad
    glfw
    OpenGL::GL
    OpenGL::GLU
    GLUT::GLUT
)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/tree.txt"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tree.txt"
)