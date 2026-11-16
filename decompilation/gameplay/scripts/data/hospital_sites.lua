-- Same-map hospital sites. ROM 0x1F830 matches FFFBF8/FFFBF9 (scene/event),
-- stores the text root in FFF0B6, runs the shared mode-exit prompt 0x1F246
-- and, when the reply sets FFF0D5, opens Hospital Options 0xC928 [C].
-- The party never leaves the map; Exit -> Yes is the 0x9B8 + 0x2FA4
-- service return [C emu: Little Chiba's, doc 27 section 6].
-- Keyed by the entity link's location id. `cyberware_catalog` names the
-- shop_catalog view that the shared 0xCA16 -> 0x56AC0 (FFF0B7=1) service
-- opens; the views share Wire-Masters' stock and differ only in the
-- FFF0B6-selected title (0x56B4E indexes 0x1AAB50).
return {
    downtown_site_12_seattle_general_hospital = {
        narrator = "council_island_hospital_narrator",
        text_profile = "council_island_hospital",
        entry_text = "seattle_general_hospital.entry.default",
        -- ROM 0x1F86E/0x1F864 selects Ivanna's Seattle-specific package pages.
        package_result_speaker = "ivanna_haversheen",
        package_handoff_event = "seattle_package_handoff",
        package_payment_event = "seattle_package_payment",
        -- ROM 0x1F84E: FFF0B6 = 0x0B ("Seattle General Hospital").
        cyberware_catalog = "seattle_general_hospital_cyberware",
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
        -- ROM 0x1FA20: FFF0B6 = 0x0A ("Council Island Hospital").
        cyberware_catalog = "council_island_hospital_cyberware",
    },
    little_chibas_chop_shop = {
        narrator = "little_chiba_narrator",
        text_profile = "little_chiba",
        entry_text = "field.mode.little_chiba.description",
        -- ROM 0x1F97C: FFF0B6 = 0x08 ("Little Chiba").
        cyberware_catalog = "little_chiba_cyberware",
    },
}
