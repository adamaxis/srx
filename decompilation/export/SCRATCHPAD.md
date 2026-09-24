# Shadowrun Genesis Ghidra scratchpad

This directory is the single checked-in Ghidra decompilation export for the US
Genesis ROM. It is a source-analysis reference only. The native port must use
typed modern C++ state and ordinary host rendering; it must not embed a 68000
interpreter, VDP emulation, ROM-address reads, or decompiler-shaped bytecode.

## Regeneration record

- ROM: `../input/shadowrun_genesis_us.bin`
- ROM SHA-256: `92A614491FC3EA12A6DF32459CCE08B48574B7B814E244C07EB860DC5C6EDB7E`
- Ghidra: 11.4.2
- Loader: patched `lab313ru/ghidra_sega_ldr`, upstream commit `aef1046`
- Language: `68000:BE:32:default` / compiler `default`
- Exported functions: 905
- Resolved direct call sites: 2,976
- Unresolved or indirect call sites: 16

The loader patch is recorded in `../tools/ghidra/ghidra_sega_ldr.patch`. The
headless exporter is `../tools/ghidra/ExportShadowrun.java`. There is no second
checked-in Ghidra project or versioned export directory.

## Export files

- `functions.tsv`: one row for every Ghidra function and its generated C file.
- `callgraph.tsv`: aggregate caller-to-callee edges for resolved direct calls.
- `call-sites.tsv`: every direct call plus every unresolved/indirect call site.
- `disassembly.s`: disassembly with resolved call comments such as
  `; call 00000436 FUN_00000436`.
- `ghidra-c/fn_*.cpp`: individual decompiler output, with the recovered direct
  callees repeated in each file header.

## Initial function notes

These are deliberately source-derived working labels, not claims that the
native runtime should copy the decompiler's types or address model.

| ROM entry | Behavior recovered | Native C++ owner | Verification | Status |
| --- | --- | --- | --- | --- |
| `0x00000236` `Reset` | Reset/boot path. Initializes VDP/Z80 state, clears VRAM regions, selects the boot/title path from hardware/header state, then enters the main coordinator. | Not assigned | ROM trace and generated `fn_00000236.cpp`; direct callees are `0x436`, `0x73A`, `0x75E`. | partial |
| `0x00000436` | Provisional VDP text/data transfer helper: forms a VDP command and streams bytes from an address register until a terminator while writing the destination port. | Not assigned | Generated `fn_00000436.cpp`; four call sites from `Reset`. | pending |
| `0x0000073A` | Timing helper: performs a delay loop and waits for a VDP status bit before returning. | Not assigned | Generated `fn_0000073A.cpp`; called by `Reset`. | pending |
| `0x0000075E` | Main runtime coordinator. Initializes persistent RAM flags, runs the repeated update/input/dispatch loop, and contains indirect dispatch through `A1`. | Not assigned | Generated `fn_0000075E.cpp`; 41 resolved direct edges and one indirect site at `0x0B42`. | partial |
| `0x00000BD2` | Frame/tick boundary helper: increments a counter and waits until the sampled timing value changes. | Not assigned | Generated `fn_00000BD2.cpp`; called throughout the main loop. | pending |
| `0x00000BF2` | Reads controller/status bits and records pending/edge flags in the shared input state. | Not assigned | Generated `fn_00000BF2.cpp`; called by the main coordinator and transition paths. | pending |
| `0x00000C2C` | Clears or folds the work RAM depending on the runtime sentinel, resets display state, clears VDP state, and invokes the low-level initialization helpers. | Not assigned | Generated `fn_00000C2C.cpp`; called first by `0x75E`. | pending |
| `0x0000131C` | Repeats the low-level update pair 17 times and raises the associated update flag. | Not assigned | Generated `fn_0000131C.cpp`; called by boot and multiple runtime paths. | pending |
| `0x0000195C` | Samples the controller helper, stores the current low-byte input value, derives changed/edge bits, and updates a lookup-derived state value. | Not assigned | Generated `fn_0000195C.cpp`; called repeatedly by the main loop. | pending |
| `0x00001A9E` | Initializes transition/timing state, runs a bounded update loop with input and state helpers, and exits through cleanup/transition code. | Not assigned | Generated `fn_00001A9E.cpp`; called from `0x75E`. | pending |
| `0x0005D746` | Bounded timed-scene/controller routine with state-position counters, frame ticking, and periodic update/cleanup calls. Exact scene identity remains open. | Not assigned | Generated `fn_0005D746.cpp`; called from `0x75E`. | pending |

## Call annotation policy

Direct calls are annotated mechanically with the target address and current
Ghidra symbol. Indirect calls stay explicit and are not guessed into a native
function name. The current unresolved sites are recorded in `call-sites.tsv`
at `0x0B42`, `0x4658`, `0xC444`, `0x11884`, `0x15928`, `0x1640A`, `0x19A1C`,
`0x1B892`, `0x1E728`, `0x1EF28`, `0x1F26E`, `0x1F58A`, `0x24E32`, `0x53AA4`,
`0x54960`, and `0x5AF48`. Resolve these only when ROM/data-flow evidence
identifies the target; do not replace them with guessed aliases.

The Ghidra project used for regeneration is the existing `../ShadowRunGenesis.rep`.
Do not create versioned or parallel decompilation directories.
