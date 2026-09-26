# Running the native game

The native executable consumes the direct files under `decompilation/assets/`.
It never reads the ROM and it does not link `ghidra_conversion`.

From `decompilation/` with the MSYS2 UCRT64 Qt kit configured:

```text
cmake --preset qt-debug
cmake --build --preset debug-qt
build-qt-debug\shadowrun_qt.exe assets
```

The Win32 framebuffer frontend is also built by the Windows preset:

```text
build-qt-debug\shadowrun_native.exe assets
```

Controls are Arrow keys, `Z` = A, `X` = B, `C` = C, and Enter = Start.

The headless direct-asset proof is:

```text
ctest --test-dir build-qt-debug -R shadowrun_native_game_smoke --output-on-failure
```

That test loads the direct resource manifest, advances the native controller
through boot/title/start options/archetype/intro into the field, and verifies a
non-empty rendered frame. It is intentionally not a ROM/emulator test.
