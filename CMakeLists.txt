cmake_minimum_required(VERSION 3.10)

project(ShopliftingBoy)

set(CMAKE_C++_STANDARD 99)

find_package(glfw3 REQUIRED)
find_package(glew  REQUIRED)
find_package(Freetype  REQUIRED)
find_package(assimp REQUIRED)
find_package(glm REQUIRED)

add_executable(ShopliftingBoy main.cpp src/Mesh.h src/Mesh.cpp)

target_link_libraries(ShopliftingBoy PRIVATE glfw Freetype::Freetype GLEW::GLEW assimp::assimp glm::glm OpenGL32)