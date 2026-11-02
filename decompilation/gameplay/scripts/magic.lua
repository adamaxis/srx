-- Stable spell IDs follow the ROM D4EA0 name/description table order.
-- The native spell table filters this catalog for the active runner.
local Magic = {}
local SpellDefinitions = require("data.spells").by_id
local E = require("enums")

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
    "barrier",
}

local focus_item_by_spell = {
    flame_dart = "flame_dart_focus",
    flame_bolt = "flame_bolt_focus",
    hellblast = "hellblast_focus",
    invisibility = "invisibility_focus",
    sleep = "sleep_focus",
    stink = "stink_focus",
    super_barrier = "supr_barrier_focus",
    mana_zap = "mana_zap_focus",
    mana_blast = "mana_blast_focus",
    mana_storm = "mana_storm_focus",
    rockskin = "rockskin_focus",
    heal_wounds = "heal_wounds_focus",
    confusion = "confusion_focus",
    barrier = "barrier_focus",
}

-- Gameplay-facing spell rules use resolved effects and named attributes.
-- Source action-profile attribute selection is decoded before Lua: 0x557C4
-- maps a negative D4EA0 +0x27 value to +0x7C (Willpower), otherwise +0x77
-- (Body). The target-resistance choices below remain [H] until the spell's
-- delayed target-effect callback is traced end to end.
local spell_action_by_id = {
    flame_dart = { difficulty_modifier = 0, base_effect = 1,
        target_effect = 3, target_resistance_attribute = E.ActorAttribute.BODY },
    flame_bolt = { difficulty_modifier = 1, base_effect = 3,
        target_effect = 6, target_resistance_attribute = E.ActorAttribute.BODY },
    hellblast = { difficulty_modifier = 3, base_effect = 10,
        target_effect = 10, target_resistance_attribute = E.ActorAttribute.BODY },
    invisibility = { difficulty_modifier = 0, base_effect = 3,
        target_effect = 0, target_resistance_attribute = E.ActorAttribute.BODY },
    sleep = { difficulty_modifier = 0, base_effect = 6,
        target_effect = 3, target_resistance_attribute = E.ActorAttribute.WILLPOWER },
    stink = { difficulty_modifier = 0, base_effect = 6,
        target_effect = 0, target_resistance_attribute = E.ActorAttribute.WILLPOWER },
    super_barrier = { difficulty_modifier = 2, base_effect = 6,
        target_effect = 0, target_resistance_attribute = E.ActorAttribute.BODY },
    mana_zap = { difficulty_modifier = 0, base_effect = 1,
        target_effect = 1, target_resistance_attribute = E.ActorAttribute.WILLPOWER },
    mana_blast = { difficulty_modifier = 0, base_effect = 3,
        target_effect = 3, target_resistance_attribute = E.ActorAttribute.WILLPOWER },
    mana_storm = { difficulty_modifier = 1, base_effect = 6,
        target_effect = 6, target_resistance_attribute = E.ActorAttribute.WILLPOWER },
    rockskin = { difficulty_modifier = 2, base_effect = 3,
        target_effect = 0, target_resistance_attribute = E.ActorAttribute.BODY },
    heal_wounds = { difficulty_modifier = 0, base_effect = 3,
        target_effect = 0, target_resistance_attribute = E.ActorAttribute.BODY },
    confusion = { difficulty_modifier = 0, base_effect = 6,
        target_effect = 0, target_resistance_attribute = E.ActorAttribute.WILLPOWER },
    barrier = { difficulty_modifier = 2, base_effect = 3,
        target_effect = 0, target_resistance_attribute = E.ActorAttribute.BODY },
}

-- ROM 0x4946. Zero entries are silent at the 0x15830 action-effect handoff.
local action_effect_audio_event_by_id = {
    flame_dart = E.SoundId.SPELL_EFFECT_32,
    flame_bolt = E.SoundId.SPELL_EFFECT_34,
    hellblast = E.SoundId.SPELL_EFFECT_34,
    invisibility = E.SoundId.SPELL_EFFECT_31,
    sleep = E.SoundId.SPELL_EFFECT_37,
    stink = E.SoundId.SPELL_EFFECT_38,
    mana_zap = E.SoundId.SPELL_EFFECT_33,
    mana_blast = E.SoundId.SPELL_EFFECT_35,
    mana_storm = E.SoundId.SPELL_EFFECT_35,
    rockskin = E.SoundId.SPELL_EFFECT_39,
    heal_wounds = E.SoundId.SPELL_EFFECT_3A,
    confusion = E.SoundId.SPELL_EFFECT_36,
}

-- The actor effect handlers at 0x490A use different render clocks for the
-- spell actions that occur in the encounter tables. 0x15830 initializes the
-- target record countdown to five; these entries describe only the handler
-- cadence and presentation profile that follow that source handoff.
local actor_effect_profile_by_id = {
    flame_dart = { id = "flame-dart", action_code = E.SpellActionCode.FLAME_DART,
            repeat_countdown = 1, phase_count = 4, palette_line = 1 },
    flame_bolt = { id = "flame-bolt", action_code = E.SpellActionCode.FLAME_BOLT,
            repeat_countdown = 4, phase_count = 6, palette_line = 1 },
    sleep = { id = "sleep", action_code = E.SpellActionCode.SLEEP,
            repeat_countdown = 6, phase_count = 9, palette_line = 3 },
    mana_zap = { id = "mana-zap", action_code = E.SpellActionCode.MANA_ZAP,
            repeat_countdown = 1, phase_count = 3, palette_line = 1 },
    mana_blast = { id = "mana-blast", action_code = E.SpellActionCode.MANA_BLAST,
            repeat_countdown = 4, phase_count = 6, palette_line = 2 },
    heal_wounds = { id = "heal-wounds",
            action_code = E.SpellActionCode.HEAL_WOUNDS,
            repeat_countdown = 4, phase_count = 16, palette_line = 3 },
}

-- 0x15830 branches to 0x49A2 for these two action codes before it reads
-- +0xA2. All other action codes return when that link is zero [C].
local untargeted_effect_kind_by_id = {
    super_barrier = E.SpellEffectKind.FIELD_BARRIER,
    barrier = E.SpellEffectKind.FIELD_BARRIER,
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

function Magic.target_motion_range(spell_id)
    if type(spell_id) ~= "string" then
        return nil
    end
    local definition = SpellDefinitions[spell_id]
    return definition and definition.target_motion_range or nil
end

function Magic.spell_action_definition(spell_id)
    if type(spell_id) ~= "string" then
        return nil
    end
    local profile = spell_action_by_id[spell_id]
    local definition = SpellDefinitions[spell_id]
    if profile == nil or definition == nil then
        return nil
    end
    local effect_profile = actor_effect_profile_by_id[spell_id]
    return {
        spell_id = spell_id,
        focus_item_id = focus_item_by_spell[spell_id],
        difficulty_modifier = profile.difficulty_modifier,
        base_effect = profile.base_effect,
        target_effect = profile.target_effect,
        target_resistance_attribute = profile.target_resistance_attribute,
        audio_event = action_effect_audio_event_by_id[spell_id],
        untargeted_effect_kind = untargeted_effect_kind_by_id[spell_id] or
            E.SpellEffectKind.NONE,
        actor_effect = effect_profile,
    }
end

function Magic.untargeted_effect_kind(spell_id)
    if type(spell_id) ~= "string" then
        return nil
    end
    return untargeted_effect_kind_by_id[spell_id] or E.SpellEffectKind.NONE
end

function Magic.actor_effect_profile(spell_id)
    if type(spell_id) ~= "string" then
        return nil
    end
    local profile = actor_effect_profile_by_id[spell_id]
    if profile == nil then
        return nil
    end
    return {
        id = profile.id,
        action_code = profile.action_code,
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
