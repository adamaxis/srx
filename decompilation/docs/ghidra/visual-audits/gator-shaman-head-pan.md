# Gator Shaman head and wilderness pan audit

Date: 2026-10-06  
Defect: OPEN-02, Gator Shaman head detaches during the wilderness pan  
Status: resolved for head-to-body attachment during this pan

## Source trace

The startup caller at ROM 0x59C5E enters stage 2 at 0x59D7C. Stage setup
initializes the wilderness scroll at FFDD08=-0x40 and FFDD0A=-0x20, and the
Gator Shaman head-list position at FFDD46=0x138 and FFDD48=0xF1. The stage
renders the entity records through 0x5A37C, which hands the selected list to
the sprite-record builder at 0x1378. Ghidra's export corroborates this call
sequence; the byte values and transitions were checked against the US ROM.

During the first 0xA0-count loop, FFDD08 advances one pixel every source
frame while FFDD46 advances with it, stopping at 0x178. FFDD0A advances every
other source frame when FFDF88 bit 0 permits it, while FFDD48 decreases,
stopping at 0xD1. The resulting invariants are head-x minus x-scroll = 0x178
and head-y plus y-scroll = 0xD1. After the first loop, stage 2 resets the
transient sprite and animation services, runs its second 0x1F0-count story
loop, and returns to the stage orchestrator. There is no separate sound
request in this pan routine. The orchestrated stage music starts and stops are
documented in the OPEN-01 audit; this pan routine adds no audio event.

The Qt state already held the source offsets and head coordinates in typed
OpeningIntroStage2State. The defect was in the presenter: the named persistent
wilderness map layers kept their fixed asset origins while the head list used
the moving coordinates. The renderer now translates the wilderness layers by
(x-scroll, -y-scroll) for the duration of that scene draw, then restores the
painter before later scene content. It does not retain an old layer or
transform across the screen transition.

## Full-screen evidence

The Release whole-run recapture is recorded in
[the post-fix intro review](full-intro-capture-2026-10-06-pan-fix.md). It
contains 6,782 consecutive 320x224 frames from pre-title through the stable
field handoff. All 18 contact sheets (1,131 full-screen samples at 10 fps)
were reviewed end to end. Full-size post-fix frames around the wilderness
story and cockpit transition were inspected separately.

The focused Qt QA writes complete 320x224 screens at pan start, midpoint, and
settled state. Debug and Release both report:

| State | Opening frame | Scroll x/y | Head x/y | Invariant x/y | Body pixels aligned |
|---|---:|---:|---:|---:|---:|
| Start | 688 | -64 / -32 | 312 / 241 | 376 / 209 | 1,924 / 1,924 |
| Midpoint | 720 | -32 / -16 | 344 / 225 | 376 / 209 | 1,924 / 1,924 |
| Settled | 752 | 0 / 0 | 376 / 209 | 376 / 209 | 1,924 / 1,924 |

CTest: shadowrun_ghidra_conversion_qt_opening_wilderness_pan_qa, plus the
startup, rotor, and landing Qt QA tests, passed in Debug and Release. The
full-screen settled dump is
decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/opening-wilderness-pan-qa/actual.bmp;
Debug writes the matching path under its build directory. These pixel checks
prove that the source-derived transform is applied consistently to the named
asset in the Qt frame. They do not act as an independent Genesis golden.

The independent recording supports the same actor and layer relationship in
section 02 frames 000480, 000510, 000570, and 000630. At its 30 fps sampling,
the recording supports the movement and settled composition but cannot
establish exact 60 Hz timing. The multiple consecutive native samples and
source counters establish the host cadence separately.

## Nine-question visual review

1. **Does it look correct? Yes.** The post-fix head stays attached to the
   corresponding body layer through start, midpoint, and settled screenshots;
   the independent section 02 samples support that composition.
2. **Does the look match behavior? Yes.** Typed scroll and head coordinates
   preserve both ROM invariants at all three checkpoints.
3. **Are the visuals correct? Yes for this pan relationship.** The layer
   placement follows the source scroll, with full-screen 320x224 output. No
   claim is made about unrelated intro palette or asset differences.
4. **Does it animate as expected? Yes.** Horizontal scroll and head x advance
   every source frame for 64 updates; vertical scroll and head y update on
   alternating eligible frames for 32 updates. Both stop at the source
   endpoints and remain settled through the rest of the loop.
5. **Is there a sound?** No distinct sound call was found in the ROM pan
   routine. The OPEN-01 stage route is verified in its own audio audit; this
   pan-specific review adds no audio playback evidence.
6. **Are other screen parts correct? Reviewed.** The full intro was swept.
   OPEN-03 and OPEN-06 remain separate, unresolved cockpit goals; this fix does
   not close them.
7. **Do multiple frames support the interpretation? Yes.** Three native
   states, a continuous full-run capture, and four independent section 02
   frames support the automatic pan interpretation. This animation has no
   menu click path.
8. **Does new UI replace old UI cleanly? Yes / not applicable.** This is a
   scene layer, not a child menu. The per-frame painter transform is restored
   immediately, and the full-run capture shows the later cockpit, airport,
   cab, and field scenes without a retained wilderness transform.
9. **Was the whole time window reviewed? Yes.** The capture starts before the
   intro and continues through the destination field hold; the 10 fps contact
   sheets were reviewed end to end and full-size frames checked at transitions.
   The process completed the 120-frame stable field hold without a stall or
   crash.

## Outcome

- **Code changed:** Qt wilderness layer transform in
  ghidra_conversion_qt_main.cpp.
- **Behavior implemented:** source scroll moves the wilderness body layers
  with the Gator Shaman head coordinates.
- **Internally checked:** Debug/Release CTest and full-screen start,
  midpoint, settled BMP dumps with typed-state and pixel-coverage checks.
- **Independently visually checked:** section 02 frames 000480, 000510,
  000570, and 000630, plus the complete native run.
- **Reported defect resolved:** OPEN-02 head-to-body separation during this
  pan is resolved. Broader intro fidelity remains partial.
