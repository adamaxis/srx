# Opening intro to field audio ownership

Date: 2026-11-06
Scope: playtest backlog #52, from the Start-to-opening route through the first
field-music request.

## Source trace

The US ROM separates music requests from sound effects. Opening music IDs
`0x10`, `0x11`, and `0x12` are sent through `0x530EA` at `0x59C72`, `0x59C92`,
and `0x59CB8`. That entry checks the music-enable flag `FFFC08`, stops the same
resource, then starts it. Intro effects use the SFX entry `0x530DE`, gated by
`FFFC0A`:

| Event | ROM call | Trigger |
|---|---|---|
| `0x2E` | `0x59FAA` -> `0x530DE` | `0x59F8E` requests it when elapsed overlay frame equals `0x0168` |
| `0x46` | `0x5A134` -> `0x530DE` | sequence counter `FFF126` reaches 4 |
| `0x2F` | `0x5A2BE` -> `0x530DE` | persistent-sequence counter `FFF12E` reaches 4 |

The conversion had sent the music and effect requests through one
`service_audio_command` callback wired to `play_music`. That made the three
effects follow the music enable setting. The opening callbacks now express
`play_music` and `play_sound_effect` separately; Qt sends effects through the
`0x530DE` host path. Integration QA checks the three IDs arrive once as effects, and checks
`0x2E` at the source-relative `0x0168` trigger. Audio CTest checks that the
effect still starts with music disabled and that field music still starts
with effects disabled.

The field track request is also profile-driven. Scene 2 selects profile
`0x172C80`; its word at `+0x40`, ROM address `0x172CC0`, is `0x000F` in the US
ROM. The field service `0x2830` sets the music latch and `0x2900` reads the
profile word and requests that ID. Lua returns the profile's music fact, so
the host does not assign a track from a scene-name rule. The provided
GPGX/port timeline records field music `0x0F` at frame 3562 from opening music
`0x10`, with the first field picture around frame 3628. See
[`opening-intro-timeline.md`](opening-intro-timeline.md).
For its early-Start branch, the same timeline records the field music request
at frame 378 and the first field picture at frame 444 in both GPGX and the
port.

## What the reported sound gap means

At the intro's final Start exit, `0x59CFE` runs the visible 34-VBlank palette
transition with the current track active, then calls `0x53126`, which ramps
attenuation, sends `FF 16`, and restores level zero. The first field music
request follows the field setup. The opening timeline measures both GPGX and
port starting profile music `0x0F` at frame 3562. The stage-3 audio stream has
no within-stage restart in the 90-second ROM-driver trace; its final YM write
is at 19.004830 seconds after the `0x12` request. The current 60-second
observation FLAC retains silence after that source activity and ends at the
later `FF 16` boundary. Therefore the quiet interval before field music is
present in the source path. No music was inserted into that gap.

The source identifies the later field request as profile music `0x0F`; the
recorded name "Red Baron" is not encoded in the profile word or the traced
request, so that label remains [H]. The three intro effect IDs are confirmed
as SFX requests and their timings are source-backed [C]. Their semantic
identity as helicopter sounds is [H]; the supplied longplay does not isolate
those three effects well enough to establish that label or an independent
audio onset for each one.

## Outcome

- **Code changed:** opening music and sound-effect callbacks now use distinct
  typed host paths.
- **Behavior implemented:** intro effects follow the SFX enable state; stage
  and field music follow the music enable state. The ROM-derived event IDs and
  timing were preserved.
- **Internally checked:** startup integration verifies music IDs `0x10`/`0x11`/
  `0x12`, effect IDs `0x2E`/`0x46`/`0x2F`, and the source-relative `0x2E`
  trigger. Debug/Release audio playback QA verifies the independent gates.
- **Independently checked:** the existing GPGX timeline aligns music `0x10`,
  stage music requests, and the field music `0x0F` onset with the port. The
  longplay does not isolate the three intro effects.
- **Still open:** correlate each effect against a captured Genesis audio
  stream and verify the user-described "helicopter" identity. Windows physical
  output for these exact cues has not been listened to.
