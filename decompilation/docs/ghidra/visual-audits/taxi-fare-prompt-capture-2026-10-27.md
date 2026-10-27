# Taxi fare prompt capture audit

**Status:** one fare and accepted-ride sequence is visually matched to the ROM
destination record and Lua branch. The taxi cabbie portrait binding is still
unresolved, and this capture does not expose the wallet or source scene ID.

## Independent capture

The longplay segment is
`video analysis/long_gameplay_video_rest_ffmpeg_30fps_320x224/section_41_source_10620-10815/`.

| Frame | Capture time | Observation |
|---|---|---|
| `frame_002400.png` | `02:58:20.000` | Cabbie portrait and Puyallup Barrens fare quote; visible choices are `Sure thing, Cabbie.` and `Sorry. I changed my mind.` |
| `frame_002425.png` | `02:58:20.833` | Follow-up page says `Hop in, chum.` |
| `frame_002430.png` | `02:58:21.000` | `Hop in, chum.` remains visible. |
| `frame_002460.png` | `02:58:22.000` | Dialogue has closed; the display is in the travel transition. |
| `frame_002600.png` | `02:58:26.667` | Field map is visible again; the exact scene is not identified by the capture. |

The quote and follow-up are one observed path through the fare prompt. The
screen sequence supports a selection of `Sure thing, Cabbie.` followed by the
normal `Hop in, chum.` result. It does not show the wallet before and after,
so the exact 40-nuyen debit is source-derived rather than visually measured.

## ROM-to-display trace

The cab destination table at `0xD445D` maps the selected cursor to region 1,
Puyallup Barrens. Its record starts at `0xD4482`; the inline name is
`Puyallup Barrens` and the fare byte at `0xD449F` is `0x28` (40 decimal). The
current `data.taxi_destinations.lua` row carries that region and amount into
`taxi_cabbie.lua::fare_offer_text`.

The fare wording also joins source data rather than a fixed screen label:
the `0x207B0` fare-comment path selects the 20-nuyen, 50-nuyen, or other-fare
comment. The capture's “out of my way” comment is the ROM text at `0xFE2EA`,
which is the other-fare branch used for 40. The three fare comments are
present at `0x103C22`, `0xFE2D1`, and `0xFE2EA`. The seven destination records
use fares 20, 40, 75, or 100; none uses 50, so the 50-nuyen comment is not
reachable from the current destination table.

The visible `40¥` mark is consistent with the active text path: Lua formats
the numeric amount with a `$` byte, and `DialoguePanelPresenter` draws text
through the extracted `menu.field.font` bitmap. Its renderer maps byte `0x24`
to the corresponding 8x8 source-font cell; it does not replace the character
with a host font or a separately chosen currency symbol.

## Choice and state branches

The quote is reached only after the earlier taxi-service and ride-offer
gates. The source trace at `0x20714`, `0x2076A`, `0x208A8`, and `0x208FE` is
implemented in `taxi_cabbie.lua`:

- If FFFC1E bit 0 is clear, the cabbie gives the traffic-blocked page and no
  fare quote appears.
- If the selected region equals `FFFBF8`, the already-there page appears and
  there is no ride.
- The first ride question can be declined without changing the destination.
- Accepting that question checks the leader's Ork Armband fact. An eligible
  armband ride bypasses the fare quote. Salish-Shidhe also requires its visa;
  a successful Salish ride consumes it.
- In the fare quote shown here, `Sure thing, Cabbie.` deducts the destination
  fare when funds cover it, then says `Hop in, chum.`. When funds are short,
  the ROM instead gives its free-ride page (“for free... this time”) and
  accepts the ride without a debit. `Sorry. I changed my mind.` ends the fare
  offer without changing funds or destination.

The existing `taxi_cabbie_tests.lua` enumerates the fare values, comments,
traffic and already-there gates, declined offers, armband and visa cases,
insufficient funds, paid travel, and visa consumption. The focused Debug/Release
Qt taxi QA now also reproduces the Puyallup fare page, exercises the B-button
decline with unchanged funds and location, and continues through the paid
Downtown trip. The generated fare image is internal regression evidence; the
independent Genesis frame remains the visual authority.

## Portrait match and limits

The ROM path now resolves the portrait. Entry `0x20714` sets `A5=0x121F10`
before calling the shared dialogue routine `0x1F246`; the 12-byte profile
points to palette `0x121BDC`, pattern stream `0x1268DA`, and sprite stream
`0x122338`. The offline profile decoder produces the 56x72 cabbie image,
registered as `dialogue.portraits.taxi.cabbie`. The Lua speaker uses that alias.
The focused Qt taxi QA follows Redmond's ROM cursor order to select Puyallup,
saves the matching 40-nuyen fare prompt, checks its visible portrait pixels at
source placement `(16,32)`, and then declines without charging or traveling. A
separate path still accepts the Downtown fare and completes travel. I inspected
the captured Genesis crop and the extracted art: the face, cap, and crop match
at that placement.

The capture does not show class or race values, wallet balance, service-bit
state, armband state, visa state, or a scene ID. It cannot establish those
inputs from the displayed fare alone. The traced fare amount uses the
destination record; no class or race value enters the taxi fare calculation.
The fare screenshot produced by the Qt test is internal regression evidence;
the independent capture remains the visual authority for the shown branch.
Keep the audit partial until the other fare-gate outcomes are matched to
independent frames or a route/state trace.
