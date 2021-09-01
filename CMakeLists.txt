cmake_minimum_required(VERSION 3.21)
project(glfw-transparent-blur)

add_executable(main main.cpp)
target_compile_features(main PRIVATE cxx_std_20)

add_subdirectory(external)
target_link_libraries(main PRIVATE GLFWPP AcrylicCompositor libglew_static Imgui)
target_compile_definitions(main PRIVATE GLEW_STATIC)