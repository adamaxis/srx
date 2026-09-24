# Ghidra conversion workspace

This is the readable-C++ conversion area for behavior recovered from the
canonical Ghidra export. It is intentionally separate from both the native
game and the GenesisBus emulator:

- `decompilation/CMakeLists.txt` does not include this directory.
- It has no dependency on `GenesisBus`, ROM-addressed RAM, VDP objects, or the
  `shadowrun_port` library.
- Its functions use ordinary typed values and fixed-size host containers.
- Source addresses are provenance comments, not runtime lookup keys.

The first slice covers the controller edge expression at `0x195C`, the 64-word
palette operations at `0x1088`/`0x10D6`/`0x1368`, the seventeen-pass
transitions at `0x131C`/`0x12F6`, and the title state shared by `0x1A9E`,
`0x1BAC`, `0x1BEA`, `0x1C1E`, `0x1C6A`, `0x1E4E`, `0x2634`, and `0x26D0`. The `0x1A9E` prompt cadence, `0x1C1E`/`0x1C6A` text/sprite leaf state, and `0x1E4E` logo-column events are typed separately from unresolved writers, projection, and blitters. Vblank
waits, prompt drawing, resource setup, and unresolved hardware calls remain
explicit boundaries until their source inputs are understood.

Build this directory independently when validating the conversion slice:

```text
cmake -S decompilation/ghidra_conversion -B decompilation/ghidra_conversion/build
cmake --build decompilation/ghidra_conversion/build
ctest --test-dir decompilation/ghidra_conversion/build --output-on-failure
```
