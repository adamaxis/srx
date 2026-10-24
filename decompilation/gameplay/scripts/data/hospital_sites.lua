-- Same-map hospital sites. ROM 0x1F830 matches FFFBF8/FFFBF9 (scene/event),
-- stores the text root in FFF0B6, runs the shared mode-exit prompt 0x1F246
-- and, when the reply sets FFF0D5, opens Hospital Options 0xC928 [C].
-- The party never leaves the map; Exit -> Yes is the 0x9B8 + 0x2FA4
-- service return [C emu: Little Chiba's, doc 27 section 6].
-- Keyed by the entity link's location id.
return {
    council_island_hospital = {
        mode = 4, event = 4, text_root = 0x0A,   -- 0x1FA02
        narrator = "council_island_hospital_narrator",
        text_profile = "council_island_hospital",
        entry_text = "council_island_hospital.entry.default",
    },
    little_chibas_chop_shop = {
        mode = 2, event = 1, text_root = 0x08,   -- 0x1F96A -> tail 0x1FA50
        narrator = "little_chiba_narrator",
        text_profile = "little_chiba",
        entry_text = "field.mode.little_chiba.description",
    },
}
