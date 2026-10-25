# Build-time asset import (SRX_AUTO_EXTRACT_ASSETS). Runs
# `shadowrun_graphics_extract <rom> <staging> --all-assets` into a scratch
# directory in the build tree, then copies the results into the source asset
# directory. Git-tracked files there (resources.tsv, catalog.tsv, curated
# dialogue text, reviewed art) are never overwritten. Without git, only
# missing files are added.
#
# Required: -DEXTRACTOR= -DROM= -DSTAGING= -DDEST= -DSTAMP=
# Optional: -DGIT_EXECUTABLE=
foreach(required EXTRACTOR ROM STAGING DEST STAMP)
    if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
        message(FATAL_ERROR "srx_extract_assets: ${required} is not set")
    endif()
endforeach()

file(REMOVE_RECURSE "${STAGING}")
file(MAKE_DIRECTORY "${STAGING}")
message(STATUS "Extracting game assets from ${ROM}")
execute_process(
    COMMAND "${EXTRACTOR}" "${ROM}" "${STAGING}" --all-assets
    RESULT_VARIABLE extract_result
    OUTPUT_VARIABLE extract_output
    ERROR_VARIABLE extract_output)
if(NOT extract_result EQUAL 0)
    message(FATAL_ERROR
        "shadowrun_graphics_extract failed (${extract_result}):\n${extract_output}")
endif()

set(have_tracked_list FALSE)
if(GIT_EXECUTABLE AND EXISTS "${GIT_EXECUTABLE}")
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" -c core.quotepath=off ls-files .
        WORKING_DIRECTORY "${DEST}"
        RESULT_VARIABLE git_result
        OUTPUT_VARIABLE tracked_output
        ERROR_QUIET)
    if(git_result EQUAL 0)
        set(have_tracked_list TRUE)
        string(REPLACE "\r" "" tracked_output "${tracked_output}")
        string(REPLACE ";" "\\;" tracked_output "${tracked_output}")
        string(REPLACE "\n" ";" tracked_lines "${tracked_output}")
        foreach(tracked IN LISTS tracked_lines)
            if(NOT tracked STREQUAL "")
                set("srx_tracked_${tracked}" TRUE)
            endif()
        endforeach()
    endif()
endif()

file(GLOB_RECURSE extracted RELATIVE "${STAGING}" "${STAGING}/*")
set(copied 0)
set(kept 0)
foreach(relative IN LISTS extracted)
    set(source "${STAGING}/${relative}")
    set(target "${DEST}/${relative}")
    if(EXISTS "${target}")
        if(NOT have_tracked_list OR DEFINED "srx_tracked_${relative}")
            math(EXPR kept "${kept} + 1")
            continue()
        endif()
        file(SHA256 "${source}" source_hash)
        file(SHA256 "${target}" target_hash)
        if(source_hash STREQUAL target_hash)
            continue()
        endif()
    endif()
    get_filename_component(target_dir "${target}" DIRECTORY)
    file(MAKE_DIRECTORY "${target_dir}")
    file(COPY_FILE "${source}" "${target}")
    math(EXPR copied "${copied} + 1")
endforeach()
file(REMOVE_RECURSE "${STAGING}")
message(STATUS "Assets: ${copied} written to ${DEST}; ${kept} existing files kept (git-tracked; all existing files when git is unavailable)")
file(TOUCH "${STAMP}")
