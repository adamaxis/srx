# Council Island Inn, Scene 4/event 0

## Source trace

Scene-4 collision record 92 is at ROM `0x15BDCE`. It covers x `0x0440..0x045F`
and y `0x0220..0x023F`, with collision/event value `0x00C0`. The field-mode
service dispatcher reaches the Council Island Inn handler when `FFFBF8 == 4`
and `FFFBF9 == 0` (`0x1F7C2..0x1F7D6`, `0x1F7CE..0x1F7D6`). That handler
(`0x1F7CE..0x1F82E`) prepares the dialogue layout at `0x121FD0`, sets the
requested payment to `0x28` (40), and branches on signed `FFF0D5`:

| `FFF0D5` | ROM text root | Decoded text pointer | Result |
|---:|---:|---:|---|
| Positive | `0xF4A98` | `0x117067` | Package delivery/payment passage; remains on the field map |
| Negative | `0xF4A90` | `0x113991` | Package handoff passage; remains on the field map |
| Zero | `0xF4A60`, then `0xF4A78` | `0x119FCF`, then `0x11A046` | Crash/Leave prompt, then 40-nuyen Pay/Leave prompt |

The room text asks for exactly 40 nuyen. The shared payment helper at
`0x21458..0x21488` subtracts `FFF10D` from wallet `FFFBFE`; balances below 40
take the insufficient branch without changing the wallet, while balances of
40 or more are debited and set `FFF10D` to `-1`. Back at `0x1F81A`, that
negative result enters Hotel Options at `0xCF38`; all branches return `D6=-1`
and suppress the event-table Scene-13 route for this interaction.

The text pointers were decoded directly from the US ROM with
`shadowrun_graphics_extract --dialogue-record`. The introduction, offer, and
both signed result passages match the text carried by
`gameplay/scripts/council_island_inn.lua`. The paid and insufficient responses
reuse the existing shared hotel text keys.

## Converted route and coverage

The Scene-4 entity's `fieldExitPrompt` task dispatches to the Lua location
handler. It validates the building, mode, event, and signed alternate result;
the Lua boundary accepts only representable signed-byte values (`-128..127`)
and reports malformed or out-of-range values explicitly;
the two choices at each zero-result prompt are forwarded through the source
A/B input path. Payment uses the typed `building.charge_40` host service.
Successful payment returns the typed `open_service hotel.options` outcome with
origin policy; refusal, package results, and insufficient funds stay on Scene
4.

Seven physical-input CTests start below record 92 and reach it with ordinary
Up input. They cover this branch matrix:

| Route | Input/state | Wallet before -> after | Checked result |
|---|---|---:|---|
| `package-positive` | `FFF0D5 = +1` | 100 -> 100 | Positive package passage; no service or charge |
| `package-negative` | `FFF0D5 = -1` | 100 -> 100 | Negative package passage; no service or charge |
| `decline-inn` | Zero result; B at Crash/Leave | 100 -> 100 | Closes on Scene 4 |
| `decline-room` | Crash, then B at Pay/Leave | 100 -> 100 | Closes on Scene 4 |
| `insufficient` | Crash, Pay; balance below 40 | 39 -> 39 | Low-creds response; no Hotel Options |
| `paid-exact` | Crash, Pay; exact price | 40 -> 0 | Paid response, Hotel Status, then Hotel Options on Scene 4 |
| `paid-excess` | Crash, Pay; balance above price | 100 -> 60 | Same-map Hotel Status and Hotel Options |

Each test writes a full 320x224 `actual.bmp` and named route checkpoints under
`qt_runtime/field-council-island-inn-qa/<route>/`. The CTests assert the ROM
collision/event, decoded choice and result text, wallet changes, same-map
behavior, and green-pixel presence in the dialogue, Hotel Status, and Hotel
Options regions. Debug and Release each produce 37 BMPs; every corresponding
capture has the same SHA-256 hash. The full Debug frames for the default
prompt, payment prompt, paid/insufficient responses, both package results,
Hotel Status, and Hotel Options were inspected at original resolution.

## Evidence limits

No independent Genesis capture of this doorway's dialogue or room transaction
was identified. The trace, input branches, wallet behavior, UI text, and
internal rendering regression checks are covered; pixel-for-pixel Genesis
fidelity remains unverified. This audit does not connect the inn to a separate
Scene-13 interior.
