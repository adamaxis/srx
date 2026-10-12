# Field map audio profile

## Status

Partial ROM trace. The current evidence does not identify a per-map music
track, and it does not establish that field gameplay is silent.

## Traced paths

- New Game leaves the title/options flow at ROM `0x007F0`, resets menu audio
  arrays, selects scene 2, loads its scene profile through `0x03084`, and
  initializes the field scene through `0x0281E`.
- `0x03084` selects and copies the scene descriptor/profile block. The
  descriptor fields currently extracted by the map importer contain scene
  setup, camera, spawn, and actor-list data; no music resource identifier has
  been found in that block.
- The field loop calls `0x02900`. That routine checks the pending audio-command
  flag and, when set, passes the pending command value to `0x530EA`. The audio
  service advances queued commands; the traced call does not use the scene ID
  to choose a song.
- Ordinary field transitions pass through `0x02BD2` and the field-mode
  transition state machine at `0x02CE0`. The scene is reloaded from the
  selected scene profile. No scene-ID-to-track lookup or direct per-map music
  request has been identified in the entry and transition branches examined.
- Menu mode has separate audio calls: the menu loop submits event `0x5B` around
  its HUD tick and event `0x03` to the audio command-frame service near
  `0x1763A`–`0x17656`. These menu calls are not evidence for a field-map track.

The addresses above refer to the US ROM; `decompilation/export/disassembly.s`
is a Ghidra cross-check of the control flow, not the source of the behavior
claim.

## Limits and next evidence needed

The transition trace is not a complete audit of every script, event, or audio
driver command that can run after arrival. Audio may be requested by a field
event, or state left active by the prior mode may continue. Therefore the
viewer reports that no per-map music assignment has been found in the traced
entry paths; it does not label a map silent and does not infer a track from
art, a guide label, or a nearby audio filename.

To settle audible carry-over and stop behavior, follow the audio command queue
through its driver-side state changes and compare those transitions with an
independent capture of the original game. Keep menu, intro, event sound, and
field background music as separate evidence until that trace identifies each
one.
