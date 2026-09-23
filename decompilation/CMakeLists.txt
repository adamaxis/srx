cmake_minimum_required(VERSION 3.20)
project(shadowrun_rom_analysis LANGUAGES CXX)

option(SHADOWRUN_QT_FRONTEND
       "Build the Qt Widgets native frontend (requires Qt 6 Widgets)" OFF)

enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(shadowrun_rom_analyzer
    src/rom_analyzer.cpp
)

add_library(shadowrun_port STATIC
    src/boot.cpp
    src/compression.cpp
    src/entity_projection.cpp
    src/entity_state.cpp
    src/graphics.cpp
    src/intro_scene.cpp
    src/memory_bus.cpp
    src/native_input.cpp
    src/native_port.cpp
    src/object_mask.cpp
    src/object_list.cpp
    src/object_update.cpp
    src/palette.cpp
    src/resource_setup.cpp
    src/runtime.cpp
    src/session.cpp
    src/terrain.cpp
    src/z80_transport.cpp
)
target_include_directories(shadowrun_port PUBLIC include)

add_executable(shadowrun_boot_reconstruction_test
    tests/boot_reconstruction_test.cpp
)
target_link_libraries(shadowrun_boot_reconstruction_test PRIVATE shadowrun_port)
add_test(NAME shadowrun_boot_reconstruction COMMAND shadowrun_boot_reconstruction_test)

add_executable(shadowrun_native_port_test
    tests/native_port_test.cpp
)
target_link_libraries(shadowrun_native_port_test PRIVATE shadowrun_port)
target_compile_definitions(shadowrun_native_port_test PRIVATE
    SHADOWRUN_TEST_ROM="${CMAKE_CURRENT_SOURCE_DIR}/input/shadowrun_genesis_us.bin")
add_test(NAME shadowrun_native_port COMMAND shadowrun_native_port_test)

add_executable(shadowrun_graphics_test
    tests/graphics_test.cpp
)
target_link_libraries(shadowrun_graphics_test PRIVATE shadowrun_port)
target_compile_definitions(shadowrun_graphics_test PRIVATE
    SHADOWRUN_TEST_ROM="${CMAKE_CURRENT_SOURCE_DIR}/input/shadowrun_genesis_us.bin")
add_test(NAME shadowrun_graphics COMMAND shadowrun_graphics_test)

add_executable(shadowrun_menu_system_test
    tests/menu_system_test.cpp
)
target_link_libraries(shadowrun_menu_system_test PRIVATE shadowrun_port)
target_compile_definitions(shadowrun_menu_system_test PRIVATE
    SHADOWRUN_TEST_ROM="${CMAKE_CURRENT_SOURCE_DIR}/input/shadowrun_genesis_us.bin")
add_test(NAME shadowrun_menu_system COMMAND shadowrun_menu_system_test)

add_executable(shadowrun_menu_visual_regression_test
    tests/menu_visual_regression_test.cpp
)
target_link_libraries(shadowrun_menu_visual_regression_test PRIVATE shadowrun_port)
target_compile_definitions(shadowrun_menu_visual_regression_test PRIVATE
    SHADOWRUN_TEST_ROM="${CMAKE_CURRENT_SOURCE_DIR}/input/shadowrun_genesis_us.bin"
    SHADOWRUN_VISUAL_ARTIFACT_DIR="${CMAKE_CURRENT_BINARY_DIR}/visual-regression")
add_test(NAME shadowrun_menu_visual_regression COMMAND shadowrun_menu_visual_regression_test)

add_executable(shadowrun_visual_dump
    src/visual_dump.cpp
)
target_link_libraries(shadowrun_visual_dump PRIVATE shadowrun_port)

add_executable(shadowrun_boot_sequence_test
    tests/boot_sequence_test.cpp
)
target_link_libraries(shadowrun_boot_sequence_test PRIVATE shadowrun_port)
target_compile_definitions(shadowrun_boot_sequence_test PRIVATE
    SHADOWRUN_TEST_ROM="${CMAKE_CURRENT_SOURCE_DIR}/input/shadowrun_genesis_us.bin"
    SHADOWRUN_VISUAL_ARTIFACT_DIR="${CMAKE_CURRENT_BINARY_DIR}/visual-regression")
add_test(NAME shadowrun_boot_sequence COMMAND shadowrun_boot_sequence_test)

add_executable(shadowrun_leftover_clear_test
    tests/leftover_clear_test.cpp
)
target_link_libraries(shadowrun_leftover_clear_test PRIVATE shadowrun_port)
target_compile_definitions(shadowrun_leftover_clear_test PRIVATE
    SHADOWRUN_ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/assets"
    SHADOWRUN_VISUAL_ARTIFACT_DIR="${CMAKE_CURRENT_BINARY_DIR}/visual-regression")
add_test(NAME shadowrun_leftover_clear COMMAND shadowrun_leftover_clear_test)

add_executable(shadowrun_graphics_extract
    src/graphics_extract.cpp
)
target_link_libraries(shadowrun_graphics_extract PRIVATE shadowrun_port)

add_library(shadowrun_audio_extract_lib STATIC
    src/audio_extract.cpp
)
target_include_directories(shadowrun_audio_extract_lib PUBLIC include)

add_executable(shadowrun_audio_extract
    src/audio_extract_main.cpp
)
target_link_libraries(shadowrun_audio_extract PRIVATE shadowrun_audio_extract_lib)

add_executable(shadowrun_audio_extract_test
    tests/audio_extract_test.cpp
)
target_link_libraries(shadowrun_audio_extract_test PRIVATE
    shadowrun_audio_extract_lib)
target_compile_definitions(shadowrun_audio_extract_test PRIVATE
    SHADOWRUN_TEST_ROM="${CMAKE_CURRENT_SOURCE_DIR}/input/shadowrun_genesis_us.bin")
add_test(NAME shadowrun_audio_extract COMMAND shadowrun_audio_extract_test)

add_executable(shadowrun_graphics_catalog_test
    tests/graphics_catalog_test.cpp
)
target_compile_definitions(shadowrun_graphics_catalog_test PRIVATE
    SHADOWRUN_ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/assets")
add_test(NAME shadowrun_graphics_catalog COMMAND shadowrun_graphics_catalog_test)

add_executable(shadowrun_native_asset_directory_test
    tests/native_asset_directory_test.cpp
)
target_link_libraries(shadowrun_native_asset_directory_test PRIVATE shadowrun_port)
target_compile_definitions(shadowrun_native_asset_directory_test PRIVATE
    SHADOWRUN_ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/assets")
add_test(NAME shadowrun_native_asset_directory
         COMMAND shadowrun_native_asset_directory_test)

set(SHADOWRUN_ROM "${CMAKE_CURRENT_SOURCE_DIR}/input/shadowrun_genesis_us.bin")
if (EXISTS "${SHADOWRUN_ROM}")
    # Extraction is an explicit import step. A normal compile must never
    # rewrite the checked-out asset directory or create a second copy of a
    # large aggregate pack in the build tree.
    add_custom_target(shadowrun_title_boot_assets
        COMMAND "${CMAKE_COMMAND}" -E make_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        COMMAND "$<TARGET_FILE:shadowrun_graphics_extract>"
                "${SHADOWRUN_ROM}"
                "${CMAKE_CURRENT_SOURCE_DIR}/assets"
                --title-boot-assets
        DEPENDS shadowrun_graphics_extract "${SHADOWRUN_ROM}"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/graphics.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/graphics_extract.cpp"
        COMMENT "Extracting individual title/boot assets from the US ROM"
        VERBATIM
    )
    add_custom_target(shadowrun_all_assets
        COMMAND "${CMAKE_COMMAND}" -E make_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        COMMAND "$<TARGET_FILE:shadowrun_graphics_extract>"
                "${SHADOWRUN_ROM}"
                "${CMAKE_CURRENT_SOURCE_DIR}/assets"
                --all-assets
        DEPENDS shadowrun_graphics_extract "${SHADOWRUN_ROM}"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/graphics.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/graphics_extract.cpp"
        COMMENT "Extracting the complete native graphics catalog from the US ROM"
        VERBATIM
    )
    add_custom_target(shadowrun_all_resources
        COMMAND "${CMAKE_COMMAND}" -E make_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        COMMAND "$<TARGET_FILE:shadowrun_graphics_extract>"
                "${SHADOWRUN_ROM}"
                "${CMAKE_CURRENT_SOURCE_DIR}/assets"
                --all-assets
        COMMAND "$<TARGET_FILE:shadowrun_audio_extract>"
                "${SHADOWRUN_ROM}"
                "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        DEPENDS shadowrun_graphics_extract shadowrun_audio_extract
                "${SHADOWRUN_ROM}"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/graphics.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/graphics_extract.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/audio_extract.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/audio_extract_main.cpp"
        COMMENT "Extracting all indexed graphics and Genesis sound resources"
        VERBATIM
    )
endif()

if (WIN32)
    add_executable(shadowrun_native
        src/native_main.cpp
    )
    target_link_libraries(shadowrun_native PRIVATE shadowrun_port user32 gdi32)

    if (SHADOWRUN_QT_FRONTEND)
        find_package(Qt6 REQUIRED COMPONENTS Widgets)
        # Keep the normal MinGW entry point. The installed Qt kit is static
        # and its Windows GUI entry-point archive targets a different MinGW
        # CRT; the QWidget itself is still a normal native window.
        add_executable(shadowrun_qt
            src/qt_native_main.cpp
        )
        target_link_libraries(shadowrun_qt PRIVATE
            shadowrun_port Qt6::Widgets
        )
        if (CMAKE_BUILD_TYPE STREQUAL "Release")
            find_program(SHADOWRUN_WINDEPLOYQT
                NAMES windeployqt windeployqt.exe
                HINTS "${CMAKE_PREFIX_PATH}/bin")
            if (SHADOWRUN_WINDEPLOYQT)
                add_custom_command(TARGET shadowrun_qt POST_BUILD
                    COMMAND "${SHADOWRUN_WINDEPLOYQT}"
                            --release --no-translations
                            "$<TARGET_FILE:shadowrun_qt>"
                    COMMENT "Deploying Qt 6 runtime beside shadowrun_qt.exe"
                    VERBATIM)
            endif()
            get_filename_component(SHADOWRUN_COMPILER_BIN
                "${CMAKE_CXX_COMPILER}" DIRECTORY)
            foreach(SHADOWRUN_RUNTIME_DLL
                    libstdc++-6.dll libgcc_s_seh-1.dll libwinpthread-1.dll)
                add_custom_command(TARGET shadowrun_qt POST_BUILD
                    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                            "${SHADOWRUN_COMPILER_BIN}/${SHADOWRUN_RUNTIME_DLL}"
                            "$<TARGET_FILE_DIR:shadowrun_qt>/${SHADOWRUN_RUNTIME_DLL}"
                    COMMENT "Deploying matching compiler runtime ${SHADOWRUN_RUNTIME_DLL}"
                    VERBATIM)
            endforeach()
        endif()
    endif()
endif()

if (MSVC)
    target_compile_options(shadowrun_rom_analyzer PRIVATE /W4 /permissive-)
    target_compile_options(shadowrun_port PRIVATE /W4 /permissive-)
else()
    target_compile_options(shadowrun_rom_analyzer PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(shadowrun_port PRIVATE -Wall -Wextra -Wpedantic)
endif()
