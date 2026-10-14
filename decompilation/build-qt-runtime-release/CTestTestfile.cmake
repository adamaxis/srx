# CMake generated Testfile for 
# Source directory: C:/Github/shadowrun/decompilation/ghidra_conversion
# Build directory: C:/Github/shadowrun/decompilation/build-qt-runtime-release
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[shadowrun_ghidra_conversion_runner]=] "C:/Github/shadowrun/decompilation/build-qt-runtime-release/shadowrun_ghidra_conversion_runner.exe" "--assets" "C:/Github/shadowrun/decompilation/ghidra_conversion/../assets")
set_tests_properties([=[shadowrun_ghidra_conversion_runner]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Github/shadowrun/decompilation/ghidra_conversion/CMakeLists.txt;98;add_test;C:/Github/shadowrun/decompilation/ghidra_conversion/CMakeLists.txt;0;")
subdirs("qt_runtime")
