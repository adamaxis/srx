# Tarislar City Inn: scene 1, event 1

Status: **partial**. The source-gated Inn service and dialogue tasks are
connected through the shared Lua location-task path. Debug and Release
`FieldSceneRuntime` QA now reaches record 68 with ordinary Up input, exercises
the bit-0/bit-1 title-service gate, and covers Crash, Leave, room payment,
decline, and the paid Hotel status/options sequence. The physical transaction
permutations use 30 nuyen for success, 29 for insufficient funds, and 30 for
decline; they assert the displayed 30-nuyen offer, wallet debit or no-debit,
and same-map outcome. Four more physical routes cover both Caleb/Errascoe topic
orders crossed with Recruit/Thanks. Each reaches the attack page, Renraku
aftermath, Tabatha's room, and David's Holopix before returning to scene 1. The
supplied longplay independently covers the Inn encounter from the intro
through Tabatha's apartment prompt. It does not show the room-payment
alternatives or establish the subsequent combat outcome.

## ROM trace

- Scene 1 record 68 is at `0x17D9AC`. Event-table row 1 at `0x17D621` contains
  destination byte 53. The ordinary scene-table result is retained in Lua
  link data as source evidence.
- `0x53A78` runs indexed title services before generic route selection.
  Service 1 at `0x53B16` requires scene/event 1/1, FFFC1E bit 0 set, bit 1
  clear, and FFF0D5 zero. Its text roots are `0xF49E8` and `0xF49FC`. The
  source returns a stay-here result after the service; it does not enter the
  table's scene 53.
- The shared field prompt selector at `0x1F65A` selects the scene-1/event-1
  roots. Alternate result >0 uses root `0xF49E0`; alternate result <0 uses
  `0xF49D8`. For alternate result 0, bit 13 set and bit 30 clear selects the
  Frosty option root `0xF4990`; otherwise the default root is `0xF4964`.
- Room prompt `0xF49C0` offers the 30-nuyen stay. Command `0x06` at `0x21458`
  charges the active source price. The paid result opens Hotel Options at
  `0xCF38`; insufficient funds return to the field. Both paths remain on the
  exterior map.
- Selecting Tabatha runs command `0x16` at `0x218BC`, marking active story
  trigger 1 in FFFC1E. The conversation starts at `0xF4EA4`; after she agrees
  to receive the runner, command `0x14` at `0x21862` calls `0x59C34` to add one
  to each live runner's source byte at `FF018D`, `FF028D`, and `FF038D`. It
  also sets FFFC30 bits 2/4/5/6/8, FFFC34 bits 2/4, and FFFC27 bits 2/4.
  It loads profile `0x121F7C` for the apartment and later Shale pages. The Lua
  action checks that the FFFC30 and FFFC27 stores are resolved before asking
  the host to increment the three runner bytes; an unavailable save state
  stops at a boundary without a partial acceptance update.
- The ROM page records `0xF4EA4`, `0xF4EB8`, and `0xF4ECC` point to text at
  `0x117EC5`, `0x117F78`, and `0x118060`. The second page explicitly says the
  vid-phone screen remains blank. The independent encounter shows the manager
  portrait on `frame_003390`, a blank portrait well on `frame_003600` and
  `frame_003780`, and Tabatha's portrait on the apartment choice at
  `frame_003900`. The JSON graph now selects the manager, a no-portrait
  narration profile for the two phone/interim pages, then Tabatha's profile.
  This source text and video evidence exposed both the dropped speaker on the
  manager choice panels and the inherited portrait on the blank phone pages.
- The attack page at `0xF617F` continues with command `0x4B` at `0x22112`,
  which loads Renraku art `0x12206C` and returns prompt `0xF4FA0` / text
  `0xF623E`. Command `0x4C` at `0x22124` restores profile `0x121F7C` and
  returns prompt `0xF4FB4` / text `0xF62F1`. Command `0x20` at `0x219A4`
  loads David's profile `0x121E14` and returns prompt `0xF4FC8` / text
  `0xF637B`. That record's negative first-option sentinel uses the shared
  30-frame wait and any-button return at `0x1F3C4`. The three handlers switch
  presentation profiles and prompt roots; they do not contain an actor spawn.
  The indexed service then returns to the current field map. No combat actor
  or post-ambush world-state handoff is established here.
- The Frosty command at `0x2497C` sets FFFC30 bit 23 and FFFC34 bits 4, 13,
  and 21 before displaying the information record at `0x1180D2`.

## Section-41 capture cross-check

I rechecked the independent longplay around the City Inn sequence. At
`frame_003000.png` (`02:58:40`), the manager's intro panel displays the text
stored at `0x117CE3` and `0x117D42`, with the manager portrait from profile
`0x121FB8` and the MORE marker. At `frame_003210.png` (`02:58:47`), the same
manager presents the service prompt and the three visible A/B/C labels for
Crash, Leave, and Tabatha. `frame_003420.png` (`02:58:54`) shows the manager's
Tabatha answer with MORE. This sequence is consistent with the `shale` choice
in `manager_options`, command `0x16` at `0x218BC`, and prompt `0xF4EA4` / text
`0x117EC5`; the video does not record the controller event itself.

One text detail remains **[H]**: at original 320x224 resolution, the line
around “let anybody in … clean up” appears to contain an extra “or” before
“even.” The US ROM bytes at `0x117F13` and `script-text.json` read “even to
clean up,” without “or.” The small font makes that word uncertain in this
frame, so keep the ROM-derived text unchanged until the capture wording is
settled. This observation does not alter the verified choice-to-page route.

Generated text keys for the service, room, package variants, Tabatha dialogue
and aftermath, Frosty, and menu labels live in
`assets/dialogue/script-text.json`. The profile images at `0x121FB8` and
`0x121F7C` are direct 56x72 assets extracted from the US ROM.

## Lua route and host behavior

`gameplay/world/entities/puyallup_site_03_tarislar_city_inn/interactions.json` carries
two entity-owned routes. The priority service row declares a generic
`lua_field_service` task under the source flag/alternate-result gate. The
default area-event row retains scene 53 and attaches the generic field-exit
prompt task. Each row carries a typed location context and the Inn manager profile.

`building.lua` turns those columns into typed `indexed_title_service` and
`field_exit_prompt` tasks. `GameScriptRuntime` validates and returns the task
record; `FieldSceneRuntime` sets the location/field context and opens the Lua
task through `FieldDialogueRuntime`. The shared host accepts the 30-nuyen
building transaction and the runner source-byte-triple adjustment. The Inn's
indexed task now runs `conversations/tarislar_city_inn.json`: the named room
charge action yields paid/insufficient edges, and the paid terminal requests
`hotel.options` with return policy `origin`. `FieldSceneRuntime` dispatches
that named host service and also recognizes the shared `hospital.options` ID.
Story facts for the Inn's three ROM flag groups use the existing typed native
accessors. Tabatha's source prompt pages and all Caleb/Errascoe choice
permutations now live in the JSON graph. Before the runner-byte service can
change Karma, the action checks that each FFFC30 tip and FFFC27 referral bit it
will write is resolved. Page-level speaker profiles follow
the ROM changes from the Inn manager to Tabatha, the Renraku team, Tabatha,
and David. Continuation pages use the source-derived input delay; the final
Holopix page uses the 30-frame any-button presentation contract. The runner
byte-triplet adjustment remains a narrow Lua action.

The generic dialogue menu resolver now receives whether a prompt carries
inline title text. It resolves the stable choice labels while leaving the
static title key absent for inline prompts. This allows inline ROM prompts to
use the shared presenter without conflicting with their menu profile's title
key.

## Validation

- `building_route_tests.lua` checks the indexed service task, default scene-53
  prompt task, and signed alternate-result routes.
- `tarislar_city_inn_tests.lua` checks service prerequisites, Frosty gating,
  both package branches, room payment outcomes, both Tabatha topic permutations,
  the refusal page's MORE continuation, profile changes across the aftermath,
  the 30-frame Holopix page, and the typed same-map return. It also verifies
  command `0x14` stops before any acceptance mutation when saved FFFC30/FFFC27
  state is unresolved or the runner-byte service is unavailable.
- `shadowrun_gameplay_script_runtime_qa` validates the loaded entity route and
  Lua task through the typed C++ route parser.
- `shadowrun_ghidra_field_dialogue_runtime` exercises the source panels,
  payment success/failure, Tabatha's acceptance flags, both Caleb/Errascoe
  physical-input paths, the refusal and attack pages, all three aftermath
  profiles, and the 30-frame Holopix return. It also checks the manager
  portrait on the intro/service/payment/paid/insufficient panels, the blank
  vid-phone panels, and Tabatha's portrait at the apartment choice. It writes
  full 320x224
  `actual.bmp` panels to
  `qt_runtime/tarislar-city-inn-visual-audit/`.
- `shadowrun_ghidra_conversion_qt_field_tarislar_city_inn_qa` starts at
  `(0x5400,0x0D00)`, reaches ROM record 68/event 1 by ordinary movement, and
  verifies title service 1 opens only for story bit 0 set and bit 1 clear.
  It saves the source-gated intro, three-choice service menu, farewell, and
  settled scene-1 `actual.bmp` in both Debug and Release. It checks that Leave
  does not charge and the numeric scene-53 table destination is not used.
- Three additional Debug/Release CTests enter the same record by ordinary Up:
  `...city_inn_paid_qa` sets 30 nuyen, selects Crash then Pay, verifies the
  paid text and zero wallet, follows the graph's real `open_service
  hotel.options` result, acknowledges the Hotel status page, and reaches the
  selected Hotel Options menu on scene 1. `...city_inn_insufficient_qa` sets
  29, verifies the low-credits page, preserves 29, and returns to scene 1.
  `...city_inn_declined_qa` sets 30, selects No, verifies farewell, preserves
  30, and returns to scene 1. All three capture the complete offer, result,
  and final state; the paid route also captures Hotel status and options.
- Four Debug/Release CTests enter by ordinary Up, choose Tabatha, and cover the
  cross product of Caleb-first/Errascoe-first and Recruit/Thanks. They verify
  both source topic responses, the one-option follow-up, recruit refusal versus
  direct Thanks, attack text, Renraku/Tabatha/David profiles, David's portrait
  and text, the 30-frame wait (including an ignored early A edge), a fresh A
  edge after a sampled release, and the scene-1 return with unchanged funds.
  Their 320x224 captures include the manager's three-choice menu and every
  subsequent presentation state through `actual.bmp`.
- The 101 route BMPs across entry, payment, and Tabatha physical paths have
  matching Debug/Release SHA-256 hashes. At original 320x224 resolution, the
  service menu, 30-nuyen offer, paid and insufficient pages, decline farewell,
  Hotel status/options, and settled field returns were inspected. Tabatha's
  Caleb and Errascoe responses, attack page, Renraku page, David's Holopix
  presentation, and final field return were inspected as well. These host
  captures confirm source-driven presentation and route assertions; the
  independent longplay does not show price alternatives or post-attack
  branches.
- `shadowrun_ghidra_field_dialogue_runtime` advances the JSON graph through
  paid and insufficient-funds results and checks the typed `hotel.options`
  outcome. Together, these tests cover the graph transaction and host service
  dispatch in separate, explicit steps.
- All thirteen Debug captures were inspected at native 320x224 resolution,
  including visible text, choice rows, and portrait wells. Their Release
  counterparts have matching SHA-256 hashes. The independent longplay frames
  are in `video analysis/long_gameplay_video_rest_ffmpeg_30fps_320x224/`
  under `section_41_source_10620-10815/`; the targeted frames are `003000`,
  `003210`, `003225`, `003390`, `003420`, `003600`, `003780`, and `003900`.
  The longplay confirms those presentation states; it does not cover every
  payment or post-attack branch.

The City Inn's 30-nuyen offer is a fixed source price; the outcome varies with
the runner's wallet and the service's story gate, not the archetype/race
profile. That is specific to this Inn transaction; starting presets,
class/race-specific abilities and limits, and calculated displays such as
Off/Def bars remain governed by their separate source traces. This hill remains
partial because no independent Genesis capture shows the room-payment or
post-attack branches, and the attack narrative's actor/combat outcome is not
established by the profile and prompt transitions after `0xF617F`.
