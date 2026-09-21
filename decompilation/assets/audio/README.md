# Genesis sound resources

These files are exact byte slices from the US Genesis ROM. The game does not contain standard MIDI files: the 68000 uploads a custom Z80 sound driver at ROM 0x26C5E and passes four ROM pointers to its initializer at ROM 0x531AA.

`catalog.tsv` records the source offsets and lengths. The two indexed tables are split into 128 and 94 individual event streams, and the descriptor table is split into 22 records. These include music and effect event data, but their event encoding is interpreted by the Z80 driver. MIDI conversion is intentionally not emitted until that format is traced; these raw resources are the authoritative input for that next step.

Extracted 249 files from 5 source blobs (78364 bytes).
