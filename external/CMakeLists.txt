add_subdirectory(glfwpp)

add_library(AcrylicCompositor "Win32-Acrylic-Effect/Acrylic Window/AcrylicCompositor.cpp" "Win32-Acrylic-Effect/Acrylic Window/AcrylicCompositor.h")
target_include_directories(AcrylicCompositor PUBLIC "Win32-Acrylic-Effect/Acrylic Window")