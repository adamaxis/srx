# Ghidra conversion source layout

The 248 conversion source files are now assigned to 28 behavioral build
modules. The assignment is explicit in
[`conversion-source-modules.tsv`](conversion-source-modules.tsv) and is
checked by `tools/audit_conversion_modules.ps1`. The leaf files remain
temporarily because each one has a focused source-trace test and a direct ROM
provenance comment; they are not the target native architecture and should be
merged into the listed module files as their shared state contracts are
verified.

`decompilation/ghidra_conversion/` is the offline, readable-C++ evidence
layer. It is not the native game and it is not an emulator. Its job is to
turn source-traced ROM behavior into typed contracts that can later be folded
into the native game one defended hill at a time.

The tree is organized by game ownership rather than by ROM address. The
28-module consolidation is the finer boundary used for eventual source merge:

| Domain | Owns | Current source files |
| --- | --- | ---: |
| `core` | shared input, timing, transfer, data, and source-boundary primitives | 16 |
| `entities` | entity records, movement, collision, animation, actions, and effects | 53 |
| `field` | field modes, terrain, viewport, HUD, and field transitions | 55 |
| `world` | world activity, world effects, and world-sequence behavior | 30 |
| `menus` | inventory, equipment, HUD/menu selection, and menu services | 21 |
| `cyberspace` | Matrix entry, navigation, programs, storage, and CyberInfo | 14 |
| `title_intro` | title, intro choreography, title effects, and reveal services | 33 |
| `session` | runners, runner records, roster, and save/restore state | 14 |
| `audio` | source audio command/service contracts | 3 |
| `rendering` | sprite records, display/resource boundaries, glyphs, and text | 9 |

Each domain has the same three-way shape:

```text
include/shadowrun_ghidra/<domain>/   typed public contracts
src/<domain>/                        readable implementations
tests/<domain>/                      focused source-trace evidence
```

The conversion CMake project builds one
`shadowrun_ghidra_conversion_modules` archive from all domain implementations
and a `shadowrun_ghidra_conversion_runtime` archive from the separate
`src/integration/` composition sources. The runtime archive links the domain
archive; it is the executable-facing Ghidra conversion runtime, not the native
game runtime.
The current 254 CTest cases (the runner plus 253 focused test executables)
remain separate so a regression still points at a specific behavior contract;
they are not proposed game-runtime libraries. The conversion archive is never
linked into the native game.

The standalone `shadowrun_ghidra_conversion_runner` links the conversion
runtime. The `src/integration/` entry points intentionally do not appear in
the 28-row leaf ownership map: they exercise several typed contracts together
to prove conversion ordering and handoffs, but they do not claim ownership of
their ROM behavior or become native game state. The startup integration
specifically composes direct-asset verification, boot/title setup, the New
Game/Load Game selector, save validation, and Joshua's archetype handoff; it
remains a conversion-runtime boundary rather than a native-port dependency.

New work belongs in the existing domain file that owns the behavior. A new
file is justified only for a real ownership boundary or a materially
different source contract, not merely because another ROM entry was found.
Related leaf functions should be merged as their shared state and behavior
become certain. ROM addresses remain provenance in comments, the export
ledger, and the scratchpad; they are not runtime names or memory dependencies.

The module map is intentionally not a mechanical “one module per ROM region”
split. For example, entity animation is separate from entity movement because
it owns cadence/resource selection rather than collision; menu inventory is
separate from shared HUD services because it mutates runner records; and title
stage choreography is separate from title/world effects because the latter
owns effect lifetimes and behavior handoffs. Those are the boundaries that
should become the eventual native source files.

Duplicate generic helpers are consolidated at the domain boundary. For
example, the field terrain work-region fill/copy operations now have one
canonical host contract instead of two conflicting implementations with
different argument orderings.
