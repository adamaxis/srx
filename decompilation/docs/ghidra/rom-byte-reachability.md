# US ROM byte-level code reachability

This is a conservative recursive-descent disassembly of ROM bytes, not a complete control-flow proof. The walk starts from valid 68000 vector targets, follows direct branches and calls, follows indirect calls resolved by Ghidra or a discovered absolute pointer table, and follows discovered indirect jump tables. Other unresolved transfers are reported and their targets are not guessed. Data that is reached through an indirect edge can still decode as an instruction; review the unresolved list before using this as a completeness claim.

ROM size: **2,097,152 bytes**. SHA-256: `92a614491fc3ea12a6df32459cce08b48574b7b814e244c07eb860dc5c6edb7e`.
Vector table: `0x000000..0x0000FF`; `63` valid ROM vector slots resolve to `9` unique instruction roots: `0x000200`, `0x000208`, `0x000210`, `0x000218`, `0x000220`, `0x000228`, `0x00022E`, `0x000234`, `0x000236`.

## Reachability summary

| Measure | Result |
|---|---:|
| Decoded reachable instruction starts | 19,668 |
| Reachable instruction bytes (union) | 85,668 |
| Reachable bytes inside known function spans | 79,682 |
| Reachable bytes outside known function spans | **5,986** |
| Reachable ranges outside known function spans | **39** |
| Exported functions used for sized spans | 911 |
| Status-ledger-only entry addresses | 163 |
| Resolved indirect call targets followed | 25 |
| Absolute pointer dispatch tables discovered | 4 |
| Indirect JMP sites found in Ghidra listing | 2 |
| Unresolved indirect jump sites | 1 |
| Reachable unresolved indirect jumps | 0 |
| Unresolved indirect calls | 8 |
| Invalid/out-of-ROM control-flow targets | 0 |
| Vector/branch targets that Capstone could not decode | 0 |

Known function spans use the exact `entry` and `size` values in `decompilation/export/functions.tsv`. The status ledger contains entry addresses but no sizes; for ledger-only entries, only the decoded entry instruction is masked as known. No body length is inferred for those rows.

## Largest reachable ranges outside known functions

Ranges are half-open byte intervals in the calculations; the table displays inclusive end addresses.

| Rank | Start | End (inclusive) | Bytes |
|---:|---:|---:|---:|
| 1 | `0x0121E4` | `0x01253D` | 858 |
| 2 | `0x058394` | `0x0586C9` | 822 |
| 3 | `0x054486` | `0x054661` | 476 |
| 4 | `0x01283A` | `0x0129FD` | 452 |
| 5 | `0x012638` | `0x01275B` | 292 |
| 6 | `0x01EF34` | `0x01F045` | 274 |
| 7 | `0x00C2AA` | `0x00C375` | 204 |
| 8 | `0x05924E` | `0x059317` | 202 |
| 9 | `0x053BD0` | `0x053C8F` | 192 |
| 10 | `0x053B1A` | `0x053BCB` | 178 |
| 11 | `0x0578C4` | `0x05794D` | 138 |
| 12 | `0x013EF4` | `0x013F75` | 130 |
| 13 | `0x01F08C` | `0x01F105` | 122 |
| 14 | `0x05432A` | `0x0543A1` | 120 |
| 15 | `0x01216A` | `0x0121DF` | 118 |
| 16 | `0x0006C6` | `0x000739` | 116 |
| 17 | `0x053CEA` | `0x053D59` | 112 |
| 18 | `0x053F94` | `0x053FFB` | 104 |
| 19 | `0x054226` | `0x054285` | 96 |
| 20 | `0x0142B4` | `0x014311` | 94 |
| 21 | `0x053ABA` | `0x053B15` | 92 |
| 22 | `0x05407A` | `0x0540D5` | 92 |
| 23 | `0x054156` | `0x0541B1` | 92 |
| 24 | `0x0543A6` | `0x054401` | 92 |
| 25 | `0x005AC6` | `0x005B1D` | 88 |

## Discovered pointer dispatch tables

The analyzer recognizes the `lea table,An` → `movea.l (An),An` → indirect transfer shape and reads consecutive big-endian longword entries. A nearby explicit ROM index guard bounds a table when its compare and branch shape is recognized; those entries are accepted when the raw ROM bytes decode as a 68000 instruction, even if Ghidra has no listing row there. Without a recognized guard, entries must point to Ghidra-listed code starts and stop at the next discovered table base or first invalid pointer. These remain candidate targets, not a proof that every path is reachable.

| Transfer site | Kind | Table base | Candidate entries | Candidate code targets |
|---:|---|---:|---:|---|
| `0x011768` | jmp | `0x0D2A0A` | 5 | `0x011B58`, `0x00D01C`, `0x011950`, `0x00FBA6`, `0x00C2A6` |
| `0x011884` | jsr | `0x0D2A1E` | 4 | `0x012168`, `0x012634`, `0x012836`, `0x011040` |
| `0x01EF28` | jsr | `0x0F3E56` | 6 | `0x01F1FC`, `0x01EF2C`, `0x01EF2C`, `0x01F086`, `0x01F046`, `0x01F106` |
| `0x053AA4` | jsr | `0x15FD5C` | 23 | `0x053AB6`, `0x053B16`, `0x053B78`, `0x053BCC`, `0x053CEA`, `0x0049A0`, `0x0049A0`, `0x0049A0`, `0x0049A0`, `0x0049A0`, `0x053F8C`, `0x0049A0`, `0x054076`, `0x0049A0`, `0x054152`, `0x054222`, `0x0049A0`, `0x0049A0`, `0x054322`, `0x0543A2`, `0x0049A0`, `0x05447E`, `0x0545B6` |

## Unresolved indirect jumps

Indirect `JMP` instructions without a discovered table are listed here. Unreached sites are included because a preceding unresolved call can hide their callers.

| Reachability | Site | Instruction | Resolution |
|---|---:|---|---|
| not reached from known roots | `0x02510A` | `jmp (0x2510e,PC,D7w*0x1)` | target not statically bounded |

## Indirect calls

Confirmed call-site targets and discovered pointer-table entries are traversed; unresolved calls still have their return-site fallthrough explored.

| Site | Instruction | Target / status | Callee |
|---:|---|---|---|
| `0x00AAB0` | `jsr (a1)` | `0x00B49C` | resolve_entity_fallback_movement |
| `0x011884` | `jsr (a0)` | `0x011040` | dispatch target |
| `0x011884` | `jsr (a0)` | `0x012168` | dispatch target |
| `0x011884` | `jsr (a0)` | `0x012634` | dispatch target |
| `0x011884` | `jsr (a0)` | `0x012836` | dispatch target |
| `0x01597A` | `jsr (a2)` | `0x0049A0` | noop_effect_callback |
| `0x01EF28` | `jsr (a1)` | `0x01EF2C` | dispatch target |
| `0x01EF28` | `jsr (a1)` | `0x01F046` | dispatch target |
| `0x01EF28` | `jsr (a1)` | `0x01F086` | dispatch target |
| `0x01EF28` | `jsr (a1)` | `0x01F106` | dispatch target |
| `0x01EF28` | `jsr (a1)` | `0x01F1FC` | dispatch target |
| `0x053AA4` | `jsr (a1)` | `0x0049A0` | noop_effect_callback |
| `0x053AA4` | `jsr (a1)` | `0x053AB6` | dispatch target |
| `0x053AA4` | `jsr (a1)` | `0x053B16` | dispatch target |
| `0x053AA4` | `jsr (a1)` | `0x053B78` | dispatch target |
| `0x053AA4` | `jsr (a1)` | `0x053BCC` | dispatch target |
| `0x053AA4` | `jsr (a1)` | `0x053CEA` | dispatch target |
| `0x053AA4` | `jsr (a1)` | `0x053F8C` | dispatch target |
| `0x053AA4` | `jsr (a1)` | `0x054076` | dispatch target |
| `0x053AA4` | `jsr (a1)` | `0x054152` | dispatch target |
| `0x053AA4` | `jsr (a1)` | `0x054222` | dispatch target |
| `0x053AA4` | `jsr (a1)` | `0x054322` | dispatch target |
| `0x053AA4` | `jsr (a1)` | `0x0543A2` | dispatch target |
| `0x053AA4` | `jsr (a1)` | `0x05447E` | dispatch target |
| `0x053AA4` | `jsr (a1)` | `0x0545B6` | dispatch target |
| `0x000B42` | `jsr (a1)` | indirect/unresolved | — |
| `0x0046F6` | `jsr (a1)` | indirect/unresolved | — |
| `0x00C444` | `jsr (a0)` | indirect/unresolved | — |
| `0x015928` | `jsr (a2)` | indirect/unresolved | — |
| `0x01640A` | `jsr (a1)` | indirect/unresolved | — |
| `0x01F26E` | `jsr (a1)` | indirect/unresolved | — |
| `0x01F58A` | `jsr (a0)` | indirect/unresolved | — |
| `0x05AF48` | `jsr (a1)` | indirect/unresolved | — |

## Method and limits

- Read the 64 longword vector slots at `0x000000..0x0000FF`; slot zero is the initial stack pointer and is not treated as code.
- Decode reachable instructions from the raw big-endian ROM using Capstone's M68000 decoder. Follow direct conditional branches, DBcc branches, direct calls, and direct jumps. Calls keep their return-site fallthrough; unconditional jumps and returns do not.
- Follow Ghidra `call-sites.tsv` indirect-call rows only when `status=resolved` and the target is present. For simple absolute pointer tables, recognize `lea table,An` followed by `movea.l (An),An` and an indirect `jmp/jsr`. Read candidate longword targets to the count implied by a nearby compare-and-branch index guard when recognized (greater-than skip: inclusive maximum plus one; less-than branch into dispatch: comparison immediate); otherwise stop at the next discovered table base or first non-listed pointer. The full `disassembly.s` export inventories indirect JMP sites, including sites not reached because of an earlier unresolved call.
- Merge decoded instruction bytes, subtract exported function extents and the entry instruction for status-only entries, then merge the remaining adjacent bytes into ranges.
- This report does not claim completeness for indirect targets, computed dispatches without a recognized pointer table, invalid decode boundaries, or data/code ambiguities. Table entries are candidate targets and do not establish the runtime index domain.

Regenerate with `python3 decompilation/ghidra_conversion/tools/rom_reachability.py --write-report`; validate with `python3 decompilation/ghidra_conversion/tools/rom_reachability.py --check`.
