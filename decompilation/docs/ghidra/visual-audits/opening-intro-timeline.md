# Opening intro timeline (archetype confirm -> first field picture)

Ground truth: Genesis Plus GX (libretro) driving the US ROM, per-frame luma,
thumbnails, 64 KiB work RAM (FFDF88 = wait_for_vblank 0x0BD2 call counter) and
audio. Frames with no FFDF88 increment are CPU-bound (lag) frames. Times below
are frames relative to the song 0x10 onset, which both sides start from a
command (most robust anchor). [C] = emulator/disassembly, [H] = inferred.

## ROM flow [C]

0x59C5E: music 0x10 -> 0x5AF62 (0x5B29A, 0x1406 = 0x131C 17x2 VBlanks + load,
0xC0C2, schedule loop) -> 0x53126 (6 x 3 VBlanks audio fade) -> music 0x11 ->
0x59D7C (0xC0C2, 0x1406, wilderness loops) -> 0x53126 -> music 0x12 ->
0x5AA96 (NO 0x131C: plane clears 0x0FFE/0x102A, tri-vid decode, 0x12F6 from
the wilderness palette = visual cut-in, FFDF88 saved/restored around it;
0x5AB96 darkens lines 0-3 one CRAM level per step after loop frame 0xB4) ->
0x5AD1A (0x5B29A, 0x1406, airport loop) -> 0x59CEA 0x131C (visible) ->
0x59D12 (0x1406 over black, plane clears, 0x12F6, jmp 0x134D2 cab map) ->
0x59CFE (sample, 0x131C visible, 0x53126) -> field 0x07F0.

## Port model

The conversion runs the logic frames; blocking ROM calls that run no intro
logic (0x53126, 0x131C/0x12F6 over black, decode lag) are host presentation
holds keyed by `source_boundary` names, values in Lua
`Startup.opening_intro_timeline()` (`gameplay/scripts/startup.lua`):

| boundary | hold | ROM |
|---|---|---|
| 0x5AF62.load_title_font | black 56 (34+22) | 0x131C on faded menu + decode |
| 0x5AF62.schedule_complete | freeze 9 (+1 deferral frame) | schedule still tail [H] |
| 0x59D7C.begin_stage2 | freeze 18 | 0x53126 |
| 0x59D9C.install_wilderness_map_layers | black 30 | 0x1406 decode lag |
| 0x5AA96.begin_trivid_orchestrator | freeze 18 | 0x53126 |
| 0x5AA96.decode_trivid_source_streams | black 22 | decode lag |
| 0x5AD1A.prepare_airport_resources | black 23 | 0x1406 decode lag |
| 0x59D12.enter_post_intro_mode | black 78 (34+1+34+9) | 0x1406 + 0x12F6 + 0x134D2 setup |
| 0x59CFE.prepare_mode_entry | black 109 (18+91) | 0x53126 + 0x07F0 setup |
| field.opening_auto_walk.armed | fade_in 30 | 0x12F6 field fade-in before the walk |
| field.opening_auto_walk.consumed | black 38 (18+20) | 0x53126 + dialogue load |
| boot.title_setup | black 103 (34+35+34) | 0x1D7A: 0x1406 (0x131C + decode) + 0x12F6 |

`Startup.opening_audio_waits()` (same file) holds later audio commands:
`field.opening_auto_walk.scene_music` = 8 frames after the preceding 0x53126
(0x07F0 setup before 0x2900 play_music).

The Start exit (0x59CFE) is logic, not a hold: 0x131C fades the interrupted
scene for 34 VBlanks with the music still playing, then 0x53126.

## Result (emulator vs port, frames after the 0x10 onset)

| event | emulator | port |
|---|---|---|
| Seattle plate (black ends) | 56 | 56 |
| song 0x11 onset | 704 | 704 |
| wilderness black | 716-770 | 716-770 |
| song 0x12 onset / tri-vid black | 1478 / 1478-1500 | 1478 / 1479-1501 |
| tri-vid -> airport black | 1758-1869 | 1758-1869 |
| airport -> cab map black | 3202-3301 | 3202-3302 |
| cab map -> field black | 3494-3627 | 3494-3627 |
| field song 0x0F onset | 3562 | 3562 |
| field fade-in luma +3628/+3630/+3632 | 20.8/30.0/32.3 | 22.7/32.5/34.8 |
| Jake's walk starts | 3659 | ~3659 |
| motel-sign fade / audio fade end | ~3680 / 3715 | ~3680 / 3714 |
| dialogue panel / text | 3733 / 3737 | 3735 / 3735 |
| Start at +300: song 0x0F / field visible | 378 / 444 | 378 / 444 |

Boot (no anchor): FASA first frame -> title logo reveal 378 frames, both.

Internal scene events (frame-diff matched) have median offset 0..-1, max 4,
except the wilderness projectile sweep (emu events 1284-1326 unmatched [H]).
The grok-fixes-3 "18-frame music-vs-visual offset" was the missing 0x53126
blocking fade; with the holds 0x11/0x12 onsets match exactly.

Open: the archetype exit fade shows its first dark frame ~5 frames later in
the port relative to the press/music (generic fade, not flagged); the
wilderness projectile sweep shows fewer tracers in the port [H]; the dialogue
panel frame/text reveal differ by about 2 frames.
