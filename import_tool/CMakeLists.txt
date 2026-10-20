cmake_minimum_required(VERSION 3.20)

set(TOOL_NAME import_tool)

add_executable(${TOOL_NAME}
    src/main.cpp
)
ay_stage_audio_cook_assets(${TOOL_NAME})

target_link_libraries(${TOOL_NAME} PRIVATE
    AYResourceImport
    AYIO
    AYLog
)

if(MSVC)
    target_compile_options(${TOOL_NAME} PRIVATE /utf-8)
endif()

# Module-local genuine process/codec checks. A fresh nonce keeps previous
# evidence and makes repeated test runs independent without deleting user data.
add_custom_target(AYTool_ImportRecipeChecks
    COMMAND "${CMAKE_COMMAND}" "-DIMPORT_TOOL=$<TARGET_FILE:import_tool>"
        "-DWORK_ROOT=${CMAKE_CURRENT_BINARY_DIR}/recipe-workflow"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/RecipeWorkflow.cmake"
    DEPENDS import_tool
    VERBATIM)
if(BUILD_TESTING)
    add_test(NAME AYTool_ImportRecipes
        COMMAND "${CMAKE_COMMAND}" "-DIMPORT_TOOL=$<TARGET_FILE:import_tool>"
            "-DWORK_ROOT=${CMAKE_CURRENT_BINARY_DIR}/recipe-workflow"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/RecipeWorkflow.cmake")
    set_tests_properties(AYTool_ImportRecipes PROPERTIES LABELS "tooling;import" TIMEOUT 90)
endif()
