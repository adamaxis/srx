cmake_minimum_required(VERSION 3.20)
project(shadowrun_ide LANGUAGES CXX)

# This is an IDE-only umbrella project. The native port and the Ghidra
# conversion retain independent source trees, libraries, build outputs, and
# runtime responsibilities; this file only lets Visual Studio expose both
# target families in one CMake target/startup-item bar.
add_subdirectory(decompilation "${CMAKE_BINARY_DIR}/native")
add_subdirectory(decompilation/ghidra_conversion
                 "${CMAKE_BINARY_DIR}/ghidra_conversion")

# Give Visual Studio enough information to run the standalone conversion
# runner directly from its startup-item button. The runner remains an offline
# conversion executable and is not linked into shadowrun_port or any emulator.
set_property(TARGET shadowrun_ghidra_conversion_runner
    PROPERTY VS_DEBUGGER_COMMAND_ARGUMENTS
             "--assets \"${CMAKE_SOURCE_DIR}/decompilation/assets\"")
set_property(TARGET shadowrun_ghidra_conversion_runner
    PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
