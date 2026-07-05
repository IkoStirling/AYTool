cmake_minimum_required(VERSION 3.12)
project(package_tool)

set(TOOL_NAME package_tool)

set(SRC_FILES
    src/main.cpp
)

add_executable(${TOOL_NAME} ${SRC_FILES})

target_include_directories(${TOOL_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../AYStorage/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../AYIO/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../AYTime/include
)

target_link_libraries(${TOOL_NAME} PRIVATE
    AYStorage
    AYIO
    AYTime
)

find_package(zstd CONFIG REQUIRED)
find_package(lz4 CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

target_link_libraries(${TOOL_NAME} PRIVATE
    zstd::libzstd
    lz4::lz4
    unofficial::sqlite3::sqlite3
)