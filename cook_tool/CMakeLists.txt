cmake_minimum_required(VERSION 3.20)

set(TOOL_NAME cook_tool)

add_executable(${TOOL_NAME}
    src/main.cpp
)

target_link_libraries(${TOOL_NAME} PRIVATE
    AYResource
    AYStorage
    AYIO
)

# AYResource already pulls Assimp/etc.; keep tool lean.
if(MSVC)
    target_compile_options(${TOOL_NAME} PRIVATE /utf-8)
endif()
