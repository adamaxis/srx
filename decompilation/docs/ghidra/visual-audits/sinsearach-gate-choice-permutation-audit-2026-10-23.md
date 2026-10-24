# Sinsearach gate choice and return audit

Date: 2026-10-23  
ROM: US Genesis Shadowrun  
Scope: Scene 9 record 230/event 0; physical menu alternatives, text-page
continuations, and same-scene refusal return

## Source path and display inputs

ROM record 230 at `0x1B578E` reaches the mode-9/event-0 finalizer at `0x1FD80`.
The finalizer returns D6=-1 after selecting a root through `0x1FE38`, so the
generic event-table destination is suppressed. The root depends on story bit
18, Group Item bit 5, and story bit 20:

- hostile text at `0x11B5A0` when bit 18 is clear or Group Item bit 5 is set;
- friendly questions/elders/later root at `0x11B5BC` when both hostile gates
  are clear and bit 20 is clear;
- friendly Frosty/elders/later root at `0x11B5D8` when bit 20 is set.

Command `0x33` at `0x21CAA` selects the three-topic prompt at `0x11B610`.
Caleb and Frosty answer records at `0x11B648` and `0x11B664` share the
Another Question/Elders/Later choice order. Frosty's unavailable response
record at `0x11B5F4` loops on Frosty, then routes Elders to `0x11B6AC` or Later
through command `0x3A` at `0x21E9E`. The direct Lodge prompt is at `0x11B6AC`;
its Thanks choice reaches command `0x34` at `0x21CE4`. With bits 13/17/18/19/20
set and Group Item bit 5 plus story bit 30 clear, that selects Lady Gillian;
otherwise the ROM selects the escort/busy group at `0xF4D14`.

Command `0x39` at `0x21E8C` selects the gate-return-plus-fare root at
`0x11B6D4`; `0x3A` selects the fare-only root at `0x11B6EC`. Both display the
fixed 1,000-nuyen offer. Command `0x38` at `0x21E66` compares the live wallet,
debits exactly 1,000 only when affordable, and selects paid or short-wallet
text. The No path shows Joshua's response at `0x1050DC`, the guard reply at
`0x0F7152`, then returns in place. Race and class are not inputs to these
choice-root or fare predicates. The displayed fare is a ROM string; its
eligibility and debit come from the typed wallet action.

## Physical route permutations

The three Debug/Release Qt cases enter Scene 9 below record 230 and use
ordinary Up to reach the gate. Each verifies full-frame 320x224 output, the
expected page text, and exact visible choice-label order. The story flags are
set per source branch rather than inferred from one generic prompt.

| CTest route | Physical path | Captured branch output |
| --- | --- | --- |
| `...gate_questions_physical_qa` | Questions → Caleb → Another Question → Frosty question → Elders → Thanks → escort/busy → command `0x39` | Both answer pages and their labels, the looped question prompt, the Elders fallback pages, and the 1,000-nuyen return prompt |
| `...gate_frosty_elders_physical_qa` | Frosty → Frosty-left → Frosty self-loop → Elders → Thanks | Repeated Frosty-left page is pixel-identical; the Elders handoff reaches Lady Gillian under the source flags |
| `...gate_frosty_later_physical_qa` | Frosty → Frosty-left → self-loop → Later → No → Joshua response → guard response → return | The 1,000-nuyen prompt, both No pages, and clean Scene-9 field frame; wallet balance and story bit 30 remain unchanged |

The CMake target names have the `shadowrun_ghidra_conversion_qt_field_`
prefix. Across the three runs, all 27 Debug BMPs have byte-identical Release
counterparts. The question/answer pages, Frosty-left loop, Elders prompt,
fare refusal, and settled field return were inspected at native resolution.
The first no-fare return capture was black because QA sampled during the modal
fade; the test now drains the source-context return transition before writing
`actual.bmp`.

## Disposition

- **Code changed:** added three physical-input QA routes and full-frame capture
  assertions. No production dialogue or transaction behavior changed.
- **ROM behavior checked:** root selection inputs; ordered A/B/C menu labels;
  both question topics; question loop; answer follow-up; Frosty-left self-loop
  and both exits; direct and fallback Elders results; commands `0x39`/`0x3A`;
  fixed 1,000 fare text; No response and unchanged wallet on return.
- **Debug verification:** the three new `...questions_physical_qa`,
  `...frosty_elders_physical_qa`, and `...frosty_later_physical_qa` CTests
  pass.
- **Release verification:** the same three CTests pass; all 27 BMPs match
  their Debug hashes.
- **Independent visual evidence:** no matching Genesis encounter capture has
  been identified in the supplied longplay. These screenshots establish host
  route output only, not independent Genesis pixel fidelity.
- **Still open:** independent Genesis encounter visuals remain unavailable.
  Keep the gate hill partial.

See also [the complete gate and cinematic source audit](../cinematic-story-pages-source-audit.md)
and [the hill index](../HILLS.md).

## 2026-10-24 hostile-root physical audit

Raw US ROM disassembly of `0x1FE38..0x1FE84` confirms the selector reads
`FFFC1E` bit 18 first. When it is clear, ROM address `0x11B5A0` is passed to
the prompt renderer at `0x1F246`; when set, Group Item bit 5 in `FFFC22` can
also select that hostile root. With both hostile conditions clear, bit 20
selects `0x11B5BC` or `0x11B5D8`. The hostile root's menu catalog retains the
ROM Questions / Elders / Later order.

Added the ordinary-input route
`shadowrun_ghidra_conversion_qt_field_sinsearach_gate_hostile_root_physical_qa`.
It starts at the Scene-9 checkpoint, sets bit 18 clear and Group Item flags to
zero, walks Up into record 230/event 0, checks the exact hostile text
(`Hold it, round-ear!`) and all three ordered labels, then follows Questions,
both topic pages, the question loop, and the Elders/busy fallback. The primary
`actual.bmp` is the hostile root; eleven full-frame BMPs are emitted. Debug and
Release pass, and all eleven corresponding captures are byte-identical. The
reviewed 320x224 root capture shows the hostile text above the three-choice
panel. It is native-host evidence only: no independent Genesis encounter
capture has been found.
