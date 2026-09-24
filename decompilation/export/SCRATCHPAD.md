# Shadowrun Genesis Ghidra scratchpad

This directory is the single checked-in Ghidra decompilation export for the US
Genesis ROM. It is a source-analysis reference only. The native port must use
typed modern C++ state and ordinary host rendering; it must not embed a 68000
interpreter, VDP emulation, ROM-address reads, or decompiler-shaped bytecode.

Record each reviewed function here before porting it:

| ROM entry | Behavior recovered | Native C++ owner | Verification | Status |
| --- | --- | --- | --- | --- |
|  |  |  |  | pending |

The Ghidra project is `../ShadowRunGenesis.rep`. Regenerate this one export
from that project and `../input/shadowrun_genesis_us.bin` when Ghidra is
available. Do not create versioned or parallel decompilation directories.
