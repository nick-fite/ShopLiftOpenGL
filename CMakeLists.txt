cmake_minimum_required(VERSION 3.10)

project(ShopliftingBoy)

set(CMAKE_C++_STANDARD 99)

find_package(glfw3 REQUIRED)
find_package(glew  REQUIRED)
find_package(Freetype  REQUIRED)

add_executable(ShopliftingBoy main.cpp)

target_link_libraries(ShopliftingBoy PRIVATE glfw Freetype::Freetype GLEW::GLEW OpenGL32)