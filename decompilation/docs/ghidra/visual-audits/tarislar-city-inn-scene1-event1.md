# Tarislar City Inn: scene 1, event 1

Status: **partial**. The source-gated Inn service and dialogue tasks are
connected through the shared Lua location-task path. Debug and Release
`FieldSceneRuntime` QA now reaches record 68 with ordinary Up input, exercises
the bit-0/bit-1 title-service gate, advances the intro, selects Leave with B,
and returns to scene 1 without a charge or scene-53 load. The room/Leave/
Tabatha choices and paid/insufficient transaction now run through the JSON
graph. A successful graph transaction yields typed `open_service` for
`hotel.options`; the graph payment test and a controlled QA call through the
production service dispatcher verify those two parts separately. This does
not claim an end-to-end paid physical field transaction. The supplied longplay
has no identified matching Inn capture, and the Tabatha ambush still stops at
its unconverted encounter commands.

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
- The later attack page at `0xF617F` continues through encounter commands
  `0x4B`, `0x4C`, and `0x20`. That combat handoff is not converted. Lua returns
  `tabatha_ambush_unconverted`; the dialogue host displays the source boundary
  with the three commands named.
- The Frosty command at `0x2497C` sets FFFC30 bit 23 and FFFC34 bits 4, 13,
  and 21 before displaying the information record at `0x1180D2`.

Generated text keys for the service, room, package variants, Tabatha dialogue,
Frosty, and menu labels live in `assets/dialogue/script-text.json`. The asset
was regenerated from the US ROM with the existing script copied into a
temporary output tree so separately managed text entries remained intact.

## Lua route and host behavior

`gameplay/world/entities/puyallup_site_03_tarislar_city_inn/interactions.json` carries
two entity-owned routes. The priority service row declares a generic
`lua_field_service` task under the source flag/alternate-result gate. The
default area-event row retains scene 53 and attaches the generic field-exit
prompt task. Each row carries a typed location context and the Inn narrator.

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
permutations now live in the JSON graph. The runner byte-triplet adjustment
remains a narrow Lua action; the attack page's following encounter commands
remain an explicit source boundary.

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
  the refusal page's MORE continuation into the attack page, acceptance state,
  and the explicit encounter boundary result.
- `shadowrun_gameplay_script_runtime_qa` validates the loaded entity route and
  Lua task through the typed C++ route parser.
- `shadowrun_ghidra_field_dialogue_runtime` exercises the source panels,
  payment success/failure, Tabatha's acceptance flags, both Caleb/Errascoe
  physical-input paths, the refusal and attack pages, and the visible encounter
  boundary. It writes full 320x224
  `actual.bmp` panels to
  `qt_runtime/tarislar-city-inn-visual-audit/`.
- `shadowrun_ghidra_conversion_qt_field_tarislar_city_inn_qa` starts at
  `(0x5400,0x0D00)`, reaches ROM record 68/event 1 by ordinary movement, and
  verifies title service 1 opens only for story bit 0 set and bit 1 clear.
  It saves the complete intro, three-choice service menu, farewell, and
  restored scene-1 frame in both Debug and Release
  `qt_runtime/field-tarislar-city-inn-qa/` folders. The QA checks the source
  30-nuyen service is not charged on Leave and that the numeric scene-53 table
  destination is not used for that same-map service result. After the physical
  route, it uses a controlled typed `open_service hotel.options` request to
  exercise the same production dispatcher and saves
  `controlled-hotel-options-dispatch.bmp`; it does not simulate a second Inn
  visit or represent this capture as the paid graph path.
- `shadowrun_ghidra_field_dialogue_runtime` advances the JSON graph through
  paid and insufficient-funds results and checks the typed `hotel.options`
  outcome. Together, these tests cover the graph transaction and host service
  dispatch in separate, explicit steps.
- The six Debug panels were inspected at native resolution. They show the
  complete frame and bottom choice area. This is renderer regression evidence,
  not an independent Genesis comparison.

The branch remains partial until an independent game capture is available and
the commands after `0xF617F` have native behavior.
