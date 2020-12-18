add_library(engine_api SHARED)

include(GenerateExportHeader)
generate_export_header(engine_api)

configure_file(detail/Version.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/Version.hpp @ONLY)

target_sources(engine_api PRIVATE src/Engine/api.cpp)
target_link_libraries(
  engine_api
  PUBLIC project_options
  PRIVATE project_warnings)
target_include_directories(engine_api PUBLIC include ${CMAKE_CURRENT_BINARY_DIR})
