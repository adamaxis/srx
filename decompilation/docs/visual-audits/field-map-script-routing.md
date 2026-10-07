# Field map event route planning

Date: 2026-10-07

**Status:** partial. This slice adds a typed native route policy for the
collision events already connected to the Qt field controller. It does not
cover every map event or establish a first-entry Stoker cutscene.

## ROM-derived route model

The ROM keeps physical collision and its consequence in separate steps. The
field movement path publishes the collision result; `0xB7B8` derives the
event identity from that result and starts the matching interaction. Scene
event tables are separate map data. The source-backed Qt import already keeps
each map's collision rectangles and event table as individually named host
records.

| Source input | Native route choice | Result in the connected path |
|---|---|---|
| Scene 2, collision value `0xC2` at ROM `0x173138`, field mode 2/event 2 | Stoker dialogue script | `0x1F65A` selects the generic Stoker arrival dialogue; A/B advance the source prompt graph and return to the field |
| Scene 2, collision value `0xCB` at ROM `0x173142`, field mode 2/event 11 | Ares service script | `0x1FA78` selects the Ares mode-text prompt; A opens the shop and Start returns |
| Scene 2, paired collision values `0xC0` at ROM `0x173124` and `0x17312E`, event 0 | Active map's event table | Row 0 in `0x172CDC` selects scene `0x14` (scene 20) |
| Scene 20, collision value `0x82` at ROM `0x1D9724`, event 2 | Building interaction continuation | The separate electromagnetic-lock handler `0x20E48` opens its generic prompt |
| Scene 26, collision values `0xD8` at ROM `0x1D9E46`, event `0x18` | Direct typed Security Terminal continuation | `0xB7B8` requires two negative samples and selects `0x20EC2`; the physical-input QA opens both choices and Walk Away returns |
| Any remaining in-range map-table event | Active map's own event table | The selected row is passed to the existing world-activity coordinator; missing/out-of-range tables remain unresolved |

The Stoker prompt is collision-driven after New Game has placed the runner on
scene 2. I found no ROM condition in this traced path that automatically
walks the runner into Stoker's on first map entry. The two adjacent C0 records
are a different doorway and select scene 20; they must not be treated as the
Stoker greeting.

The native route policy is `resolve_field_script_route()`. It names the
Stoker dialogue and Ares service directly, then resolves all other valid
events against the current map's event table. The Qt field-exit coordinator
uses that plan to select the dialogue event code, service route, or normal
area-event continuation. It does not interpret ROM addresses at runtime.

## Full-screen verification

I reviewed all 68 Debug route BMPs as whole 320x224 frames: 12 Stoker frames,
14 Ares frames, and 42 scene-20 frames. The reviewed states include the full
map and HUD during movement, all captured fade phases, each panel/shop state,
and the return screen. The dialogue, Ares shop, and maglock prompt replace the
old field screen; the return captures show the field redrawn without stale
panel controls. The fade checkpoints darken progressively and end at the
source black handoff.

The Ares introduction and category were also compared at full size with the
independent longplay references `ares-introduction-5983.png` and
`ares-shop-5985.png`. The prompt shell, green text, title, and offer layout
agree. The recording uses a different saved runner and wallet, so portrait and
balance differences are not treated as presentation defects. Independent
section 03 frames `000028` through `000082` show the Stoker approach, fade,
blank panel, and populated dialogue. They support screen order and appearance,
not collision input or event bytes. No matching independent scene-20 maglock
screen was found; its visual fidelity remains unverified.

## Eight-question review

1. **Does it look correct?** The full frames show the complete field, HUD,
   fade, dialogue, shop, and return screens. Stoker and Ares screens have
   independent visual references; the exact scene-20 event does not.
2. **Does the look reflect behavior?** The route traces include the collision
   values, projected event codes, selected script/table route, screen owner,
   controller action, and return. No event tuple is injected in these three
   normal-input route captures.
3. **Are the visuals correct?** Stoker and Ares screen families agree with
   their independent references. Scene-20 panel pixels remain source-asset and
   internal-renderer checked only.
4. **Is it animated?** Field movement changes pose and position; the source
   fade has 17 updates. Stoker dialogue is input-driven. The Ares title prompt
   holds for 30 frames before input; its before/after captures are identical.
   Scene-20 prompt/result waits follow their separate 10/30-frame paths.
5. **Is there sound?** No discrete collision or Stoker dialogue sound request
   was established. The Ares prompt path has no direct sound request; shop
   sounds are covered by the shop route. Event `0x30` on successful Security
   Terminal interaction and its stop lifetime remain unresolved.
6. **Are related screen areas correct?** The full images include map, runner,
   HUD, panels, shop state, and return. The Ares status portrait and wallet
   reflect the New Game fixture; they are not compared to a different save as
   if its state were identical.
7. **Do multiple frames support the interpretation?** Yes: 68 native frames
   cover movement, fades, prompts, shop, and returns; independent Stoker and
   Ares samples support the corresponding screen families. The video has no
   collision log.
8. **Was the previous screen cleared?** Yes in the reviewed routes. The new
   full-screen owner replaces the field view, and return frames contain no
   stale dialog or shop controls.

## Verification and remaining work

The focused route-policy CTest and the Stoker, Ares, scene-20, and scene-26
Security Terminal physical-entry CTests pass in Debug and Release. Their BMP
sets have matching names and SHA-256 hashes across configurations, including
the 23 captures for scene 26. The route-policy test checks direct script
priority, scene-2 row 0, a same-numbered event using another map's row, and
unavailable/out-of-range tables. The scene-26 checkpoint and eight-question
review are in [security-terminal-physical-entry.md](security-terminal-physical-entry.md).

This is a route-planning foundation, not complete map/path coverage. The
full descriptor-spawn route to the Security Terminal, additional
doorway/object records, broad terrain recovery, building item use, reverse
transitions, and world-entry one-shot conditions still need their own ROM
traces and normal-input route captures.
