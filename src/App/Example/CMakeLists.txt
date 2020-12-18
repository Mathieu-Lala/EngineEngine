add_library(example SHARED src/Example/Module.cpp)
target_link_libraries(example PRIVATE engine_api project_warnings)
target_include_directories(example PRIVATE include)
