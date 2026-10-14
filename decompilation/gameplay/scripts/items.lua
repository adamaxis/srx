-- One source-backed catalog for all 50 US-ROM item records. Names and
-- categories resolve through the extracted dialogue text. Weapon and armor
-- stats live on the item they describe instead of in parallel partial lists.
local Items = {}

local image_family_by_kind = {
    firearm = "weapons",
    grenade = "weapons",
    medical_kit = "first.aid",
    stim_patch = "first.aid",
    trauma_patch = "first.aid",
    fetish = "miscellaneous",
    passkey = "miscellaneous",
    electronic_kit = "miscellaneous",
    smart_goggles = "miscellaneous",
    enchanted_dagger = "magic",
    power_focus = "magic",
    spell_focus = "magic",
    talisman = "magic",
    spell_lock = "magic",
    totem = "magic",
    armor = "armor",
}

local function item_image_suffix(code)
    return code < 10 and ("0" .. code) or tostring(code)
end

local function label_keys(ids, prefix)
    local keys = {}
    for index, id in ipairs(ids) do
        keys[index] = prefix .. id
    end
    return keys
end

local function define_item(id, code, kind, base_cost, has_detail_art, content)
    local image_family = image_family_by_kind[kind]
    assert(image_family ~= nil, "item kind has no image family: " .. kind)
    local image_suffix = item_image_suffix(code)
    local item = {
        id = id,
        code = code,
        kind = kind,
        name_key = "items.name." .. code,
        category_key = "items.category." .. code,
        -- D4540 +0x27 is consumed by 0x13D0C while +0xB5 is active.
        -- The complete US table is zero except item 43, whose value is 2.
        action_motion_distance_profile = 0,
        pricing = {
            -- D4540 +0x22 is the purchase base. The source sale path starts
            -- from half of that value before applying runner-dependent cost
            -- adjustments, so retain both static terms on this item.
            purchase_base = base_cost,
            resale_base = math.floor(base_cost / 2),
        },
        has_detail_art = has_detail_art,
        presentation = {
            icon_resource_key = "items." .. image_family ..
                ".icons.item." .. image_suffix,
            detail_resource_key = "items." .. image_family ..
                ".detail.item." .. image_suffix,
        },
    }
    if content ~= nil then
        for key, value in pairs(content) do
            item[key] = value
        end
    end
    return item
end

-- Every row resolves to named icon/detail assets and has source-backed buy
-- and resale bases. Firearm burst cost, firing family, actor-derived attack
-- timing, reload timing, Damage rows, Power labels, Uses, and armor modifiers
-- are bound to the item they describe. Other item details depend on live
-- actor/menu state.
Items.catalog = {
    define_item("streetline", 1, "firearm", 100, true,
        { weapon = { magazine_capacity = 6, damage_bar_index = 0,
                     power_profile = "low", fire_mode = "single_round",
                     trigger_behavior = "repeat_while_held",
                     rounds_per_action = 1,
                     source_action = { effect_table_index = 0,
                                       effect_value = 1, difficulty = 4 },
                     attack_cadence = "runner_attribute",
                     attack_delay_scale = "half",
                     reload_delay_scale = "normal" } }),
    define_item("model_101t", 2, "firearm", 350, true,
        { weapon = { magazine_capacity = 12, damage_bar_index = 0,
                     power_profile = "light", fire_mode = "single_round",
                     trigger_behavior = "repeat_while_held",
                     rounds_per_action = 1,
                     source_action = { effect_table_index = 0,
                                       effect_value = 1, difficulty = 6 },
                     attack_cadence = "runner_attribute",
                     attack_delay_scale = "half",
                     reload_delay_scale = "normal" } }),
    define_item("american_l36", 3, "firearm", 300, true,
        { weapon = { magazine_capacity = 11, damage_bar_index = 0,
                     power_profile = "light", fire_mode = "single_round",
                     trigger_behavior = "repeat_while_held",
                     rounds_per_action = 1,
                     source_action = { effect_table_index = 0,
                                       effect_value = 1, difficulty = 6 },
                     attack_cadence = "runner_attribute",
                     attack_delay_scale = "half",
                     reload_delay_scale = "normal" } }),
    define_item("security_500", 4, "firearm", 450, true,
        { weapon = { magazine_capacity = 25, damage_bar_index = 0,
                     power_profile = "light", fire_mode = "single_round",
                     trigger_behavior = "repeat_while_held",
                     rounds_per_action = 1,
                     source_action = { effect_table_index = 0,
                                       effect_value = 1, difficulty = 6 },
                     attack_cadence = "runner_attribute",
                     attack_delay_scale = "half",
                     reload_delay_scale = "normal" } }),
    define_item("warhawk", 5, "firearm", 500, true,
        { weapon = { magazine_capacity = 6, damage_bar_index = 1,
                     power_profile = "brutal", fire_mode = "single_round",
                     trigger_behavior = "repeat_while_held",
                     rounds_per_action = 1,
                     source_action = { effect_table_index = 1,
                                       effect_value = 3, difficulty = 10 },
                     attack_cadence = "runner_attribute",
                     attack_delay_scale = "half",
                     reload_delay_scale = "double" } }),
    define_item("max_power", 6, "firearm", 450, true,
        { weapon = { magazine_capacity = 10, damage_bar_index = 1,
                     power_profile = "high", fire_mode = "single_round",
                     trigger_behavior = "repeat_while_held",
                     rounds_per_action = 1,
                     source_action = { effect_table_index = 1,
                                       effect_value = 3, difficulty = 9 },
                     attack_cadence = "runner_attribute",
                     attack_delay_scale = "half",
                     reload_delay_scale = "normal" } }),
    define_item("predator", 7, "firearm", 450, true,
        { weapon = { magazine_capacity = 15, damage_bar_index = 1,
                     power_profile = "high", fire_mode = "single_round",
                     trigger_behavior = "repeat_while_held",
                     rounds_per_action = 1,
                     source_action = { effect_table_index = 1,
                                       effect_value = 3, difficulty = 9 },
                     attack_cadence = "runner_attribute",
                     attack_delay_scale = "half",
                     reload_delay_scale = "normal" } }),
    define_item("ak_97", 8, "firearm", 1000, true,
        { weapon = { magazine_capacity = 30, damage_bar_index = 1,
                     power_profile = "light", fire_mode = "three_round_burst",
                     trigger_behavior = "repeat_while_held",
                     rounds_per_action = 3,
                     source_action = { effect_table_index = 1,
                                       effect_value = 3, difficulty = 6 },
                     attack_cadence = "runner_attribute",
                     attack_delay_scale = "half",
                     reload_delay_scale = "normal" } }),
    define_item("hk227_s", 9, "firearm", 1500, true,
        { weapon = { magazine_capacity = 28, damage_bar_index = 1,
                     power_profile = "medium", fire_mode = "three_round_burst",
                     trigger_behavior = "repeat_while_held",
                     rounds_per_action = 3,
                     source_action = { effect_table_index = 1,
                                       effect_value = 3, difficulty = 7 },
                     attack_cadence = "runner_attribute",
                     attack_delay_scale = "half",
                     reload_delay_scale = "normal" } }),
    define_item("mach_22", 10, "firearm", 800, true,
        { weapon = { magazine_capacity = 24, damage_bar_index = 1,
                     power_profile = "light", fire_mode = "three_round_burst",
                     trigger_behavior = "repeat_while_held",
                     rounds_per_action = 3,
                     source_action = { effect_table_index = 1,
                                       effect_value = 3, difficulty = 6 },
                     attack_cadence = "runner_attribute",
                     attack_delay_scale = "half",
                     reload_delay_scale = "normal" } }),
    define_item("allegiance", 11, "firearm", 1400, true,
        { weapon = { magazine_capacity = 5, damage_bar_index = 2,
                     power_profile = "brutal", fire_mode = "single_round",
                     trigger_behavior = "repeat_while_held",
                     rounds_per_action = 1,
                     source_action = { effect_table_index = 2,
                                       effect_value = 6, difficulty = 10 },
                     attack_cadence = "runner_attribute",
                     attack_delay_scale = "normal",
                     reload_delay_scale = "double" } }),
    define_item("roomsweeper", 12, "firearm", 1000, true,
        { weapon = { magazine_capacity = 8, damage_bar_index = 2,
                     power_profile = "high", fire_mode = "single_round",
                     trigger_behavior = "repeat_while_held",
                     rounds_per_action = 1,
                     source_action = { effect_table_index = 2,
                                       effect_value = 6, difficulty = 9 },
                     attack_cadence = "runner_attribute",
                     attack_delay_scale = "normal",
                     reload_delay_scale = "double" } }),
    define_item("frag", 13, "grenade", 300, true,
        { detail = { kind = "uses", value = 6 } }),
    define_item("scatter", 14, "grenade", 180, true,
        { detail = { kind = "uses", value = 6 } }),
    define_item("concussion", 15, "grenade", 180, true,
        { detail = { kind = "uses", value = 6 } }),
    define_item("medkit", 16, "medical_kit", 200, true,
        { detail = { kind = "uses", value = 6 } }),
    define_item("stim_patch", 17, "stim_patch", 300, true,
        { detail = { kind = "uses", value = 6 } }),
    define_item("trauma_patch", 18, "trauma_patch", 300, true,
        { detail = { kind = "uses", value = 6 } }),
    define_item("fetish", 19, "fetish", 500, true,
        { detail = { kind = "uses", value = 0 } }),
    define_item("maglock_passkey", 20, "passkey", 10000, true),
    define_item("electronic_kit", 21, "electronic_kit", 1000, true),
    define_item("smart_goggles", 22, "smart_goggles", 3000, true),
    define_item("enchanted_dagger", 23, "enchanted_dagger", 50000, true),
    define_item("power_focus", 24, "power_focus", 15000, true),
    define_item("flame_dart_focus", 25, "spell_focus", 4000, true),
    define_item("flame_bolt_focus", 26, "spell_focus", 4000, true),
    define_item("hellblast_focus", 27, "spell_focus", 4000, true),
    define_item("invisibility_focus", 28, "spell_focus", 4000, true),
    define_item("sleep_focus", 29, "spell_focus", 4000, true),
    define_item("stink_focus", 30, "spell_focus", 4000, true),
    define_item("supr_barrier_focus", 31, "spell_focus", 4000, true),
    define_item("mana_zap_focus", 32, "spell_focus", 4000, true),
    define_item("mana_blast_focus", 33, "spell_focus", 4000, true),
    define_item("mana_storm_focus", 34, "spell_focus", 4000, true),
    define_item("rockskin_focus", 35, "spell_focus", 4000, true),
    define_item("heal_wounds_focus", 36, "spell_focus", 4000, true),
    define_item("confusion_focus", 37, "spell_focus", 4000, true),
    define_item("barrier_focus", 38, "spell_focus", 4000, true),
    define_item("protection_talisman", 39, "talisman", 17500, true),
    define_item("combat_sense_spell_lock", 40, "spell_lock", 17500, true),
    define_item("rat_totem", 41, "totem", 2, false,
        { source_action_modifier_words = { 0x0002, 0xffff, 0x0000 } }),
    define_item("gator_totem", 42, "totem", 65535, false,
        { source_action_modifier_words = { 0xffff, 0x0002, 0x0000 } }),
    define_item("bear_totem", 43, "totem", 0, false,
        { action_motion_distance_profile = 2,
          source_action_modifier_words = { 0x0000, 0x0000, 0x0202 } }),
    define_item("armor_vest", 44, "armor", 200, true,
        { armor = { armor_index = 1, primary_modifier = 2,
                    secondary_modifier = 1 } }),
    define_item("armor_clothing", 45, "armor", 500, true,
        { armor = { armor_index = 2, primary_modifier = 1,
                    secondary_modifier = 2 } }),
    define_item("armor_jacket", 46, "armor", 900, true,
        { armor = { armor_index = 3, primary_modifier = 5,
                    secondary_modifier = 3 } }),
    define_item("vest_with_plates", 47, "armor", 600, true,
        { armor = { armor_index = 4, primary_modifier = 4,
                    secondary_modifier = 3 } }),
    define_item("lined_duster", 48, "armor", 700, true,
        { armor = { armor_index = 5, primary_modifier = 3,
                    secondary_modifier = 2 } }),
    define_item("light_combat_armor", 49, "armor", 10000, true,
        { armor = { armor_index = 6, primary_modifier = 6,
                    secondary_modifier = 4 } }),
    define_item("heavy_combat_armor", 50, "armor", 20000, true,
        { armor = { armor_index = 7, primary_modifier = 8,
                    secondary_modifier = 6 } }),
}

local detail_text_keys = {}
for index = 0, 31 do
    detail_text_keys[#detail_text_keys + 1] = "items.action_detail." .. index
end

Items.detail_text_keys = detail_text_keys
Items.detail_label_keys = {
    "items.detail_label.ammo",
    "items.detail_label.uses",
    "items.detail_label.rating",
}

-- ROM-ordered action rows; native controllers forward these stable IDs.
function Items.inventory_actions()
    return { "equip_or_use", "discard", "trade", "cancel" }
end

function Items.ui_text_entries()
    local ids = {
        "equipped_prefix", "equipped_suffix", "discard_prompt",
        "cannot_discard", "cannot_trade", "trade_recipient",
        "trade_full_inventory", "automatic_use", "first_aid_prompt",
        "first_aid_label", "first_aid_cancel", "first_aid_not_injured",
        "first_aid_slap_patch_used", "first_aid_medkit_used",
    }
    return ids, nil, label_keys(ids, "items.text.option.")
end

function Items.display_text_entries()
    local ids = {
        "power_meager", "power_minor", "power_very_low", "power_low",
        "power_very_light", "power_light", "power_medium",
        "power_medium_high", "power_high", "power_brutal", "power_severe",
        "power_massive", "power_incredible", "armor_vest_top",
        "armor_vest_bottom", "armor_clothing_top", "armor_clothing_bottom",
        "armor_jacket_top", "armor_jacket_bottom", "armor_plates_top",
        "armor_plates_bottom", "lined_duster_top", "lined_duster_bottom",
        "light_combat_armor_top", "light_combat_armor_bottom",
        "heavy_combat_armor_top", "heavy_combat_armor_bottom",
    }
    return ids, nil, label_keys(ids, "items.display.option.")
end

return Items
