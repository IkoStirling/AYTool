cmake_minimum_required(VERSION 3.20)

set(TOOL_NAME import_tool)

add_executable(${TOOL_NAME}
    src/main.cpp
)

target_link_libraries(${TOOL_NAME} PRIVATE
    AYResourceImport
    AYIO
    AYLog
)

if(MSVC)
    target_compile_options(${TOOL_NAME} PRIVATE /utf-8)
endif()
