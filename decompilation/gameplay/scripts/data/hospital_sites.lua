-- Same-map hospital sites. ROM 0x1F830 matches FFFBF8/FFFBF9 (scene/event),
-- stores the text root in FFF0B6, runs the shared mode-exit prompt 0x1F246
-- and, when the reply sets FFF0D5, opens Hospital Options 0xC928 [C].
-- The party never leaves the map; Exit -> Yes is the 0x9B8 + 0x2FA4
-- service return [C emu: Little Chiba's, doc 27 section 6].
-- Keyed by the entity link's location id.
return {
    downtown_site_12_seattle_general_hospital = {
        narrator = "council_island_hospital_narrator",
        text_profile = "council_island_hospital",
        entry_text = "seattle_general_hospital.entry.default",
        -- ROM 0x1F878 checks FFFC30 bit 3 and selects text at 0x111A7B.
        entry_text_variants = {
            {
                fact = "field_mode_flag_bit_3",
                text = "seattle_general_hospital.entry.michael_holopix",
            },
        },
    },
    council_island_hospital = {
        narrator = "council_island_hospital_narrator",
        text_profile = "council_island_hospital",
        entry_text = "council_island_hospital.entry.default",
    },
    little_chibas_chop_shop = {
        narrator = "little_chiba_narrator",
        text_profile = "little_chiba",
        entry_text = "field.mode.little_chiba.description",
    },
}
