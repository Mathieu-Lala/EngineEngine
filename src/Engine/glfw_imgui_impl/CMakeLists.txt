add_library(glfw_imgui_impl STATIC ${CONAN_RES_DIRS_IMGUI}/bindings/imgui_impl_glfw.cpp
                                   ${CONAN_RES_DIRS_IMGUI}/bindings/imgui_impl_opengl3.cpp)
target_link_libraries(glfw_imgui_impl PUBLIC project_options CONAN_PKG::glew CONAN_PKG::glfw CONAN_PKG::imgui)
target_compile_definitions(glfw_imgui_impl PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)
target_include_directories(glfw_imgui_impl PUBLIC ${CONAN_RES_DIRS_IMGUI}/bindings)
