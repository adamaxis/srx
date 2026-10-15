-- Stable spell IDs follow the ROM D4EA0 name/description table order.
-- The native spell table filters this catalog for the active runner.
local Magic = {}
local SourceSpellRecords = require("data.spells").by_type

local spell_ids = {
    "flame_dart",
    "flame_bolt",
    "hellblast",
    "invisibility",
    "sleep",
    "stink",
    "super_barrier",
    "mana_zap",
    "mana_blast",
    "mana_storm",
    "rockskin",
    "heal_wounds",
    "confusion",
}

-- D4EA0 action rows in one-based live-record action-code order. The row
-- stores caster residual index at +0x24, target effect index/availability at
-- +0x25, difficulty modifier at +0x26, and target attribute selector at
-- +0x27. Both effect values are decoded through the shared 166AD4 byte table
-- offline: +0x24 is consumed by 0x10660 and +0x25 by 0x55B78.
local spell_action_by_code = {
    { availability = 1, effect_table_index = 0, difficulty_modifier = 0,
      base_effect = 1, target_effect = 3, attribute_selector = 0 },
    { availability = 2, effect_table_index = 1, difficulty_modifier = 1,
      base_effect = 3, target_effect = 6, attribute_selector = 0 },
    { availability = 3, effect_table_index = 3, difficulty_modifier = 3,
      base_effect = 10, target_effect = 10, attribute_selector = 0 },
    { availability = -1, effect_table_index = 1, difficulty_modifier = 0,
      base_effect = 3, target_effect = 0, attribute_selector = 0 },
    { availability = 1, effect_table_index = 2, difficulty_modifier = 0,
      base_effect = 6, target_effect = 3, attribute_selector = -1 },
    { availability = -1, effect_table_index = 2, difficulty_modifier = 0,
      base_effect = 6, target_effect = 0, attribute_selector = -1 },
    { availability = -1, effect_table_index = 2, difficulty_modifier = 2,
      base_effect = 6, target_effect = 0, attribute_selector = 0 },
    { availability = 0, effect_table_index = 0, difficulty_modifier = 0,
      base_effect = 1, target_effect = 1, attribute_selector = -1 },
    { availability = 1, effect_table_index = 1, difficulty_modifier = 0,
      base_effect = 3, target_effect = 3, attribute_selector = -1 },
    { availability = 2, effect_table_index = 2, difficulty_modifier = 1,
      base_effect = 6, target_effect = 6, attribute_selector = -1 },
    { availability = -1, effect_table_index = 1, difficulty_modifier = 2,
      base_effect = 3, target_effect = 0, attribute_selector = 0 },
    { availability = -1, effect_table_index = 1, difficulty_modifier = 0,
      base_effect = 3, target_effect = 0, attribute_selector = 0 },
    { availability = -1, effect_table_index = 2, difficulty_modifier = 0,
      base_effect = 6, target_effect = 0, attribute_selector = -1 },
}

-- ROM 0x4946. Zero entries are silent at the 0x15830 action-effect handoff.
local action_effect_audio_event_by_code = {
    [1] = 0x32, [2] = 0x34, [3] = 0x34, [4] = 0x31,
    [5] = 0x37, [6] = 0x38, [7] = 0x00, [8] = 0x33,
    [9] = 0x35, [10] = 0x35, [11] = 0x39, [12] = 0x3a,
    [13] = 0x36, [14] = 0x00, [15] = 0x00,
}

-- The actor effect handlers at 0x490A use different render clocks for the
-- spell actions that occur in the encounter tables. 0x15830 initializes the
-- target record countdown to five; these entries describe only the handler
-- cadence and presentation profile that follow that source handoff.
local actor_effect_profile_by_code = {
    [1] = { id = "flame-dart", repeat_countdown = 1, phase_count = 4,
            palette_line = 1 },
    [2] = { id = "flame-bolt", repeat_countdown = 4, phase_count = 6,
            palette_line = 1 },
    [5] = { id = "sleep", repeat_countdown = 6, phase_count = 9,
            palette_line = 3 },
    [9] = { id = "mana-blast", repeat_countdown = 4, phase_count = 6,
            palette_line = 2 },
    [12] = { id = "heal-wounds", repeat_countdown = 4, phase_count = 16,
             palette_line = 3 },
}

local function text_keys(ids, prefix)
    local keys = {}
    for index, id in ipairs(ids) do
        keys[index] = prefix .. id
    end
    return keys
end

function Magic.spells()
    return spell_ids, nil, text_keys(spell_ids, "magic.spells.option.")
end

function Magic.spell_descriptions()
    return spell_ids, nil,
        text_keys(spell_ids, "magic.spells.description.")
end

function Magic.action_motion_distance_profile(code)
    if type(code) ~= "number" or code % 1 ~= 0 or
       code < 1 or code > #spell_action_by_code then
        return nil
    end
    local source_record = SourceSpellRecords[code - 1]
    return source_record and source_record.flag27 or nil
end

function Magic.spell_action_definition(code)
    if type(code) ~= "number" or code % 1 ~= 0 or
       code < 1 or code > #spell_action_by_code then
        return nil
    end
    local row = spell_action_by_code[code]
    local source_record = SourceSpellRecords[code - 1]
    if source_record == nil then
        return nil
    end
    local effect_profile = actor_effect_profile_by_code[code]
    return {
        spell_id = spell_ids[code],
        row_index = code - 1,
        availability = row.availability,
        effect_table_index = row.effect_table_index,
        difficulty_modifier = row.difficulty_modifier,
        base_effect = row.base_effect,
        target_effect = row.target_effect,
        attribute_selector = row.attribute_selector,
        action_modifier_selector = source_record.totem_category,
        source_record = source_record,
        audio_event = action_effect_audio_event_by_code[code] or 0,
        actor_effect = effect_profile,
    }
end

function Magic.actor_effect_profile(code)
    if type(code) ~= "number" or code % 1 ~= 0 then
        return nil
    end
    local profile = actor_effect_profile_by_code[code]
    if profile == nil then
        return nil
    end
    return {
        id = profile.id,
        initial_countdown = 5,
        repeat_countdown = profile.repeat_countdown,
        phase_count = profile.phase_count,
        palette_line = profile.palette_line,
    }
end

-- Shared Magic panel text. Runner archetype determines which native panel is
-- shown; both variants resolve their copy from this source-backed catalog.
function Magic.panel_text_entries()
    local ids = {
        "defense_title",
        "force_posture_heading",
        "success_label",
        "force_label",
        "availability_label",
        "panel_metric_labels",
        "panel_force_posture_labels",
    }
    return ids, nil, text_keys(ids, "magic.panel.text.option.")
end

return Magic
