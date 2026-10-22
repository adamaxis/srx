# Red Buffalo Woman, Scene 6 event 1

## Source route

Scene 6 profile `0x14B890` selects collision record 194 at `0x14C112` for
event 1. Its collision value is `0x00C1`, with bounds `x=0x0600..0x061F` and
`y=0x03E0..0x03FF`. The event table row at `0x14B8EC` contains `0x0D`, but
indexed title service 18 at `0x54322` sets the caller result to `-1`, so this
interaction remains on Scene 6 instead of entering field map 13.

The Lua service follows the ROM gate: `FFFC1E` bit 17 (Ritual of Binding) must
be set and bit 18 must be clear to show the gift branch rooted at `0xF4DB8`.
Otherwise the source selects the no-gift branch at `0xF4DCC`. ROM prompt nodes
`0xF5888` and `0xF58A4` distinguish the two choices: `no_business` reaches the
leave text at `0xFBD94`, while `more` reaches the penthouse/Harlequin text at
`0xFBEEC`. The `more` edge continues at node `0xF58C0` into the shared
`harlequin.main_conversation` JSON graph, also used by the Tarislar Apartments.
The graph reads prompts from the ROM text catalog and resolves speaker and menu
labels from the active building. Story bit 16 selects the Ito branch: the
no-Ito path sets field-effect bit 7 and shows the final Renraku update without
a continuation; the Ito path applies command `0x40`/story bit 28, shows the Ito
response, then returns to Lua to enter the shared map-information graph. Each
button prompt has an explicit no-selection edge back to the field, matching
the former Lua return path. On gift completion the source marks the active trigger
18 through `0x2196C`, changes Notebook tip bits 6/5 in `FFFC22`, and changes
field effect bits 29/30 in `FFFC30`.

## Physical-input coverage

`shadowrun_ghidra_conversion_qt_field_red_buffalo_physical_entry_qa` starts at
`(0x6100,0x4100)` and reaches record 194 using ordinary Up input in five field
updates. The checkpoint seeds the known New Game story gate and Notebook tip
words while retaining the New Game Joshua record. The test checks the
collision dispatch, Red Buffalo Woman portrait and introduction, the gift
page, story/Notebook/effect writes, same-map Scene 6 return, and completion of
the host modal return fade.

Debug and Release CLI frames are written under
`qt_runtime/field-red-buffalo-physical-entry-qa/`: `spawn.bmp`,
`doorway-contact.bmp`, `actual.bmp`, `gift-response.bmp`, and
`same-map-return.bmp`, with `route.tsv` recording the observed state. The
reviewed frames are full 320x224 images. They verify the current host route and
layout; no independent Genesis capture of this dialogue has been identified.

The separate `shadowrun_ghidra_conversion_qt_field_red_buffalo_no_gift_more_qa`
uses the same ordinary movement route with story bit 17 clear. It chooses
`More` with B, captures the penthouse passage, and follows the menu graph from
`0xF58C0` through the Michael, Thon, and Renraku topics. With the New Game
no-Ito state, the final catalog-backed page sets effect bit 7 and returns
cleanly to Scene 6 without setting the gift-service bit 18. Debug/Release
`GameScriptRuntime` QA also drives the Ito branch, checks command `0x40`, bit 28,
the map-information handoff, and building-specific speaker context. Its
full-frame captures include
`no-gift-prompt.bmp`, `more-response.bmp`, each Harlequin topic page,
`renraku-update.bmp`, and `same-map-return.bmp`; Debug and Release outputs
match by SHA-256. The reusable `harlequin.map_information` graph also has
direct Debug/Release coverage with Red Buffalo's speaker/profile context; its
no-map page is covered here. Physical post-Ito/map-item traversal through this
Scene-6 entry remains open.

## Open work

The no-business leave choice and More continuation are distinct, and the
physical no-gift route now reaches the no-Ito Renraku update. Physical
post-Ito/map-item coverage through this Scene-6 entry remains open, as do other
Scene-6 event records and independent visual confirmation. Keep this route
partial.
