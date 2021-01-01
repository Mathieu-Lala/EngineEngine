add_library(engine_api SHARED)

include(GenerateExportHeader)
generate_export_header(engine_api)

configure_file(detail/Version.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/Version.hpp @ONLY)

target_sources(engine_api PRIVATE src/Engine/api.cpp)
target_link_libraries(
  engine_api
  PUBLIC project_options CONAN_PKG::entt CONAN_PKG::magic_enum CONAN_PKG::spdlog glfw_imgui_impl CONAN_PKG::glm
  PRIVATE project_warnings)
target_include_directories(engine_api PUBLIC include ${CMAKE_CURRENT_BINARY_DIR})
target_compile_definitions(engine_api PUBLIC MAGIC_ENUM_RANGE_MIN=0 MAGIC_ENUM_RANGE_MAX=512)
