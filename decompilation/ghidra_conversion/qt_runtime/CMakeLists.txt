find_package(Qt6 REQUIRED COMPONENTS Widgets)

# This is the runnable Qt host for the converted source contracts. It is a
# child of ghidra_conversion, not a target in the native-port CMake project.
# It deliberately does not link shadowrun_port, shadowrun_native,
# shadowrun_qt, GenesisBus, or the native asset decoder.
add_executable(shadowrun_ghidra_conversion_qt
    src/ghidra_conversion_qt_main.cpp
    src/field_scene_runtime.cpp
    src/field_command_overview_bridge.cpp
    src/genesis_image_compositor.cpp
)
target_include_directories(shadowrun_ghidra_conversion_qt
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
set_target_properties(shadowrun_ghidra_conversion_qt PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
target_link_libraries(shadowrun_ghidra_conversion_qt
    PRIVATE
        shadowrun_ghidra_conversion_runtime
        Qt6::Widgets
)

add_executable(shadowrun_ghidra_genesis_image_compositor_test
    tests/genesis_image_compositor_test.cpp
    src/genesis_image_compositor.cpp
)
target_include_directories(shadowrun_ghidra_genesis_image_compositor_test
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
set_target_properties(shadowrun_ghidra_genesis_image_compositor_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
target_link_libraries(shadowrun_ghidra_genesis_image_compositor_test
    PRIVATE Qt6::Gui)
add_test(NAME shadowrun_ghidra_genesis_image_compositor
         COMMAND shadowrun_ghidra_genesis_image_compositor_test)

add_executable(shadowrun_ghidra_field_command_overview_bridge_test
    tests/field_command_overview_bridge_test.cpp
    src/field_command_overview_bridge.cpp
)
target_include_directories(shadowrun_ghidra_field_command_overview_bridge_test
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(shadowrun_ghidra_field_command_overview_bridge_test
    PRIVATE shadowrun_ghidra_conversion_runtime)
add_test(NAME shadowrun_ghidra_field_command_overview_bridge
         COMMAND shadowrun_ghidra_field_command_overview_bridge_test)

if (WIN32)
    find_program(SHADOWRUN_GHIDRA_QT_WINDEPLOYQT
        NAMES windeployqt windeployqt.exe
        HINTS "${CMAKE_PREFIX_PATH}/bin")
    if (SHADOWRUN_GHIDRA_QT_WINDEPLOYQT)
        # The installed Qt MinGW kit supplies release Qt libraries and the
        # release qwindows platform plugin even for a Debug application build.
        # CMAKE_BUILD_TYPE describes our symbols, not the Qt ABI selected by
        # find_package(Qt6).
        set(SHADOWRUN_GHIDRA_QT_DEPLOY_CONFIGURATION --release)
        add_custom_command(TARGET shadowrun_ghidra_conversion_qt POST_BUILD
            COMMAND "${SHADOWRUN_GHIDRA_QT_WINDEPLOYQT}"
                    ${SHADOWRUN_GHIDRA_QT_DEPLOY_CONFIGURATION}
                    --no-translations --no-compiler-runtime
                    "$<TARGET_FILE:shadowrun_ghidra_conversion_qt>"
            COMMENT "Deploying Qt 6 ${CMAKE_BUILD_TYPE} runtime beside the Ghidra conversion frontend"
            VERBATIM)
    endif()

    get_filename_component(SHADOWRUN_GHIDRA_QT_COMPILER_BIN
        "${CMAKE_CXX_COMPILER}" DIRECTORY)
    foreach(SHADOWRUN_GHIDRA_QT_RUNTIME_DLL
            libstdc++-6.dll libgcc_s_seh-1.dll libwinpthread-1.dll)
        add_custom_command(TARGET shadowrun_ghidra_conversion_qt POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                    "${SHADOWRUN_GHIDRA_QT_COMPILER_BIN}/${SHADOWRUN_GHIDRA_QT_RUNTIME_DLL}"
                    "$<TARGET_FILE_DIR:shadowrun_ghidra_conversion_qt>/${SHADOWRUN_GHIDRA_QT_RUNTIME_DLL}"
            COMMENT "Deploying matching MSYS2 runtime ${SHADOWRUN_GHIDRA_QT_RUNTIME_DLL}"
            VERBATIM)
    endforeach()
    # Pre-RUNTIME_OUTPUT_DIRECTORY builds left a copy under qt_runtime/ that
    # ninja no longer updates. Remove it so a rebuild cannot launch the stale
    # binary.
    add_custom_command(TARGET shadowrun_ghidra_conversion_qt POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E rm -f
                "${CMAKE_CURRENT_BINARY_DIR}/shadowrun_ghidra_conversion_qt.exe"
        COMMENT "Removing leftover qt_runtime copy of the conversion frontend"
        VERBATIM)
endif()
