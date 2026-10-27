# Command-menu posture focus and combat bars

This review covers the `0x119CE` focus row, posture state changes, and the
Atk/Def values presented by the field command menu. The linked combat-bar
audit now includes the higher-mode negative `+0x56` spell-action path. The
separate overhead combat loop and AI posture decisions remain outside this
review.

## Evidence

- ROM path: `0x119CE` accepts Up on command pages 2 through 6; `0x11ABE` clamps
  posture byte `+0x4D` to 0..4 and refreshes the row; `0x11AE4` draws the
  posture label/state; the caller reaches `0x12CEE` to refresh attack and
  defense values.
- ROM layout: the focus label is at `0xCA9E` (120,168), 48x8; the five-state
  bar begins at `0xCAAC` (176,168). Horizontal changes request sound event
  `0x25`, mapped to resource 037 (2.134 seconds in the decoded asset). Up,
  Down, and Start do not request this sound.
- Internal frames: `decompilation/ghidra_conversion/build/conversion/qt/debug/qt_runtime/field-posture-qa-frames/`
  and corresponding Release output. The CLI `actual.bmp` is a real focused
  posture state. Frame QA compares the named focus asset, full command-page
  pixels, status, and combat-bar regions.
- Independent visuals: section-06 frame `002550` shows the Human Shaman's
  Mana Zap command page with `Damage None`, `Power Very Low`, Neutral posture,
  and Atk/Def strips. The nearest extracted bar rows are Atk 5 and Def 4;
  video compression prevents exact pixel comparison. No selected-posture
  focus or horizontal-change transition was identified independently.

## Eight verification questions

1. **Does it look correct? — Unknown against Genesis.** The internal focused
   frame places `POSTURE` at the expected row and keeps the five-state bar
   visible. No matching independent selected-posture Genesis frame is
   available for a pixel comparison.

2. **Does the visible look match the behavioral state? — Yes, internally.**
   Up opens focus without changing the active command page or posture. Left
   and Right change the active runner's posture among Full Defense, Med.
   Defense, Neutral, Med. Offense, and Full Offense. The posture value and
   both attack/defense values are asserted after changes; attempts past either
   endpoint leave state clamped at 0 or 4.

3. **Are the visuals themselves correct? — Yes against extracted/source
   layout; unknown against Genesis pixels.** QA matches the 48x8 named focus
   image at (120,168), checks the five-state bar at (176,168), and verifies
   that both 104x8 attack/defense strips change when posture changes. It also
   requires every pixel outside the focus row to remain unchanged when focus
   opens or closes.

4. **Should it animate, and how often? — No focus animation.** The source
   redraws the selected focus/state in its controller loop; it does not
   advance a focus animation counter. The Qt QA holds the page for 30 field
   frames and requires the focused image to remain identical. Posture/bar
   changes occur on accepted horizontal input edges.

5. **Is there a sound, and how long does it play? — Yes for posture changes.**
   Each accepted Left/Right step requests event `0x25`; Up focus, Down
   deselection, and Start return are silent. The decoded resource lasts
   2.134 seconds. Shared audio QA verifies decode, `PlayingState`, duration,
   and stop on the default output; output loopback for this exact menu route
   has not been recorded.

6. **Are the other visible parts of the screen correct? — Internally checked.**
   Across command pages 2 through 6, focus changes only its 48x8 row. The
   portrait, item slots, status, bars, and selected command page remain
   visible. Changing posture updates both combat strips while preserving the
   rest of the page.

7. **Do multiple frames support the interpretation? — Internally yes;
   independently unknown.** QA captures entry and exit on all five eligible
   command pages, a 30-frame hold, every posture value, both clamps, and the
   Down-plus-horizontal priority case. The independent longplay has no
   matching selected-posture example.

8. **Does the new UI replace the old UI cleanly? — Yes, internally.** Down
   returns each page to a pixel-identical pre-focus frame without a sound
   request. Start in focus draws the normal posture row and final combat bars,
   then exits the complete command menu on that same input; the resulting
   field frame is pixel-identical to the pre-menu capture. No old selector or
   command graphics remain over the returned screen.

## Spell action path added to this review

The source's negative `+0x56` branch uses the active D4EA0 spell row for its
title/detail and `+0x9D` for the Power label. The Qt field presenter now passes
the active runner's Sorcery, Magic cap, force, spell row, item catalog, and
C8AE0 selector into the special combat-bar calculation. Its visual QA starts
from the Gator Shaman record and checks Mana Zap at force 3 / Neutral posture:
Attack 6 (bar row 5), Defense 5 (bar row 4), `Damage None`, and `Power Very
Low`. The corresponding full-frame actual.bmp is byte-identical between
Debug and Release. The complete 13-profile and 260-case spell matrix is
documented in [the equipment combat-bar audit](equipment-combat-bar-profile-permutation-audit-2026-10-23.md).

Independent section-06 frame `002550` supports the Gator Mana Zap Force-3
command page (`Damage None`, `Power Very Low`); frames `000015`/`000020` show
the Force-3 / Neutral `Set Force & Posture` panel. They support nearby layout,
not pixel-perfect matches. They do not cover the Orc Shaman Mana Storm route,
Forces 1, 2, or 4, or the other class/race profiles. The Debug/Release
profile test now physically drives Mana Storm through Forces 1..4 and checks
the `Meager`, `Minor`, `Very Low`, and `Low` Power text, the live Force
counter, D4EA0 `+0x25` Damage row, the `0x10660` residual-driven `None`/bar
branch, and the formula-derived Attack 11..14 bar rows. Those added frames
are internal Qt captures; independent Genesis evidence is still open. See the
[Force-label sweep](equipment-combat-bar-profile-permutation-audit-2026-10-23.md#force-dependent-power-label-and-mana-storm-sweep-2026-10-26).

## Outcome

- **Code changed:** Lua `menu.route`, `screens.command`, and `screens.posture`
  own command navigation and posture focus. C++ passes the live spell action
  inputs into the special combat-bar calculation and paints the D4EA0 spell
  title and force details from source data. The Bear totem modifier's D4540
  word was corrected from `0x0202` to `0x0002`. Other child-screen behavior
  is still behind the explicit legacy adapter.
- **Behavior implemented:** Up focuses posture on pages 2..6; Down silently
  deselects; a single Start from focus exits the full command menu;
  Left/Right update typed posture, source labels, attack/defense bars, and
  event `0x25`, with source clamping.
- **Internally checked:** the combat-bar profile/permutation test,
  `shadowrun_ghidra_conversion_qt_field_posture_qa`,
  `shadowrun_ghidra_conversion_qt_field_spell_combat_bar_qa`, and
  `shadowrun_ghidra_conversion_qt_field_magic_spell_qa` passed in Debug and
  Release. The shared audio test separately verifies event `0x25` playback.
- **Independently visually checked:** section-06 frame `002550` supports the
  Mana Zap action details and nearest bar rows. No matching Genesis
  selected-posture sequence or route-specific audio loopback was found.
- **Reported behavior resolved:** the requested menu navigation and visible
  combat-bar response pass internal checks; exact Genesis appearance and
  route-specific audio output remain unverified.

The posture and command-bar hills remain partial: other command callers,
overhead combat, AI posture decisions, exhaustive learned-stat/equipment
mutations, an independent selected-posture frame, and route-specific audio
loopback remain open.
