cmake_minimum_required(VERSION 3.20)

set(TOOL_NAME project_init_tool)

add_executable(${TOOL_NAME}
    src/main.cpp
)

target_link_libraries(${TOOL_NAME} PRIVATE
    AYProject
)

target_compile_features(${TOOL_NAME} PRIVATE cxx_std_20)
target_compile_definitions(${TOOL_NAME} PRIVATE
    AY_PROJECT_INIT_DEFAULT_ENGINE_SOURCE="${CMAKE_SOURCE_DIR}")

if(MSVC)
    target_compile_options(${TOOL_NAME} PRIVATE /utf-8 /W4)
else()
    target_compile_options(${TOOL_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
