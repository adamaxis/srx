# Owlfeather first contact: Scene 6, event 0

## ROM route

Scene-6 profile `0x14B890` maps collision record 193 (`0x14C108`, bounds
`0x0540..0x055F` by `0x04E0..0x04FF`, raw collision `0x00C0`) to event 0.
The default event-table row at `0x14B8EC` contains `0x0D`, but indexed title
service 2 at `0x53B78` runs first. It requires `FFFC1E` bit 1 set and bit 2
clear, clears `FFFC30` bits 8 and 9, opens dialogue `0xF4B68` with portrait
profile `0x121E14`, and consumes the interaction. The default scene-13 route is
therefore not taken during this first-contact branch.

The Lua route is attached to the `salish_owlfeather_ranger_lodge` entity link.
It starts the bit-2 trigger only when the player chooses “Tell me what you
know about my brother Michael.” The source choice is C. That choice calls ROM
command `0x17`, which adjusts the runner's three source counters, marks bit 2,
and opens the brother-information page. The three initial choices and their
text come from `salish_owlfeather.lua`.

## QA evidence

The physical-entry QA walks with ordinary Up input into record 193 and saves
full `320x224` captures. With bit 1 set and bit 2 clear, service 2 opens the
first-contact graph; C selects the brother topic, applies command `0x17`, and
sets bit 2. Separate physical routes check service 3 (`bit 2=1`, `bit 3=0`),
where Later shows both farewell pages and returns to Scene 6 with bit 3 clear,
and service 4 (`bit 3=1`, `bit 4=0`), where the returned-brother sequence,
Medicine Lodge referral, Thanks, farewell, and trigger tail return to Scene 6.

The closed-gate route is also physical: with bit 1 clear, ordinary Up reaches
the same record but bypasses the indexed service and follows event-table row 0
to scene 13. The host enters that map immediately, with no dialogue or modal
return fade. The CTest verifies the destination map and clean field HUD.

The initial Lands and Council choices also have physical routes. Lands opens
its topic page; Council opens its introduction, then its three-choice topic
page. Choosing Thanks from either path shows both farewell pages, returns to
Scene 6, and leaves bit 2 clear. Together with the C brother choice, all three
choices on the first prompt have an end-to-end physical route.

Debug and Release CTests cover first contact, both follow-ups, the closed gate,
and the two information-only opening branches. Their 39 full-frame captures
and route facts are written under
`qt_runtime/field-owlfeather-{physical-entry,search-followup,return-followup,closed-gate,opening-land-choice,opening-council-choice}-qa/`.
The images were inspected at original `320x224` resolution; every matching
Debug/Release BMP has the same SHA-256. These checks cover the host route, not
independent Genesis visuals. Deeper Ranger/village topic chains, other service
outcomes, and leaving Scene 13 remain open.

No matching independent Genesis dialogue frame has been identified. The
ungated fallback's separate scene-13 building links carry their own ROM return
policy in entity data; a physical departure from that destination remains
unverified.

## Scene-13 conditional selector follow-up

The scene-6 event-0 route can enter scene 13 with FFFC1E bit 19 already set.
At `0x167AE`, scene index 3 reads runner value 6 from `0xEEFE` and compares
marker 7 against both compact runner markers. If neither matches, script index
3 selects `0x1706C`; its sole record points to template `0x172CA` and position
data `0x17082`. The decoded record is actor name `lone11`, kind 10, palette 1,
item action 3, world position `(0x1100, 0x0300)`, heading `0xC000`, with
ordered overrides `+0x2E=0xC0` and `+0x9E=0x10`.

The ROM-generated `field/entities/world-activity-actors.tsv` includes the
selector index and all scenes 10–19 conditional lists. `world_content.lua`
provides the selected list as typed Lua actor descriptors. The new physical
QA sets story bit 19, leaves both runner markers at `0xFF`, enters record 193
with ordinary Up, and verifies scene 13 selects script index 3. It writes
full-frame `320x224` captures under
`qt_runtime/field-owlfeather-ilene-gate-script-qa/`; Debug and Release pass.
This verifies routing and selector state only. The live field actor population
does not yet instantiate this conditional record, and the captured host frame
is not an independent Genesis visual reference. The flag's narrative meaning
and this record's interaction/dialogue remain unresolved.
