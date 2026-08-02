cmake_minimum_required(VERSION 3.20)

project(AYTool
    VERSION 0.1.0
    DESCRIPTION "AYTool — offline import / cook / pack / index CLIs (core logic in AYResource/AYStorage)"
)

# Offline EXEs only. Library APIs stay in AYResource / AYStorage.
add_subdirectory(package_tool)
add_subdirectory(index_tool)
add_subdirectory(cook_tool)
add_subdirectory(import_tool)

message(STATUS "AYTool v0.1.0 (import_tool + cook_tool + package_tool + index_tool) — core: AYImportJob / AYCookShip")