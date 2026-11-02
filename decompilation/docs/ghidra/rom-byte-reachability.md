# US ROM byte-level code reachability

This is a conservative recursive-descent disassembly of ROM bytes, not a complete control-flow proof. The walk starts from valid 68000 vector targets, follows direct branches and calls, and follows indirect calls resolved by Ghidra, pointer tables, or confirmed callback records. Candidate targets from an inline indexed jump are recorded but are not followed while the input domain is unbounded. Other unresolved transfers are reported and their targets are not guessed. Data reached through an indirect edge can still decode as an instruction; review the unresolved list before using this as a completeness claim.

ROM size: **2,097,152 bytes**. SHA-256: `92a614491fc3ea12a6df32459cce08b48574b7b814e244c07eb860dc5c6edb7e`.
Vector table: `0x000000..0x0000FF`; `63` valid ROM vector slots resolve to `9` unique instruction roots: `0x000200`, `0x000208`, `0x000210`, `0x000218`, `0x000220`, `0x000228`, `0x00022E`, `0x000234`, `0x000236`.

## Reachability summary

| Measure | Result |
|---|---:|
| Decoded reachable instruction starts | 31,644 |
| Reachable instruction bytes (union) | 139,554 |
| Reachable bytes inside union of known function spans | 105,992 |
| Known function bytes reached | **109,300 / 111,098** |
| Known functions with no reached bytes | 26 |
| Partially reached known functions | 16 |
| Reachable bytes outside known function spans | **33,562** |
| Reachable ranges outside known function spans | **175** |
| Exported functions used for sized spans | 911 |
| Status-ledger-only entry addresses | 169 |
| Resolved indirect call targets followed | 365 |
| Pointer/dispatch sources discovered | 24 |
| Indirect JMP sites found in Ghidra listing | 2 |
| Unresolved indirect jump sites | 1 |
| Reachable unresolved indirect jumps | 0 |
| Unresolved indirect calls | 2 |
| Invalid/out-of-ROM control-flow targets | 0 |
| Vector/branch targets that Capstone could not decode | 1 |

Known-function byte totals use the sum of exact `entry` and `size` values in `decompilation/export/functions.tsv`; reached bytes are counted within each function extent. The current inventory totals `111,098` bytes. The status ledger contains entry addresses but no sizes; for ledger-only entries, only the decoded entry instruction is masked as known. No body length is inferred for those rows.

## Known functions with no reached bytes

These exported functions have no decoded reachable byte within their sized span. Partially reached functions are counted in the summary but are not repeated here.

| ROM entry | Size (bytes) | Name |
|---:|---:|---|
| `0x000BB6` | 28 | wait_for_even_frame |
| `0x001184` | 60 | write_masked_vdp_row |
| `0x00167C` | 164 | decode_source_coordinate_stream |
| `0x00E00A` | 42 | wait_for_controller_event |
| `0x00F404` | 18 | write_menu_footer_grid |
| `0x00F5F4` | 232 | render_bitmask_text_batch |
| `0x014F50` | 54 | advance_entity_collision_recovery |
| `0x015354` | 60 | activate_entity_audio_state |
| `0x015554` | 218 | service_nearby_entity_auxiliary_behavior |
| `0x01562E` | 24 | service_entity_auxiliary_behavior |
| `0x0189B4` | 36 | write_hud_repeated_grid_variant_c |
| `0x01D1A4` | 16 | advance_effect_vertical_position |
| `0x01D1B4` | 16 | retreat_effect_vertical_position |
| `0x01D1C4` | 12 | advance_effect_horizontal_position |
| `0x01D1D0` | 12 | retreat_effect_horizontal_position |
| `0x026074` | 58 | clear_grid_segments |
| `0x026610` | 2 | write_sequential_asset_grid |
| `0x052FD2` | 32 | flush_append_pair |
| `0x053094` | 12 | begin_bus_then_flush_append_pair |
| `0x0530A0` | 38 | controller_access_pair |
| `0x053172` | 28 | run_fixed_audio_pair |
| `0x053194` | 22 | run_controller_access |
| `0x053A24` | 84 | begin_title_audio_sequence |
| `0x055900` | 72 | resolve_entity_action_status |
| `0x055948` | 116 | resolve_equipment_action_status |
| `0x05D1B4` | 46 | prepare_pre_title_audio_variant_b |

## Reachable ranges outside known functions

Ranges are half-open byte intervals in the calculations; the table displays inclusive end addresses. All ranges are listed, ordered by descending size.

| Rank | Start | End (inclusive) | Bytes |
|---:|---:|---:|---:|
| 1 | `0x023764` | `0x02401B` | 2,232 |
| 2 | `0x022F96` | `0x02374F` | 1,978 |
| 3 | `0x01E744` | `0x01EEDB` | 1,944 |
| 4 | `0x0242FA` | `0x024963` | 1,642 |
| 5 | `0x02213C` | `0x0225D1` | 1,174 |
| 6 | `0x00CA16` | `0x00CDF5` | 992 |
| 7 | `0x024A5C` | `0x024E1B` | 960 |
| 8 | `0x022938` | `0x022C97` | 864 |
| 9 | `0x0121E4` | `0x01253D` | 858 |
| 10 | `0x058394` | `0x0586C9` | 822 |
| 11 | `0x0225D4` | `0x022873` | 672 |
| 12 | `0x022CF0` | `0x022F8F` | 672 |
| 13 | `0x00EFB4` | `0x00F235` | 642 |
| 14 | `0x024E44` | `0x025055` | 530 |
| 15 | `0x00C44A` | `0x00C64B` | 514 |
| 16 | `0x0599DA` | `0x059BCB` | 498 |
| 17 | `0x054486` | `0x054661` | 476 |
| 18 | `0x01283A` | `0x0129FD` | 452 |
| 19 | `0x01B8BC` | `0x01BA6D` | 434 |
| 20 | `0x021A66` | `0x021BE3` | 382 |
| 21 | `0x02145E` | `0x0215D9` | 380 |
| 22 | `0x021CEA` | `0x021E65` | 380 |
| 23 | `0x00B57A` | `0x00B6E3` | 362 |
| 24 | `0x004CDE` | `0x004E37` | 346 |
| 25 | `0x0240FA` | `0x024251` | 344 |
| 26 | `0x021698` | `0x0217E3` | 332 |
| 27 | `0x00CDFC` | `0x00CF37` | 316 |
| 28 | `0x020772` | `0x0208A7` | 310 |
| 29 | `0x021EA4` | `0x021FCF` | 300 |
| 30 | `0x012638` | `0x01275B` | 292 |
| 31 | `0x01EF34` | `0x01F045` | 274 |
| 32 | `0x05B378` | `0x05B481` | 266 |
| 33 | `0x02611C` | `0x02621D` | 258 |
| 34 | `0x055B78` | `0x055C75` | 254 |
| 35 | `0x02122E` | `0x021317` | 234 |
| 36 | `0x025D0E` | `0x025DDF` | 210 |
| 37 | `0x00C2AA` | `0x00C375` | 204 |
| 38 | `0x005154` | `0x00521D` | 202 |
| 39 | `0x05924E` | `0x059317` | 202 |
| 40 | `0x0596C4` | `0x059785` | 194 |
| 41 | `0x00F344` | `0x00F403` | 192 |
| 42 | `0x053BD0` | `0x053C8F` | 192 |
| 43 | `0x00DA6E` | `0x00DB25` | 184 |
| 44 | `0x053B1A` | `0x053BCB` | 178 |
| 45 | `0x01C146` | `0x01C1F5` | 176 |
| 46 | `0x0218C2` | `0x02196B` | 170 |
| 47 | `0x00F23A` | `0x00F2E1` | 168 |
| 48 | `0x004FE4` | `0x005089` | 166 |
| 49 | `0x0219AA` | `0x021A4D` | 164 |
| 50 | `0x005324` | `0x0053C3` | 160 |
| 51 | `0x00508C` | `0x005129` | 158 |
| 52 | `0x005220` | `0x0052BD` | 158 |
| 53 | `0x022898` | `0x022933` | 156 |
| 54 | `0x01B73A` | `0x01B7CF` | 150 |
| 55 | `0x021336` | `0x0213C9` | 148 |
| 56 | `0x01641A` | `0x0164A7` | 142 |
| 57 | `0x05AE9E` | `0x05AF29` | 140 |
| 58 | `0x0578C4` | `0x05794D` | 138 |
| 59 | `0x013EF4` | `0x013F75` | 130 |
| 60 | `0x01F08C` | `0x01F105` | 122 |
| 61 | `0x0056DE` | `0x005755` | 120 |
| 62 | `0x05432A` | `0x0543A1` | 120 |
| 63 | `0x01216A` | `0x0121DF` | 118 |
| 64 | `0x01BC72` | `0x01BCE7` | 118 |
| 65 | `0x0006C6` | `0x000739` | 116 |
| 66 | `0x018186` | `0x0181F7` | 114 |
| 67 | `0x01BAF8` | `0x01BB69` | 114 |
| 68 | `0x022078` | `0x0220E9` | 114 |
| 69 | `0x053CEA` | `0x053D59` | 112 |
| 70 | `0x021C76` | `0x021CE3` | 110 |
| 71 | `0x0560A8` | `0x056115` | 110 |
| 72 | `0x01B69C` | `0x01B705` | 106 |
| 73 | `0x024262` | `0x0242CB` | 106 |
| 74 | `0x05D1F6` | `0x05D25F` | 106 |
| 75 | `0x053F94` | `0x053FFB` | 104 |
| 76 | `0x01A88E` | `0x01A8F3` | 102 |
| 77 | `0x02162E` | `0x021691` | 100 |
| 78 | `0x0052C2` | `0x005321` | 96 |
| 79 | `0x054226` | `0x054285` | 96 |
| 80 | `0x0142B4` | `0x014311` | 94 |
| 81 | `0x00C8CC` | `0x00C927` | 92 |
| 82 | `0x053ABA` | `0x053B15` | 92 |
| 83 | `0x05407A` | `0x0540D5` | 92 |
| 84 | `0x054156` | `0x0541B1` | 92 |
| 85 | `0x0543A6` | `0x054401` | 92 |
| 86 | `0x01B820` | `0x01B879` | 90 |
| 87 | `0x005AC6` | `0x005B1D` | 88 |
| 88 | `0x021866` | `0x0218BB` | 86 |
| 89 | `0x0249C8` | `0x024A1D` | 86 |
| 90 | `0x0208AA` | `0x0208FD` | 84 |
| 91 | `0x021404` | `0x021457` | 84 |
| 92 | `0x02201E` | `0x022071` | 84 |
| 93 | `0x01C22A` | `0x01C27B` | 82 |
| 94 | `0x055FFE` | `0x05604F` | 82 |
| 95 | `0x0143F6` | `0x014445` | 80 |
| 96 | `0x00E47A` | `0x00E4C7` | 78 |
| 97 | `0x019FD0` | `0x01A01D` | 78 |
| 98 | `0x022CA0` | `0x022CE7` | 72 |
| 99 | `0x019C6C` | `0x019CB1` | 70 |
| 100 | `0x01CD14` | `0x01CD59` | 70 |
| 101 | `0x01A0C4` | `0x01A105` | 66 |
| 102 | `0x021FD6` | `0x022017` | 66 |
| 103 | `0x002A9A` | `0x002AD9` | 64 |
| 104 | `0x019D26` | `0x019D65` | 64 |
| 105 | `0x05D136` | `0x05D171` | 60 |
| 106 | `0x01F04C` | `0x01F085` | 58 |
| 107 | `0x01F10C` | `0x01F145` | 58 |
| 108 | `0x0215DC` | `0x021615` | 58 |
| 109 | `0x01BE82` | `0x01BEB9` | 56 |
| 110 | `0x0211EE` | `0x021225` | 56 |
| 111 | `0x0011FC` | `0x001231` | 54 |
| 112 | `0x013A5C` | `0x013A91` | 54 |
| 113 | `0x021C3C` | `0x021C71` | 54 |
| 114 | `0x020904` | `0x020937` | 52 |
| 115 | `0x024A22` | `0x024A55` | 52 |
| 116 | `0x056658` | `0x05668B` | 52 |
| 117 | `0x05D182` | `0x05D1B3` | 50 |
| 118 | `0x01B7EC` | `0x01B819` | 46 |
| 119 | `0x016D60` | `0x016D89` | 42 |
| 120 | `0x00B0AC` | `0x00B0D3` | 40 |
| 121 | `0x00B0E4` | `0x00B10B` | 40 |
| 122 | `0x013E30` | `0x013E57` | 40 |
| 123 | `0x018158` | `0x01817D` | 38 |
| 124 | `0x021C12` | `0x021C37` | 38 |
| 125 | `0x019E84` | `0x019EA5` | 34 |
| 126 | `0x021982` | `0x0219A3` | 34 |
| 127 | `0x021BEA` | `0x021C0B` | 34 |
| 128 | `0x0220F0` | `0x022111` | 34 |
| 129 | `0x024982` | `0x0249A3` | 34 |
| 130 | `0x021E6C` | `0x021E8B` | 32 |
| 131 | `0x01E09A` | `0x01E0B7` | 30 |
| 132 | `0x021846` | `0x021861` | 28 |
| 133 | `0x0014E8` | `0x001501` | 26 |
| 134 | `0x002AFA` | `0x002B13` | 26 |
| 135 | `0x0249AA` | `0x0249C3` | 26 |
| 136 | `0x019E1A` | `0x019E31` | 24 |
| 137 | `0x019F00` | `0x019F17` | 24 |
| 138 | `0x01B5FC` | `0x01B611` | 22 |
| 139 | `0x01B63E` | `0x01B653` | 22 |
| 140 | `0x059C48` | `0x059C5D` | 22 |
| 141 | `0x00285C` | `0x00286D` | 18 |
| 142 | `0x0213D0` | `0x0213E1` | 18 |
| 143 | `0x0213E8` | `0x0213F9` | 18 |
| 144 | `0x024022` | `0x024033` | 18 |
| 145 | `0x02403A` | `0x02404B` | 18 |
| 146 | `0x024052` | `0x024063` | 18 |
| 147 | `0x02406A` | `0x02407B` | 18 |
| 148 | `0x024082` | `0x024093` | 18 |
| 149 | `0x02409A` | `0x0240AB` | 18 |
| 150 | `0x0240B2` | `0x0240C3` | 18 |
| 151 | `0x0240CA` | `0x0240DB` | 18 |
| 152 | `0x0240E2` | `0x0240F3` | 18 |
| 153 | `0x02496A` | `0x02497B` | 18 |
| 154 | `0x053236` | `0x053247` | 18 |
| 155 | `0x01F1EE` | `0x01F1FB` | 14 |
| 156 | `0x01F202` | `0x01F20F` | 14 |
| 157 | `0x05D408` | `0x05D415` | 14 |
| 158 | `0x021E92` | `0x021E9D` | 12 |
| 159 | `0x022118` | `0x022123` | 12 |
| 160 | `0x02212A` | `0x022135` | 12 |
| 161 | `0x021328` | `0x021331` | 10 |
| 162 | `0x02161E` | `0x021627` | 10 |
| 163 | `0x059C2A` | `0x059C33` | 10 |
| 164 | `0x00186A` | `0x001871` | 8 |
| 165 | `0x00AE58` | `0x00AE5D` | 6 |
| 166 | `0x014220` | `0x014225` | 6 |
| 167 | `0x019F6C` | `0x019F71` | 6 |
| 168 | `0x054BCC` | `0x054BD1` | 6 |
| 169 | `0x0042EE` | `0x0042F1` | 4 |
| 170 | `0x01B5DE` | `0x01B5E1` | 4 |
| 171 | `0x021322` | `0x021325` | 4 |
| 172 | `0x0019D0` | `0x0019D1` | 2 |
| 173 | `0x01669A` | `0x01669B` | 2 |
| 174 | `0x0268E4` | `0x0268E5` | 2 |
| 175 | `0x166514` | `0x166515` | 2 |

## Discovered pointer dispatch tables

The analyzer recognizes direct and indexed `lea table,An` pointer loads, tables passed to shared dispatchers, active-record callbacks, and title delay/callback records. Pointer entries are read from raw ROM bytes. An explicit guard supplies a count when its compare/branch shape is recognized; otherwise entries stop at the next discovered table base or first invalid/non-code pointer. Caller-supplied tables use their ROM data boundary. The inline PC-indexed jump at `0x2510A` has a candidate block list, but its source record upper bound is not established, so those targets are not traversed. Tables are candidate target sets, not proof that every branch is dynamically reachable.

| Transfer site | Kind | Table / record base | Candidate entries | Candidate code targets | ROM evidence | Target domain |
|---:|---|---:|---:|---|---|---|
| `0x0046F6` | jsr longword-pointer table (stride 4) | [C] ROM `0x00490A` | 15 | `0x0049A0`, `0x00521E`, `0x005152`, `0x004CDE`, `0x0049A0`, `0x0052BE`, `0x00557E`, `0x0049A0`, `0x00508A`, `0x004FE2`, `0x004D54`, `0x0049A0`, `0x0056DE`, `0x0053C4`, `0x0049A0` | [C] `0x00490A`, [C] `0x0046EC` | bounded by source structure |
| `0x00C444` | jsr longword-pointer table (stride 4) | [C] ROM `0x0C1400` | 6 | `0x00F236`, `0x00C8C8`, `0x018150`, `0x00C448`, `0x00EFB4`, `0x0599DA` | [C] `0x0C1400`, [C] `0x00C33A` | bounded by source structure |
| `0x00C444` | jsr longword-pointer table (stride 4) | [C] ROM `0x0C14C8` | 2 | `0x00D024`, `0x00DA6A` | [C] `0x0C14C8`, [C] `0x00CFD2` | bounded by source structure |
| `0x00C444` | jsr longword-pointer table (stride 4) | [C] ROM `0x0C1558` | 3 | `0x00CA2E`, `0x00CA16`, `0x00CE7C` | [C] `0x0C1558`, [C] `0x00C9D2` | bounded by source structure |
| `0x00C444` | jsr longword-pointer table (stride 4) | [C] ROM `0x1D90C0` | 8 | `0x059B28`, `0x059B34`, `0x059B7E`, `0x059A9A`, `0x059BCC`, `0x059C2A`, `0x059C48`, `0x059C54` | [C] `0x1D90C0`, [C] `0x059A6A` | bounded by source structure |
| `0x011768` | jmp longword-pointer table (stride 4) | [C] ROM `0x0D2A0A` | 5 | `0x011B58`, `0x00D01C`, `0x011950`, `0x00FBA6`, `0x00C2A6` | [C] `0x0D2A0A`, [C] `0x011752` | bounded by source structure |
| `0x011884` | jsr longword-pointer table (stride 4) | [C] ROM `0x0D2A1E` | 4 | `0x012168`, `0x012634`, `0x012836`, `0x011040` | [C] `0x0D2A1E`, [C] `0x01187A` | bounded by source structure |
| `0x015928` | jsr longword-pointer table (stride 4) | [C] ROM `0x004964` | 15 | `0x0049A0`, `0x0049A0`, `0x0049A0`, `0x0049A0`, `0x004DCA`, `0x0049A0`, `0x0049A0`, `0x00B574`, `0x0049A0`, `0x0049A0`, `0x0049A0`, `0x004DFE`, `0x00B574`, `0x00B574`, `0x00B574` | [C] `0x004964`, [C] `0x01591E` | bounded by source structure |
| `0x01597A` | jsr longword-pointer table (stride 4) | [C] ROM `0x004964` | 15 | `0x0049A0`, `0x0049A0`, `0x0049A0`, `0x0049A0`, `0x004DCA`, `0x0049A0`, `0x0049A0`, `0x00B574`, `0x0049A0`, `0x0049A0`, `0x0049A0`, `0x004DFE`, `0x00B574`, `0x00B574`, `0x00B574` | [C] `0x004964`, [C] `0x015970` | bounded by source structure |
| `0x01640A` | jsr active-record callback (+0x8) | RAM `0xFFD740` | 2 | `0x01641A`, `0x01669A` | [C] `0x0164C8`, [C] `0x016662` | bounded by source structure |
| `0x018610` | jsr longword-pointer table (stride 4) | [C] ROM `0x0E2DFE` | 3 | `0x018AA6`, `0x018D1A`, `0x019018` | [C] `0x0E2DFE`, [C] `0x018606` | bounded by source structure |
| `0x019A1C` | jsr longword-pointer table (stride 4) | [C] ROM `0x0E2E0A` | 12 | `0x01B734`, `0x01B696`, `0x01B81A`, `0x01B8BC`, `0x01C22A`, `0x019C6C`, `0x01C1C0`, `0x01B7EC`, `0x01C146`, `0x01BAF8`, `0x01BC72`, `0x01BE82` | [C] `0x0E2E0A`, [C] `0x019A12` | bounded by source structure |
| `0x01B892` | jsr longword-pointer table (stride 4) | [C] ROM `0x01BAE4` | 5 | `0x01B92C`, `0x01B95A`, `0x01B986`, `0x01B9AE`, `0x01B9CA` | [C] `0x01BAE4`, [C] `0x01B886` | bounded by source structure |
| `0x01E728` | jsr longword-pointer table (stride 4) | [C] ROM `0x0F3DC6` | 22 | `0x01E744`, `0x01E9C0`, `0x01E9C0`, `0x01EC56`, `0x01E810`, `0x01E810`, `0x01ED18`, `0x01ED2C`, `0x01ED24`, `0x01E81C`, `0x01E7D0`, `0x01E81C`, `0x01E7D0`, `0x01E770`, `0x01EA94`, `0x01EB8E`, `0x01EC88`, `0x01E87A`, `0x01E8FA`, `0x01EDF2`, `0x01EDF2`, `0x01EDF2` | [C] `0x0F3DC6`, [C] `0x01E6A4` | bounded by source structure |
| `0x01EF28` | jsr longword-pointer table (stride 4) | [C] ROM `0x0F3E56` | 6 | `0x01F1FC`, `0x01EF2C`, `0x01EF2C`, `0x01F086`, `0x01F046`, `0x01F106` | [C] `0x0F3E56`, [C] `0x01EF0C` | bounded by source structure |
| `0x01F58A` | jsr longword-pointer table (stride 4) | [C] ROM `0x12CCB4` | 250 | `0x021326`, `0x021332`, `0x021318`, `0x021226`, `0x0211E8`, `0x0213FA`, `0x021458`, `0x02148A`, `0x0214B0`, `0x0214CE`, `0x0214F6`, `0x02155E`, `0x0215AC`, `0x0215DA`, `0x02076A`, `0x0208A8`, `0x021628`, `0x021692`, `0x021774`, `0x021846`, `0x021862`, `0x0218B2`, `0x0218BC`, `0x0218C6`, `0x0218DA`, `0x0218E4`, `0x0218F8`, `0x021902`, `0x02193A`, `0x021962`, `0x021982`, `0x021992`, `0x0219A4`, `0x0219B6`, `0x0219C8`, `0x0219DA`, `0x0219EA`, `0x021A0E`, `0x021A32`, `0x021A66`, `0x021A84`, `0x021AE6`, `0x021B6A`, `0x021B82`, `0x021BE4`, `0x021C0C`, `0x021C38`, `0x021C72`, `0x0213CA`, `0x0213E2`, `0x02156C`, `0x021CAA`, `0x021CE4`, `0x021DCC`, `0x021DEC`, `0x021E08`, `0x021E66`, `0x021E8C`, `0x021E9E`, `0x021EB0`, `0x021ED0`, `0x021EE8`, `0x021EF8`, `0x021F20`, `0x021F32`, `0x021F6E`, `0x021F80`, `0x021FB8`, `0x021FD0`, `0x021FF2`, `0x022018`, `0x022072`, `0x0220CC`, `0x0220EA`, `0x022108`, `0x022112`, `0x022124`, `0x022136`, `0x022162`, `0x02218E`, `0x0221BA`, `0x0221E6`, `0x022212`, `0x02223E`, `0x022276`, `0x022296`, `0x0222BA`, `0x0222DA`, `0x02235C`, `0x022452`, `0x0224B4`, `0x022512`, `0x02252A`, `0x022542`, `0x02256A`, `0x0225B6`, `0x0225D2`, `0x022648`, `0x022688`, `0x0226A0`, `0x0226CA`, `0x02274A`, `0x0227DC`, `0x0227F8`, `0x022898`, `0x0228D0`, `0x022908`, `0x022934`, `0x02296E`, `0x0229A8`, `0x0229D4`, `0x0229FA`, `0x022A1C`, `0x022A54`, `0x022A9E`, `0x022AF4`, `0x022B4A`, `0x022BA0`, `0x022BF6`, `0x022C22`, `0x022C46`, `0x022EC6`, `0x022EE8`, `0x022F14`, `0x022C98`, `0x022CC8`, `0x022CE8`, `0x022D18`, `0x022D38`, `0x022D5A`, `0x022D86`, `0x022DB2`, `0x022DDE`, `0x022E18`, `0x022E52`, `0x022E8C`, `0x022F40`, `0x022F64`, `0x022F90`, `0x02300E`, `0x023106`, `0x02317A`, `0x023222`, `0x02323A`, `0x023256`, `0x02330A`, `0x023322`, `0x02333A`, `0x0233A6`, `0x023412`, `0x023452`, `0x02354A`, `0x02371A`, `0x023782`, `0x023812`, `0x023876`, `0x0238EA`, `0x023920`, `0x023978`, `0x0239C8`, `0x023A3A`, `0x023A72`, `0x023A84`, `0x023A8C`, `0x023AA8`, `0x023B0C`, `0x023B5A`, `0x023B88`, `0x023BB4`, `0x023C38`, `0x023C52`, `0x023CB4`, `0x023CE0`, `0x023CF2`, `0x023D04`, `0x023D32`, `0x023D4E`, `0x023E42`, `0x023E76`, `0x023EA2`, `0x023ED6`, `0x023F02`, `0x023F36`, `0x023F62`, `0x023F7A`, `0x023FA4`, `0x023FCC`, `0x023FF4`, `0x02401C`, `0x024034`, `0x02404C`, `0x024064`, `0x02407C`, `0x024094`, `0x0240AC`, `0x0240C4`, `0x0240DC`, `0x0240F4`, `0x02410C`, `0x024124`, `0x024152`, `0x02416A`, `0x0241A4`, `0x02429E`, `0x0242FA`, `0x024332`, `0x024392`, `0x0243CA`, `0x024432`, `0x024482`, `0x024402`, `0x02441A`, `0x0244D2`, `0x02451C`, `0x02457C`, `0x0245AA`, `0x0245F4`, `0x024622`, `0x024662`, `0x02466C`, `0x0246AA`, `0x0246FC`, `0x024724`, `0x0247D0`, `0x024846`, `0x02489A`, `0x024918`, `0x024936`, `0x024954`, `0x024964`, `0x02497C`, `0x0249A4`, `0x0230EE`, `0x0249C4`, `0x0249FA`, `0x024A1E`, `0x024A56`, `0x024AA8`, `0x024B5A`, `0x024BF0`, `0x024C34`, `0x024C4C`, `0x024C96`, `0x023710`, `0x024CF4`, `0x024D0C`, `0x024D9C`, `0x024DBE`, `0x024DE0`, `0x024DFE` | [C] `0x12CCB4`, [C] `0x01F580` | bounded by source structure |
| `0x0214C4` | jsr longword-pointer table (stride 8) | [C] ROM `0x0F3E1E` | 6 | `0x01E78C`, `0x01EAB4`, `0x01EBAA`, `0x01ECA4`, `0x01E896`, `0x01E91A` | [C] `0x0F3E1E`, [C] `0x0214B0` | bounded by source structure |
| `0x0214EC` | jsr runtime record-field dispatch (stride 8, offset 4) | [C] ROM `0x0F3E1E` | 2 | `0x01EB32`, `0x01E9AC` | [C] `0x0F3E1E`, [C] `0x0214CE`, [C] `0x0214E8` | bounded by source structure |
| `0x024E32` | jsr PC-relative word-offset table | [C] ROM `0x024E3A` | 5 | `0x024E44`, `0x024E84`, `0x024EAC`, `0x024F28`, `0x024F16` | [C] `0x024E32`, [C] `0x024E24`, [C] `0x024E3A` | bounded by source structure |
| `0x02510A` | jmp inline fixed-stride candidates | [C] ROM `0x02510E` | 160 | `0x02510E`, `0x025120`, `0x025132`, `0x025144`, `0x025156`, `0x025168`, `0x02517A`, `0x02518C`, `0x02519E`, `0x0251B0`, `0x0251C2`, `0x0251D4`, `0x0251E6`, `0x0251F8`, `0x02520A`, `0x02521C`, `0x02522E`, `0x025240`, `0x025252`, `0x025264`, `0x025276`, `0x025288`, `0x02529A`, `0x0252AC`, `0x0252BE`, `0x0252D0`, `0x0252E2`, `0x0252F4`, `0x025306`, `0x025318`, `0x02532A`, `0x02533C`, `0x02534E`, `0x025360`, `0x025372`, `0x025384`, `0x025396`, `0x0253A8`, `0x0253BA`, `0x0253CC`, `0x0253DE`, `0x0253F0`, `0x025402`, `0x025414`, `0x025426`, `0x025438`, `0x02544A`, `0x02545C`, `0x02546E`, `0x025480`, `0x025492`, `0x0254A4`, `0x0254B6`, `0x0254C8`, `0x0254DA`, `0x0254EC`, `0x0254FE`, `0x025510`, `0x025522`, `0x025534`, `0x025546`, `0x025558`, `0x02556A`, `0x02557C`, `0x02558E`, `0x0255A0`, `0x0255B2`, `0x0255C4`, `0x0255D6`, `0x0255E8`, `0x0255FA`, `0x02560C`, `0x02561E`, `0x025630`, `0x025642`, `0x025654`, `0x025666`, `0x025678`, `0x02568A`, `0x02569C`, `0x0256AE`, `0x0256C0`, `0x0256D2`, `0x0256E4`, `0x0256F6`, `0x025708`, `0x02571A`, `0x02572C`, `0x02573E`, `0x025750`, `0x025762`, `0x025774`, `0x025786`, `0x025798`, `0x0257AA`, `0x0257BC`, `0x0257CE`, `0x0257E0`, `0x0257F2`, `0x025804`, `0x025816`, `0x025828`, `0x02583A`, `0x02584C`, `0x02585E`, `0x025870`, `0x025882`, `0x025894`, `0x0258A6`, `0x0258B8`, `0x0258CA`, `0x0258DC`, `0x0258EE`, `0x025900`, `0x025912`, `0x025924`, `0x025936`, `0x025948`, `0x02595A`, `0x02596C`, `0x02597E`, `0x025990`, `0x0259A2`, `0x0259B4`, `0x0259C6`, `0x0259D8`, `0x0259EA`, `0x0259FC`, `0x025A0E`, `0x025A20`, `0x025A32`, `0x025A44`, `0x025A56`, `0x025A68`, `0x025A7A`, `0x025A8C`, `0x025A9E`, `0x025AB0`, `0x025AC2`, `0x025AD4`, `0x025AE6`, `0x025AF8`, `0x025B0A`, `0x025B1C`, `0x025B2E`, `0x025B40`, `0x025B52`, `0x025B64`, `0x025B76`, `0x025B88`, `0x025B9A`, `0x025BAC`, `0x025BBE`, `0x025BD0`, `0x025BE2`, `0x025BF4`, `0x025C06`, `0x025C18`, `0x025C2A`, `0x025C3C` | [C] `0x0250F8`, [C] `0x02510A` | candidate only; input bound open |
| `0x053AA4` | jsr longword-pointer table (stride 4) | [C] ROM `0x15FD5C` | 23 | `0x053AB6`, `0x053B16`, `0x053B78`, `0x053BCC`, `0x053CEA`, `0x0049A0`, `0x0049A0`, `0x0049A0`, `0x0049A0`, `0x0049A0`, `0x053F8C`, `0x0049A0`, `0x054076`, `0x0049A0`, `0x054152`, `0x054222`, `0x0049A0`, `0x0049A0`, `0x054322`, `0x0543A2`, `0x0049A0`, `0x05447E`, `0x0545B6` | [C] `0x15FD5C`, [C] `0x053A92` | bounded by source structure |
| `0x054960` | jsr longword-pointer table (stride 4) | [C] ROM `0x1664D8` | 1 | `0x166514` | [C] `0x1664D8`, [C] `0x05492E` | bounded by source structure |
| `0x05AF48` | jsr schedule delay/callback records | [C] ROM `0x05ADD0` | 5 | `0x05AEC6`, `0x05AEDA`, `0x05AEEE`, `0x05B378`, `0x0049A0` | [C] `0x05ADD0`, [C] `0x05AFA2` | bounded by source structure |
| `0x05AF48` | jsr schedule delay/callback records | [C] ROM `0x05ADF0` | 2 | `0x05AE9E`, `0x0049A0` | [C] `0x05ADF0`, [C] `0x059D82` | bounded by source structure |

## Unresolved indirect jumps

Indirect `JMP` instructions without a discovered table are listed here. Unreached sites are included because a preceding unresolved call can hide their callers.

| Reachability | Site | Instruction | Resolution |
|---|---:|---|---|
| not reached from known roots | `0x02510A` | `jmp (0x2510e,PC,D7w*0x1)` | candidate inline range `[C] 0x02510E`: 160 starts; input bound not established |

## Indirect calls

Confirmed call-site targets and bounded discovered dispatch sources are traversed; unresolved calls still have their return-site fallthrough explored.

| Site | Instruction | Target / status | Callee |
|---:|---|---|---|
| `0x0046F6` | `jsr (a1)` | `0x0049A0` | noop_effect_callback |
| `0x0046F6` | `jsr (a1)` | `0x004CDE` | dispatch target |
| `0x0046F6` | `jsr (a1)` | `0x004D54` | dispatch target |
| `0x0046F6` | `jsr (a1)` | `0x004FE2` | dispatch target |
| `0x0046F6` | `jsr (a1)` | `0x00508A` | dispatch target |
| `0x0046F6` | `jsr (a1)` | `0x005152` | dispatch target |
| `0x0046F6` | `jsr (a1)` | `0x00521E` | dispatch target |
| `0x0046F6` | `jsr (a1)` | `0x0052BE` | dispatch target |
| `0x0046F6` | `jsr (a1)` | `0x0053C4` | run_effect_variant_a |
| `0x0046F6` | `jsr (a1)` | `0x00557E` | run_effect_variant_b |
| `0x0046F6` | `jsr (a1)` | `0x0056DE` | dispatch target |
| `0x00AAB0` | `jsr (a1)` | `0x00B49C` | resolve_entity_fallback_movement |
| `0x00C444` | `jsr (a0)` | `0x00C448` | dispatch target |
| `0x00C444` | `jsr (a0)` | `0x00C8C8` | dispatch target |
| `0x00C444` | `jsr (a0)` | `0x00CA16` | dispatch target |
| `0x00C444` | `jsr (a0)` | `0x00CA2E` | dispatch target |
| `0x00C444` | `jsr (a0)` | `0x00CE7C` | dispatch target |
| `0x00C444` | `jsr (a0)` | `0x00D024` | dispatch target |
| `0x00C444` | `jsr (a0)` | `0x00DA6A` | dispatch target |
| `0x00C444` | `jsr (a0)` | `0x00EFB4` | dispatch target |
| `0x00C444` | `jsr (a0)` | `0x00F236` | dispatch target |
| `0x00C444` | `jsr (a0)` | `0x018150` | dispatch target |
| `0x00C444` | `jsr (a0)` | `0x0599DA` | dispatch target |
| `0x00C444` | `jsr (a0)` | `0x059A9A` | dispatch target |
| `0x00C444` | `jsr (a0)` | `0x059B28` | dispatch target |
| `0x00C444` | `jsr (a0)` | `0x059B34` | dispatch target |
| `0x00C444` | `jsr (a0)` | `0x059B7E` | dispatch target |
| `0x00C444` | `jsr (a0)` | `0x059BCC` | reset_runner_status_triplet |
| `0x00C444` | `jsr (a0)` | `0x059C2A` | dispatch target |
| `0x00C444` | `jsr (a0)` | `0x059C48` | dispatch target |
| `0x00C444` | `jsr (a0)` | `0x059C54` | dispatch target |
| `0x011884` | `jsr (a0)` | `0x011040` | dispatch target |
| `0x011884` | `jsr (a0)` | `0x012168` | dispatch target |
| `0x011884` | `jsr (a0)` | `0x012634` | dispatch target |
| `0x011884` | `jsr (a0)` | `0x012836` | dispatch target |
| `0x015928` | `jsr (a2)` | `0x0049A0` | noop_effect_callback |
| `0x015928` | `jsr (a2)` | `0x004DCA` | dispatch target |
| `0x015928` | `jsr (a2)` | `0x004DFE` | dispatch target |
| `0x015928` | `jsr (a2)` | `0x00B574` | dispatch target |
| `0x01597A` | `jsr (a2)` | `0x0049A0` | noop_effect_callback |
| `0x01640A` | `jsr (a1)` | `0x01641A` | dispatch target |
| `0x01640A` | `jsr (a1)` | `0x01669A` | dispatch target |
| `0x018610` | `jsr (a6)` | `0x018AA6` | run_cyber_info_screen |
| `0x019A1C` | `jsr (a4)` | `0x019C6C` | dispatch target |
| `0x019A1C` | `jsr (a4)` | `0x01B696` | dispatch target |
| `0x019A1C` | `jsr (a4)` | `0x01B734` | dispatch target |
| `0x019A1C` | `jsr (a4)` | `0x01B7EC` | dispatch target |
| `0x019A1C` | `jsr (a4)` | `0x01B81A` | dispatch target |
| `0x019A1C` | `jsr (a4)` | `0x01B8BC` | dispatch target |
| `0x019A1C` | `jsr (a4)` | `0x01BAF8` | dispatch target |
| `0x019A1C` | `jsr (a4)` | `0x01BC72` | dispatch target |
| `0x019A1C` | `jsr (a4)` | `0x01BE82` | dispatch target |
| `0x019A1C` | `jsr (a4)` | `0x01C146` | dispatch target |
| `0x019A1C` | `jsr (a4)` | `0x01C1C0` | dispatch target |
| `0x019A1C` | `jsr (a4)` | `0x01C22A` | dispatch target |
| `0x01B892` | `jsr (a1)` | `0x01B92C` | dispatch target |
| `0x01B892` | `jsr (a1)` | `0x01B95A` | dispatch target |
| `0x01B892` | `jsr (a1)` | `0x01B986` | dispatch target |
| `0x01B892` | `jsr (a1)` | `0x01B9AE` | dispatch target |
| `0x01B892` | `jsr (a1)` | `0x01B9CA` | dispatch target |
| `0x01E728` | `jsr (a0)` | `0x01E744` | dispatch target |
| `0x01E728` | `jsr (a0)` | `0x01E770` | dispatch target |
| `0x01E728` | `jsr (a0)` | `0x01E7D0` | dispatch target |
| `0x01E728` | `jsr (a0)` | `0x01E810` | dispatch target |
| `0x01E728` | `jsr (a0)` | `0x01E81C` | dispatch target |
| `0x01E728` | `jsr (a0)` | `0x01E87A` | dispatch target |
| `0x01E728` | `jsr (a0)` | `0x01E8FA` | dispatch target |
| `0x01E728` | `jsr (a0)` | `0x01E9C0` | dispatch target |
| `0x01E728` | `jsr (a0)` | `0x01EA94` | dispatch target |
| `0x01E728` | `jsr (a0)` | `0x01EB8E` | dispatch target |
| `0x01E728` | `jsr (a0)` | `0x01EC56` | dispatch target |
| `0x01E728` | `jsr (a0)` | `0x01EC88` | dispatch target |
| `0x01E728` | `jsr (a0)` | `0x01ED18` | dispatch target |
| `0x01E728` | `jsr (a0)` | `0x01ED24` | dispatch target |
| `0x01E728` | `jsr (a0)` | `0x01ED2C` | dispatch target |
| `0x01E728` | `jsr (a0)` | `0x01EDF2` | dispatch target |
| `0x01EF28` | `jsr (a1)` | `0x01EF2C` | dispatch target |
| `0x01EF28` | `jsr (a1)` | `0x01F046` | dispatch target |
| `0x01EF28` | `jsr (a1)` | `0x01F086` | dispatch target |
| `0x01EF28` | `jsr (a1)` | `0x01F106` | dispatch target |
| `0x01EF28` | `jsr (a1)` | `0x01F1FC` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02076A` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0208A8` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0211E8` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021226` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021318` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021326` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021332` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0213CA` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0213E2` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0213FA` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021458` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02148A` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0214B0` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0214CE` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0214F6` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02155E` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02156C` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0215AC` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0215DA` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021628` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021692` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021774` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021846` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021862` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0218B2` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0218BC` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0218C6` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0218DA` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0218E4` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0218F8` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021902` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02193A` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021962` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021982` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021992` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0219A4` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0219B6` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0219C8` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0219DA` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0219EA` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021A0E` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021A32` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021A66` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021A84` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021AE6` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021B6A` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021B82` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021BE4` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021C0C` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021C38` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021C72` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021CAA` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021CE4` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021DCC` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021DEC` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021E08` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021E66` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021E8C` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021E9E` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021EB0` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021ED0` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021EE8` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021EF8` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021F20` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021F32` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021F6E` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021F80` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021FB8` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021FD0` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x021FF2` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022018` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022072` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0220CC` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0220EA` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022108` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022112` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022124` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022136` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022162` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02218E` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0221BA` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0221E6` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022212` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02223E` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022276` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022296` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0222BA` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0222DA` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02235C` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022452` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0224B4` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022512` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02252A` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022542` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02256A` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0225B6` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0225D2` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022648` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022688` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0226A0` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0226CA` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02274A` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0227DC` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0227F8` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022898` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0228D0` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022908` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022934` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02296E` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0229A8` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0229D4` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0229FA` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022A1C` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022A54` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022A9E` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022AF4` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022B4A` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022BA0` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022BF6` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022C22` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022C46` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022C98` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022CC8` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022CE8` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022D18` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022D38` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022D5A` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022D86` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022DB2` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022DDE` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022E18` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022E52` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022E8C` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022EC6` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022EE8` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022F14` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022F40` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022F64` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x022F90` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02300E` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0230EE` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023106` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02317A` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023222` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02323A` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023256` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02330A` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023322` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02333A` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0233A6` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023412` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023452` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02354A` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023710` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02371A` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023782` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023812` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023876` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0238EA` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023920` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023978` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0239C8` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023A3A` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023A72` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023A84` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023A8C` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023AA8` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023B0C` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023B5A` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023B88` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023BB4` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023C38` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023C52` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023CB4` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023CE0` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023CF2` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023D04` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023D32` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023D4E` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023E42` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023E76` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023EA2` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023ED6` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023F02` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023F36` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023F62` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023F7A` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023FA4` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023FCC` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x023FF4` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02401C` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024034` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02404C` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024064` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02407C` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024094` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0240AC` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0240C4` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0240DC` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0240F4` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02410C` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024124` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024152` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02416A` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0241A4` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02429E` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0242FA` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024332` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024392` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0243CA` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024402` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02441A` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024432` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024482` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0244D2` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02451C` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02457C` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0245AA` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0245F4` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024622` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024662` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02466C` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0246AA` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0246FC` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024724` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0247D0` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024846` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02489A` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024918` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024936` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024954` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024964` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x02497C` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0249A4` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0249C4` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x0249FA` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024A1E` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024A56` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024AA8` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024B5A` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024BF0` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024C34` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024C4C` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024C96` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024CF4` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024D0C` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024D9C` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024DBE` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024DE0` | dispatch target |
| `0x01F58A` | `jsr (a0)` | `0x024DFE` | dispatch target |
| `0x0214C4` | `jsr (a0)` | `0x01E78C` | dispatch target |
| `0x0214C4` | `jsr (a0)` | `0x01E896` | dispatch target |
| `0x0214C4` | `jsr (a0)` | `0x01E91A` | dispatch target |
| `0x0214C4` | `jsr (a0)` | `0x01EAB4` | dispatch target |
| `0x0214C4` | `jsr (a0)` | `0x01EBAA` | dispatch target |
| `0x0214C4` | `jsr (a0)` | `0x01ECA4` | dispatch target |
| `0x0214EC` | `jsr (a0)` | `0x01E9AC` | dispatch target |
| `0x0214EC` | `jsr (a0)` | `0x01EB32` | dispatch target |
| `0x024E32` | `jsr (a0)` | `0x024E44` | dispatch target |
| `0x024E32` | `jsr (a0)` | `0x024E84` | dispatch target |
| `0x024E32` | `jsr (a0)` | `0x024EAC` | dispatch target |
| `0x024E32` | `jsr (a0)` | `0x024F16` | dispatch target |
| `0x024E32` | `jsr (a0)` | `0x024F28` | dispatch target |
| `0x053AA4` | `jsr (a1)` | `0x0049A0` | noop_effect_callback |
| `0x053AA4` | `jsr (a1)` | `0x053AB6` | dispatch target |
| `0x053AA4` | `jsr (a1)` | `0x053B16` | dispatch target |
| `0x053AA4` | `jsr (a1)` | `0x053B78` | dispatch target |
| `0x053AA4` | `jsr (a1)` | `0x053BCC` | dispatch target |
| `0x053AA4` | `jsr (a1)` | `0x053CEA` | dispatch target |
| `0x053AA4` | `jsr (a1)` | `0x053F8C` | dispatch target |
| `0x053AA4` | `jsr (a1)` | `0x054076` | dispatch target |
| `0x053AA4` | `jsr (a1)` | `0x054152` | dispatch target |
| `0x053AA4` | `jsr (a1)` | `0x054222` | dispatch target |
| `0x053AA4` | `jsr (a1)` | `0x054322` | dispatch target |
| `0x053AA4` | `jsr (a1)` | `0x0543A2` | dispatch target |
| `0x053AA4` | `jsr (a1)` | `0x05447E` | dispatch target |
| `0x053AA4` | `jsr (a1)` | `0x0545B6` | dispatch target |
| `0x054960` | `jsr (a0)` | `0x166514` | dispatch target |
| `0x05AF48` | `jsr (a1)` | `0x0049A0` | noop_effect_callback |
| `0x05AF48` | `jsr (a1)` | `0x05AE9E` | dispatch target |
| `0x05AF48` | `jsr (a1)` | `0x05AEC6` | dispatch target |
| `0x05AF48` | `jsr (a1)` | `0x05AEDA` | dispatch target |
| `0x05AF48` | `jsr (a1)` | `0x05AEEE` | dispatch target |
| `0x05AF48` | `jsr (a1)` | `0x05B378` | dispatch target |
| `0x000B42` | `jsr (a1)` | indirect/unresolved | — |
| `0x01F26E` | `jsr (a1)` | indirect/unresolved | — |

## Method and limits

- Read the 64 longword vector slots at `0x000000..0x0000FF`; slot zero is the initial stack pointer and is not treated as code.
- Decode reachable instructions from the raw big-endian ROM using Capstone's M68000 decoder. Follow direct conditional branches, DBcc branches, direct calls, and direct jumps. Calls keep their return-site fallthrough; unconditional jumps and returns do not.
- Follow Ghidra `call-sites.tsv` indirect-call rows only when `status=resolved` and the target is present. Recognize indexed longword pointer loads, caller-supplied tables, ten active-effect callback slots, and title schedule records. For indexed PC jumps, derive fixed-stride candidate starts from the inline arithmetic and branch boundary but leave them unresolved when the record value has no proven upper bound. The full `disassembly.s` export inventories indirect JMP sites, including sites not reached because of an earlier unresolved call.
- Merge decoded instruction bytes, subtract exported function extents and the entry instruction for status-only entries, then merge the remaining adjacent bytes into ranges.
- This report does not claim completeness for indirect targets, computed dispatches without a recognized pointer table, invalid decode boundaries, or data/code ambiguities. Table entries are candidate targets and do not establish the runtime index domain.

Regenerate with `python3 decompilation/ghidra_conversion/tools/rom_reachability.py --write-report`; validate with `python3 decompilation/ghidra_conversion/tools/rom_reachability.py --check`.
