# Lone Star Security, scene 3/event 6

## ROM route

Scene-3 record 96 at `0x18AB96` covers map coordinates `x=0x0700..0x071F`,
`y=0x0040..0x005F`. Its collision value `0x00C6` publishes event 6. Event
table row 6 at `0x18A6FC` is `0x29`, so the generic field route selects numeric
resource-activity scene 41. The ROM route record and table identify the
destination; the site name is the corroborated map crosswalk.

The event is outside the shared maglock event range (2 through 5), and the
scene-3 trigger profile has no indexed title-service branch for event 6. The
route therefore enters scene 41 without a site-specific entry dialogue or
story gate. Lua entity link data owns the event-table route and the shared
collision-probe return policy.

## Physical route QA

Debug and Release CTest `shadowrun_ghidra_conversion_qt_field_penumbra_lone_star_entry_qa`
starts at `(0x7000,0x0700)`, reaches record 96 with ordinary Up, and checks
event 6 loads scene 41 in resource-activity mode. The Release route trace
records 13 movement frames and 17 settle frames. Full 320x224 captures include
`spawn.bmp`, `doorway-contact.bmp`, and `actual.bmp`; I inspected the full
contact and destination frames. This is a controlled physical-input route
check, not an independent Genesis visual comparison.

## Remaining scope

The route's return from scene 41 and the source-selected exit placement have
not yet been exercised on this location. Keep this route partial until its
exit is traced and tested. No independent Genesis capture of this location is
identified.
