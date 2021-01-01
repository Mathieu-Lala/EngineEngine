add_library(
  engine_core SHARED src/Engine/Core.cpp src/Engine/dll/Handle.cpp src/Engine/graphics/Window.cpp
                     src/Engine/graphics/Shader.cpp src/Engine/EventManager.cpp src/Engine/widget/ComponentTree.cpp)
target_include_directories(engine_core PUBLIC include)
target_link_libraries(engine_core PUBLIC engine_api project_warnings CONAN_PKG::nlohmann_json CONAN_PKG::stb
                                         CONAN_PKG::CLI11 CONAN_PKG::openal)
