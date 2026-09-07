cmake_minimum_required(VERSION 3.20)

set(TOOL_NAME cook_tool)

add_executable(${TOOL_NAME}
    src/main.cpp
)

target_link_libraries(${TOOL_NAME} PRIVATE
    AYResourceCook
    AYStorage
    AYIO
)

# Cook consumes the runtime metadata/package contract without importer SDKs.
if(MSVC)
    target_compile_options(${TOOL_NAME} PRIVATE /utf-8)
endif()
