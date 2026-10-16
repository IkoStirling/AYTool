cmake_minimum_required(VERSION 3.20)

project(AYTool
    VERSION 0.1.0
    DESCRIPTION "AYTool — offline import / cook / pack / index CLIs (core logic in AYResource/AYStorage)"
)

# Offline audio authoring Cook must remain usable when the tools are relocated.
function(ay_stage_audio_cook_assets target)
    set(_audio_samples "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../../EngineAssets/AYAudio/samples")
    if(EXISTS "${_audio_samples}")
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${_audio_samples}"
                "$<TARGET_FILE_DIR:${target}>/assets/AYAudio/samples"
            COMMENT "Stage piano Cook samples and attribution")
    endif()
endfunction()

# Offline EXEs only. Library APIs stay in AYResource / AYStorage.
add_subdirectory(package_tool)
add_subdirectory(index_tool)
add_subdirectory(cook_tool)
add_subdirectory(import_tool)
add_subdirectory(project_build_tool)
add_subdirectory(project_init_tool)


message(STATUS "AYTool v0.3.0 (project init/build + import/cook/package/index CLIs)")
