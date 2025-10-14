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

file(GLOB SOURCE_FILES
    main.cpp
    imgui/*.cpp 
    imgui/*.h
    imgui/misc/cpp/imgui_stdlib.*
    imgui/backends/imgui_impl_glfw.*
    imgui/backends/imgui_impl_opengl3.*)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})



target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    
    ${CMAKE_CURRENT_SOURCE_DIR}/glad/include

    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends

    ${GLFW_INCLUDE_DIRS}
)


target_link_libraries(${PROJECT_NAME} PRIVATE
    glad
    glfw
    OpenGL::GL
    OpenGL::GLU
    GLUT::GLUT
)