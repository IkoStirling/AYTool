cmake_minimum_required(VERSION 3.20)

set(TOOL_NAME package_tool)

add_executable(${TOOL_NAME}
    src/main.cpp
)

target_include_directories(${TOOL_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Core libs live under AYFoundation (not AYRuntime/AYStorage).
target_link_libraries(${TOOL_NAME} PRIVATE
    AYStorage
    AYIO
)

if(MSVC)
    target_compile_options(${TOOL_NAME} PRIVATE /utf-8)
endif()
