# Ghidra Genesis loader

The ROM analysis uses the Sega Mega Drive / Genesis loader from
[`lab313ru/ghidra_sega_ldr`](https://github.com/lab313ru/ghidra_sega_ldr), pinned
for this workspace to upstream commit `aef1046` and built against Ghidra 11.4.2.
The local corrections are recorded in `ghidra_sega_ldr.patch` rather than
vendoring a second copy of Ghidra or maintaining parallel projects.

The patch makes the loader suitable for this headless, standard-Genesis import:

- selects Ghidra's actual `68000:BE:32:default` language instead of 68020;
- removes nondeterministic 32X/Sega CD prompts from automated imports; and
- does not create a function for vector 0, which is the initial supervisor stack
  pointer rather than executable code.

Build and install the patched plugin with Gradle and the local Ghidra install,
then let the loader auto-detect the ROM. The checked-in export script writes a
single flat export containing `functions.tsv`, `callgraph.tsv`,
`call-sites.tsv`, annotated `disassembly.s`, and one `ghidra-c/fn_*.cpp` per
function. Its output is source-analysis evidence only; the native port does not
consume the decompiler output at runtime.
