-- Source-derived combat policy for close actor reactions. The host supplies
-- typed actor records and bounded ROM random draws; this module owns the
-- 0x55C76 trial sequence and returns the resulting meter snapshot.
local Combat = {}
local E = require("enums")
local SpellEffectKind = E.SpellEffectKind
local SpellTuning = E.SpellTuning
local SoundId = E.SoundId
local Item = {
    COMBAT_SENSE_SPELL_LOCK = "combat_sense_spell_lock",
    POWER_FOCUS = "power_focus",
    PROTECTION_TALISMAN = "protection_talisman",
    SMART_GOGGLES = "smart_goggles",
    FETISH = "fetish",
    RAT_TOTEM = "rat_totem",
    GATOR_TOTEM = "gator_totem",
    BEAR_TOTEM = "bear_totem",
}

-- ROM 0x490A spell-callback table: the callbacks for action codes 1 (0x521E),
-- 2 (0x5152), 8 (0x508A) and 9 (0x4FE2) call 0xB0AC / 0xB0E4 instead of
-- 0xAFF2 when the spell changed the target's +0x4C to zero. 0xB0AC selects
-- pose 0xE0, 0xB0E4 pose 0xE4. Code 2's caller (0x51C8) compares live
-- +0x0E to 0x000F and skips 0xB0AC [C].
-- [H: codes are E.SpellType + 1 (code 12 = heal), i.e. flame dart/bolt and
-- mana zap/blast; the names are not traced.]
local SpellDeath = {
    flame_dart = { animation_variant = "spell_death_e0" },
    flame_bolt = {
        animation_variant = "spell_death_e0",
        excluded_actor_fact = "blocks_spell_death_pose",
    },
    mana_zap = { animation_variant = "spell_death_e4" },
    mana_blast = { animation_variant = "spell_death_e4" },
}

local function signed_byte(value)
    value = value & 0xff
    return value >= 0x80 and value - 0x100 or value
end

local function signed_word(value)
    value = value & 0xffff
    return value >= 0x8000 and value - 0x10000 or value
end

local function add_source_byte(value, delta)
    return (value + delta) & 0xff
end

local function subtract_source_byte(value, amount)
    return (value & 0xff00) | ((value - amount) & 0xff)
end

local function add_source_byte_to_word(value, amount)
    return (value & 0xff00) | ((value + amount) & 0xff)
end

local function inventory_items(record)
    return type(record.inventory) == "table" and record.inventory or {}
end

local function combat_cyberware(record)
    return type(record.cyberware) == "table" and record.cyberware or {}
end

local function inventory_item_at(record, slot)
    for _, item in ipairs(inventory_items(record)) do
        if item.slot == slot then
            return item
        end
    end
    return nil
end

local function find_inventory_item(record, item_id, reverse, maximum_slot)
    local inventory = inventory_items(record)
    if reverse then
        for index = #inventory, 1, -1 do
            local item = inventory[index]
            if item.id == item_id and
               (maximum_slot == nil or item.slot <= maximum_slot) then
                return item
            end
        end
    else
        for _, item in ipairs(inventory) do
            if item.id == item_id and
               (maximum_slot == nil or item.slot <= maximum_slot) then
                return item
            end
        end
    end
    return nil
end

local function command_average(record)
    local total = (record.attributes.quickness +
                   record.attributes.intelligence +
                   record.attributes.willpower) & 0xff
    local value = total >> 1
    local spell_lock = find_inventory_item(
        record, Item.COMBAT_SENSE_SPELL_LOCK, false)
    if spell_lock ~= nil then
        return (value + (spell_lock.command_bonus or 0)) & 0xff
    end
    return value
end

-- +0x4D arrives as a host-derived percentage; ROM 0x55F20 applies 25
-- percent per posture step with integer division [C].
local function posture_reduction(value, posture_reduction_percent)
    return math.floor((value * posture_reduction_percent) / 100)
end

local function item18_skill_value(record)
    local value = record.skills.sorcery & 0xff
    -- 0x55F02 scans positions 1..7 from the last slot down; slot 8 belongs
    -- to the separate special-item path at 0x104F6.
    local focus = find_inventory_item(
        record, Item.POWER_FOCUS, true, 7)
    if focus ~= nil then
        return add_source_byte_to_word(value, focus.sorcery_bonus or 0)
    end
    return value
end

local function action_phase_modifier(value, record, spell_id)
    local magic = game.modules.magic
    local items = game.modules.items
    local definition = type(magic) == "table" and
        type(magic.spell_action_definition) == "function" and
        magic.spell_action_definition(spell_id) or nil
    if type(definition) ~= "table" or
       type(definition.spell_id) ~= "string" then
        return nil
    end
    local focus = find_inventory_item(
        record, definition.focus_item_id, true, 7)
    if focus ~= nil then
        if type(focus.spell_action_bonus) ~= "number" then
            return nil
        end
        value = add_source_byte_to_word(value, focus.spell_action_bonus)
    end

    local totem = inventory_item_at(record, 8)
    if totem == nil or
       (totem.id ~= Item.RAT_TOTEM and totem.id ~= Item.GATOR_TOTEM and
        totem.id ~= Item.BEAR_TOTEM) then
        return value
    end
    local item = type(items) == "table" and
        type(items.by_id) == "table" and items.by_id[totem.id] or nil
    local modifiers = type(item) == "table" and
        item.spell_action_modifiers or nil
    local modifier = type(modifiers) == "table" and
        modifiers[definition.spell_id] or nil
    -- ROM 0x104F6 skips the totem word when C8AE0 supplies a negative
    -- selector; the generated semantic map omits those spell entries.
    if modifier == nil then
        return value
    end
    if type(modifier) ~= "number" then
        return nil
    end
    value = (value + modifier) & 0xffff
    if signed_word(value) <= 0 then
        value = 1
    end
    return value
end

local function spell_residual(force, roll_count, spell)
    -- ROM 0x10660 starts with caster +0x9D (force), rounds it upward when
    -- halved, adds D4EA0 +0x26, then applies its integer percentage and
    -- half-value sequence. This is the caster's fetish/behavior-meter path;
    -- it is distinct from the delayed target roll at 0x55B78/0x559BC.
    local effective = math.max(
        math.floor((force + 1) / 2) + spell.difficulty_modifier, 2)
    local percent_steps = effective >= 7 and 10 or (7 - effective) * 20
    local product = roll_count * percent_steps
    local adjusted = math.floor(product / 100)
    if (product % 100) >= 50 then
        adjusted = adjusted + 1
    end
    adjusted = math.floor(adjusted / 2)
    return spell.base_effect - adjusted
end

local function thresholded_rolls(count, difficulty, random_bound)
    if type(random_bound) ~= "function" then
        return nil
    end
    local successes = 0
    local trials = count == 0 and 1 or count
    for _ = 1, trials do
        local accumulated = 0
        while true do
            local roll = random_bound(6)
            if type(roll) ~= "number" or roll % 1 ~= 0 or
               roll < 0 or roll >= 6 then
                return nil
            end
            local face = roll + 1
            if face == 6 then
                accumulated = accumulated + face
            else
                if accumulated + face >= difficulty then
                    successes = successes + 1
                end
                break
            end
        end
    end
    return successes
end

local function condition_modifier(difficulty, attacker, target)
    local has_cyberware = combat_cyberware(attacker).cybereyes == true
    if has_cyberware then
        difficulty = difficulty - 1
    end
    local attacker_traits = attacker.combat_traits
    local target_traits = target.combat_traits
    if target_traits.adds_two_condition_difficulty then
        return difficulty + 2
    end
    if has_cyberware or
       attacker_traits.ignores_cover_condition_penalty or
       not target.actor_facts.interaction.no_coverage_difficulty_bonus then
        return difficulty
    end
    return difficulty + 4
end

local function unlinked_spell_condition_modifier(difficulty, attacker)
    -- 0x55740 reads target +0x9B and +0x11 through A1 [C]. On the player's
    -- +0xA2==0 path, A1 is left by the outer caller or a previous actor
    -- callback (see 0xA8EA/0xAAA6), so those bytes are not a stable defender
    -- profile. Keep the attacker-side cyberware reduction and do not invent
    -- a self target [H].
    if combat_cyberware(attacker).cybereyes == true then
        return difficulty - 1
    end
    return difficulty
end

local function add_wound_difficulty_penalty(difficulty, physical_meter,
                                            mental_meter)
    -- ROM 0xD5A adds two below five, one from five through seven, and zero
    -- from eight upward for each meter before 0x0DBA rolls.
    local function add_meter_penalty(meter)
        if meter < 8 then
            difficulty = difficulty + (meter < 5 and 2 or 1)
        end
    end
    add_meter_penalty(physical_meter)
    add_meter_penalty(mental_meter)
    return difficulty
end

local function ready_party_bonus(context)
    local bonus = 0
    for _, candidate in ipairs(context.ai_target_candidates or {}) do
        local record = candidate.combatant
        if not candidate.world_activity_actor and type(record) == "table" and
           record.actor_facts.lifecycle.active and
           record.combat_traits.qualifies_for_party_support_bonus then
            local item18 = item18_skill_value(record)
            bonus = bonus + math.floor(item18 / 4)
            if (item18 % 4) >= 3 then
                bonus = bonus + 1
            end
        end
    end
    return bonus
end

local function dynamic_status_modifier(difficulty, attacker, linked, context)
    -- ROM 0x55786/0x557EE/0x55804 selects the two action-profile paths;
    -- the host has already translated +0xDF into these named choices.
    local options = type(attacker.action_profile_options) == "table" and
        attacker.action_profile_options or {}
    local primary = options.primary == true
    local secondary_requested = options.secondary == true
    local secondary = not primary and secondary_requested
    if not primary and not secondary then
        return difficulty
    end

    local profiles = type(linked) == "table" and
        linked.dynamic_action_profile or nil
    local profile = type(profiles) == "table" and
        profiles[primary and "primary" or "secondary"] or nil
    if type(profile) ~= "table" then
        return nil
    end

    local roll_count = profile.roll_count
    local roll_difficulty = profile.difficulty
    local attribute_name = profile.attribute
    if attribute_name ~= E.ActorAttribute.BODY and
       attribute_name ~= E.ActorAttribute.WILLPOWER then
        return nil
    end
    local attribute = attacker.attributes[attribute_name]
    if attacker.is_party_member then
        attribute = add_source_byte_to_word(
            ready_party_bonus(context), attribute)
    end
    local successes = thresholded_rolls(attribute, roll_difficulty,
                                        context.ai_random_bound)
    if successes == nil then
        return nil
    end
    local remaining = roll_count - successes
    if remaining <= 0 then
        return difficulty
    end
    if secondary_requested then
        remaining = math.floor(remaining / 2)
    end
    return difficulty + remaining
end

local function secondary_armor_modifier(target)
    local items = game.modules.items
    for _, item in ipairs(type(items) == "table" and items.catalog or {}) do
        local armor = item.armor
        if type(armor) == "table" and
           item.id == target.equipped_armor_id then
            return armor.secondary_modifier
        end
    end
    return 0
end

local function defense_modifier(value, attacker)
    if attacker.actor_facts.interaction.defense_difficulty_reduction then
        return value - 2
    end
    local talisman = find_inventory_item(
        attacker, Item.PROTECTION_TALISMAN, false)
    if talisman ~= nil then
        return value - math.floor((talisman.rating or 0) / 2)
    end
    return value
end

local function minimum_plus_attribute(value, attribute, add_attribute)
    local result = math.min(value, attribute)
    if add_attribute then
        result = result + attribute
    end
    return result == 0 and 1 or result
end

local function adjusted_meters(record, physical, damage, context)
    local physical_meter = record.physical_meter
    local mental_meter = record.mental_meter
    if record.is_party_member then
        if context.combat_meter_service_blocked then
            return physical_meter, mental_meter
        end
        local function advance_delay(delta)
            if not record.actor_facts.lifecycle.active then
                physical_meter = 0
                return
            end
            local updated = add_source_byte(physical_meter, delta)
            if signed_byte(updated) > 0 then
                physical_meter = math.min(updated, 10)
            else
                physical_meter = 0
            end
        end
        if physical then
            advance_delay(-damage)
        else
            if not record.actor_facts.lifecycle.active then
                mental_meter = 0
            else
                local updated = add_source_byte(mental_meter, -damage)
                if signed_byte(updated) > 0 then
                    mental_meter = math.min(updated, 10)
                else
                    advance_delay(signed_byte(updated))
                    mental_meter = 0
                end
            end
        end
        return physical_meter, mental_meter
    end

    local delta = record.combat_traits.takes_fixed_meter_damage and
        -1 or -damage
    if physical then
        local updated = add_source_byte(physical_meter, delta)
        physical_meter = signed_byte(updated) > 0 and updated or 0
    else
        local updated = add_source_byte(mental_meter, delta)
        mental_meter = signed_byte(updated) > 0 and updated or 0
    end
    return physical_meter, mental_meter
end

local function copy_combatant(record)
    local copy = {}
    for key, value in pairs(record) do
        if key == "actor_facts" and type(value) == "table" then
            local facts = {}
            for group_name, group in pairs(value) do
                if type(group) == "table" then
                    facts[group_name] = {}
                    for name, fact in pairs(group) do
                        facts[group_name][name] = fact == true
                    end
                end
            end
            copy[key] = facts
        elseif key == "attributes" or key == "skills" or
               key == "cyberware" or key == "combat_traits" then
            local values = {}
            for name, number in pairs(value) do
                values[name] = number
            end
            copy[key] = values
        elseif key == "weapon_action_policies" then
            local policies = {}
            for action_id, policy in pairs(value) do
                local policy_copy = {}
                for name, item in pairs(policy) do
                    policy_copy[name] = item
                end
                policies[action_id] = policy_copy
            end
            copy[key] = policies
        elseif key == "action_profile_options" then
            copy[key] = {
                primary = value.primary == true,
                secondary = value.secondary == true,
            }
        elseif key == "dynamic_action_profile" then
            local profiles = {}
            for profile_name, profile in pairs(value) do
                local profile_copy = {}
                for name, item in pairs(profile) do
                    profile_copy[name] = item
                end
                profiles[profile_name] = profile_copy
            end
            copy[key] = profiles
        elseif key == "inventory" then
            local inventory = {}
            for index, item in ipairs(value) do
                local item_copy = {}
                for name, state in pairs(item) do
                    item_copy[name] = state
                end
                inventory[index] = item_copy
            end
            copy[key] = inventory
        elseif type(value) == "table" then
            local array = {}
            for index, item in ipairs(value) do
                array[index] = item
            end
            copy[key] = array
        else
            copy[key] = value
        end
    end
    return copy
end

local function publish_player_hurt_event(target, defender, physical_meter,
                                         mental_meter)
    if defender.is_party_member ~= true or
       type(target) ~= "table" or type(target.id) ~= "string" or
       target.id == "" then
        return
    end
    local pending = game.player_hurt_pending_by_actor_id
    if type(pending) ~= "table" then
        pending = {}
        game.player_hurt_pending_by_actor_id = pending
    end
    if physical_meter == 0 then
        -- 0xAFF2 takes the zero-meter record through the C0B2 down path.
        pending[target.id] = nil
    elseif physical_meter < defender.physical_meter or
           mental_meter < defender.mental_meter then
        -- Party records are the player side of this event. World actors keep
        -- using their existing incoming_hit event below. The host record is
        -- resolved from this actor ID when it applies the combat result.
        pending[target.id] = true
    end
end

local function lifecycle_equal(left, right)
    for _, name in ipairs({"active", "thinking_enabled", "ai_runner",
                           "downed", "group_leader", "dead"}) do
        if left[name] ~= right[name] then
            return false
        end
    end
    return true
end

local function commit_combatant_meters(target, defender, physical_meter,
                                      mental_meter, lifecycle, spell_death)
    if type(lifecycle) == "table" then
        defender.actor_facts.lifecycle = lifecycle
    end
    local changed = physical_meter ~= defender.physical_meter or
                    mental_meter ~= defender.mental_meter or
                    not lifecycle_equal(defender.actor_facts.lifecycle,
                                        target.combatant.actor_facts.lifecycle)
    if not changed then
        return false
    end
    local state = game.actors[target.id]
    if type(state) ~= "table" then
        state = { actor_id = target.id }
        game.actors[target.id] = state
    end
    state.combatant = copy_combatant(defender)
    state.combatant.physical_meter = physical_meter
    state.combatant.mental_meter = mental_meter
    if not defender.is_party_member then
        -- ROM 0xAFF2 runs on the damaged actor after either 0x55C76 or
        -- weapon hit resolution changes a live world actor's meters.
        -- Preserve the event in that actor's Lua state for its next heartbeat.
        state.incoming_hit = {
            physical_meter = physical_meter,
            mental_meter = mental_meter,
            spell_death_variant = physical_meter == 0 and
                spell_death ~= nil and
                (spell_death.excluded_actor_fact == nil or
                 defender[spell_death.excluded_actor_fact] ~= true) and
                spell_death.animation_variant or nil,
        }
    end
    publish_player_hurt_event(
        target, defender, physical_meter, mental_meter)
    return true
end

local function consume_fetish_residual(record, residual)
    local remaining = residual
    local changed = false
    -- 0x10660 consumes fetish uses from the last occupied inventory slot
    -- toward the first. Other item quantities and firearm rounds are intact.
    for index = #inventory_items(record), 1, -1 do
        local item = record.inventory[index]
        if item.id == Item.FETISH then
            local uses = item.uses_remaining or 0
            changed = true
            if remaining <= uses then
                item.uses_remaining = uses - remaining
                if item.uses_remaining == 0 then
                    table.remove(record.inventory, index)
                end
                return "fetish_consumed", changed
            end
            remaining = remaining - uses
            table.remove(record.inventory, index)
        end
    end
    return "fetish_exhausted", changed
end

local function resolve_spell_action(context, target, allow_untargeted)
    local untargeted = target == nil
    if type(context) ~= "table" or
       (untargeted and not allow_untargeted) or
       type(context.combatant) ~= "table" or
       (not untargeted and
        (type(target) ~= "table" or
         type(target.combatant) ~= "table" or
         type(target.id) ~= "string" or target.id == "")) or
       (untargeted and
        (type(context.actor_id) ~= "string" or context.actor_id == "")) or
       type(context.ai_random_bound) ~= "function" or
       type(context.action_spell_id) ~= "string" then
        return nil
    end
    local magic = game.modules.magic
    local spell = type(magic) == "table" and
        type(magic.spell_action_definition) == "function" and
        magic.spell_action_definition(context.action_spell_id) or nil
    if type(spell) ~= "table" then
        return nil
    end

    local attacker = copy_combatant(context.combatant)
    local defender = not untargeted and copy_combatant(target.combatant) or nil
    local result = {
        spell_id = context.action_spell_id,
        target_id = not untargeted and target.id or "",
        untargeted = untargeted,
        field_effect_kind = SpellEffectKind.NONE,
        action_result = 0,
        roll_attempted = false,
        outcome = "roll_failed",
        effect_requested = false,
        effect_started = false,
        behavior_meter_delta = 0,
        residual_effect = 0,
        inventory_changed = false,
        special_result_published = false,
        special_result_successes = 0,
        special_result_bonus = 0,
        combatant = attacker,
    }

    -- ROM 0x1033E skips this action when +0x9C is one. For type-three and
    -- later records it resets the active action header to the first carried
    -- item; the field host applies that header after the heartbeat.
    if attacker.mental_meter == 1 then
        if not attacker.is_party_member then
            local first_item = inventory_item_at(attacker, 1)
            result.next_action_kind = first_item == nil and "none" or "item"
            result.next_action_item_id = first_item and first_item.id or nil
            if type(context.profile_prefix) == "string" then
                result.next_profile_prefix = context.profile_prefix
            end
            attacker.equipped_action_id = first_item and first_item.id or nil
        end
        result.outcome = "target_state_gate"
        return result
    end

    local skill = item18_skill_value(attacker)
    local initial_trials = posture_reduction(
        skill, attacker.posture_reduction_percent)
    initial_trials = math.min(
        initial_trials, attacker.spell_trial_cap or 0)
    initial_trials = add_source_byte_to_word(initial_trials, attacker.force or 0)
    initial_trials = action_phase_modifier(
        initial_trials, attacker, context.action_spell_id)
    if initial_trials == nil then
        result.outcome = "source_data_unavailable"
        return result
    end
    initial_trials = initial_trials & 0xff
    result.initial_trial_count = initial_trials

    local difficulty = untargeted
        and unlinked_spell_condition_modifier(4, attacker)
        or condition_modifier(4, attacker, defender)
    difficulty = dynamic_status_modifier(
        difficulty, attacker, defender, context)
    if difficulty == nil then
        result.outcome = "profile_data_unavailable"
        return result
    end
    difficulty = add_wound_difficulty_penalty(
        difficulty, attacker.physical_meter, attacker.mental_meter)
    difficulty = math.max(difficulty, 2)
    result.difficulty = difficulty

    result.roll_attempted = true
    local successes = thresholded_rolls(
        initial_trials, difficulty, context.ai_random_bound)
    if successes == nil then
        result.outcome = "random_service_unavailable"
        return result
    end
    attacker.action_result = successes
    result.action_result = successes
    result.combatant = attacker
    if successes == 0 then
        return result
    end

    -- 0x103D8 dispatches the effect before resolving the caster residual [C].
    result.outcome = "effect_dispatched"
    result.effect_requested = true
    if untargeted then
        -- ROM 0x15830 returns at 0x1584C for a zero +0xA2 link, except action
        -- codes 7 and 14, which enter 0x49A2 and create the directional
        -- five-cell field barrier [C]. This is not a self-targeted actor effect.
        result.field_effect_kind = spell.untargeted_effect_kind or
            SpellEffectKind.NONE
        if result.field_effect_kind == SpellEffectKind.FIELD_BARRIER then
            for slot = 0, 17 do
                if context.world_effect_slot_occupancy == nil or
                   context.world_effect_slot_occupancy[slot + 1] ~= true then
                    result.spell_effect_slot = slot
                    result.effect_started = true
                    result.audio_event = SoundId.FIELD_3B
                    break
                end
            end
        end
    else
        -- ROM 0x1582A reserves a target effect only when a target record is
        -- linked. A full pool still preserves the roll and caster drain.
        for slot = 0, 17 do
            if context.world_effect_slot_occupancy == nil or
               context.world_effect_slot_occupancy[slot + 1] ~= true then
                result.spell_effect_slot = slot
                result.effect_started = true
                break
            end
        end
        if result.effect_started and spell.audio_event ~= nil then
            result.audio_event = spell.audio_event
        end
    end
    if not untargeted and
       (spell.spell_id == "stink" or spell.spell_id == "confusion") then
        result.special_result_published = true
        result.special_result_successes = successes
        result.special_result_bonus = attacker.force or 0
    end

    local remaining_skill = skill - posture_reduction(
        skill, attacker.posture_reduction_percent)
    local effect_trials = (remaining_skill +
        attacker.attributes.willpower) & 0xff
    local residual = spell_residual(
        attacker.force or 0,
        effect_trials, spell)
    result.residual_effect = residual
    if residual <= 0 then
        if attacker.mental_meter == 0 then
            result.behavior_meter_delta = 1
            if attacker.actor_facts.lifecycle.active then
                attacker.mental_meter = math.min(
                    attacker.mental_meter + result.behavior_meter_delta, 10)
            else
                attacker.mental_meter = 0
            end
        else
            result.outcome = "effect_residual_empty"
        end
        return result
    end

    local consumption, inventory_changed = consume_fetish_residual(
        attacker, residual)
    result.outcome = consumption
    result.inventory_changed = inventory_changed
    if consumption == "fetish_exhausted" and attacker.mental_meter == 0 then
        result.behavior_meter_delta = 1
        if attacker.actor_facts.lifecycle.active then
            attacker.mental_meter = math.min(
                attacker.mental_meter + result.behavior_meter_delta, 10)
        else
            attacker.mental_meter = 0
        end
    end
    result.combatant = attacker
    return result
end

function Combat.resolve_spell_action(context, target)
    return resolve_spell_action(context, target, false)
end

function Combat.resolve_untargeted_spell_action(context)
    return resolve_spell_action(context, nil, true)
end

function Combat.resolve_spell_impact(context, pending, target)
    if type(context) ~= "table" or type(pending) ~= "table" or
       type(target) ~= "table" or type(context.combatant) ~= "table" or
       type(target.combatant) ~= "table" or
       type(context.ai_random_bound) ~= "function" or
       type(pending.spell_id) ~= "string" or
       type(pending.action_result) ~= "number" or
       type(pending.target_id) ~= "string" or
       type(target.id) ~= "string" or
       target.id == "" or
       target.id ~= pending.target_id then
        return nil
    end

    local magic = game.modules.magic
    local spell = type(magic) == "table" and
        type(magic.spell_action_definition) == "function" and
        magic.spell_action_definition(pending.spell_id) or nil
    if type(spell) ~= "table" then
        return nil
    end

    local caster = copy_combatant(context.combatant)
    local defender = copy_combatant(target.combatant)
    local result = {
        spell_id = pending.spell_id,
        target_id = target.id,
        target_trial_count = 0,
        difficulty = 2,
        target_roll_successes = 0,
        effect_value = 0,
        outcome = "target_roll_failed",
        physical = spell.spell_id ~= "sleep",
        physical_meter = defender.physical_meter,
        mental_meter = defender.mental_meter,
        actor_facts = copy_combatant(defender).actor_facts,
        actor_lifecycle_changed = false,
        changed = false,
    }

    if spell.spell_id == "heal_wounds" then
        -- ROM 0x56DE ends by calling 0x55C2C. That callback heals directly
        -- from the caster's stored +0x9F result; unlike the offensive spell
        -- callback, it does not perform a second target roll.
        local heal_amount = signed_byte(pending.action_result)
        if heal_amount <= 0 then
            heal_amount = 1
        end
        local next_lifecycle = copy_combatant(defender).actor_facts.lifecycle
        next_lifecycle.downed = false
        next_lifecycle.dead = false
        next_lifecycle.active = true
        local physical_meter =
            defender.actor_facts.lifecycle.active
            and math.min(defender.physical_meter + heal_amount, 10) or 0
        local mental_meter = defender.mental_meter
        if mental_meter == 0 and next_lifecycle.active then
            mental_meter = math.min(mental_meter + 1, 10)
        end
        result.physical_meter = physical_meter
        result.mental_meter = mental_meter
        result.actor_facts.lifecycle = next_lifecycle
        result.actor_lifecycle_changed = not lifecycle_equal(
            next_lifecycle, defender.actor_facts.lifecycle)
        result.changed = commit_combatant_meters(
            target, defender, physical_meter, mental_meter,
            next_lifecycle)
        result.effect_value = result.changed and
            math.min(heal_amount, 10) or 0
        result.outcome = result.changed and
            "effect_applied" or "effect_no_change"
        return result
    end

    -- 0x55B78 has a special early branch for the first three action codes
    -- against +0x9B == 0x17. It supplies D4/D6 but inherits D5 from its
    -- caller; keep that path explicit until the effect callback's D5 input
    -- is traced rather than assigning it an invented spell strength.
    if (spell.spell_id == "flame_dart" or
        spell.spell_id == "flame_bolt" or
        spell.spell_id == "hellblast") and
       defender.combat_traits.is_special_spell_target then
        result.outcome = "source_special_target_path"
        result.target_trial_count = 12
        result.difficulty = 2
        return result
    end

    local resistance_attribute = spell.target_resistance_attribute
    if resistance_attribute ~= E.ActorAttribute.BODY and
       resistance_attribute ~= E.ActorAttribute.WILLPOWER then
        result.outcome = "source_data_unresolved"
        return result
    end
    local attribute = defender.attributes[resistance_attribute]
    if defender.is_party_member then
        attribute = add_source_byte_to_word(
            attribute, ready_party_bonus(context))
    end
    result.target_trial_count = attribute & 0xff
    local force = caster.force or 0
    result.difficulty = math.max(force, 2)
    local target_successes = thresholded_rolls(
        result.target_trial_count, result.difficulty,
        context.ai_random_bound)
    if target_successes == nil then
        result.outcome = "random_service_unavailable"
        return result
    end
    result.target_roll_successes = target_successes

    -- ROM 0x559BC subtracts the target's +0x9F roll from the caster's stored
    -- action result and adjusts the D4EA0 effect value by that signed byte.
    local action_result = pending.action_result & 0xff
    local delta = signed_byte(action_result - target_successes)
    local effect_value = spell.target_effect or 0
    if delta < 0 then
        effect_value = signed_byte(add_source_byte(effect_value, delta))
        if effect_value <= 0 then
            result.outcome = "target_resisted"
            return result
        end
    else
        effect_value = effect_value + delta
        if effect_value >= 0x0b then
            effect_value = 0x0a
        end
    end

    -- 0xFFF112 is one here, so the target armor-table subtractor is skipped;
    -- 0x55E62 applies the caster's interaction/defensive equipment modifier.
    effect_value = defense_modifier(effect_value, caster)
    if effect_value <= 0 then
        result.outcome = "target_resisted"
        return result
    end

    local physical_meter, mental_meter = adjusted_meters(
        defender, result.physical, effect_value, context)
    result.effect_value = effect_value
    result.physical_meter = physical_meter
    result.mental_meter = mental_meter
    result.changed = commit_combatant_meters(
        target, defender, physical_meter, mental_meter, nil,
        SpellDeath[pending.spell_id])
    result.outcome = result.changed and "effect_applied" or "effect_no_change"
    return result
end

-- ROM 0x55B16: the default difficulty-four action roll grants a six-roll
-- bonus, then enters 0x559BC with difficulty six, target +0x77 trials, and
-- action value two. This is distinct from the peer reaction at 0x55C76.
function Combat.resolve_default_bonus_action(context, target)
    if type(context) ~= "table" or type(target) ~= "table" or
       type(context.combatant) ~= "table" or
       type(target.combatant) ~= "table" or
       type(context.ai_random_bound) ~= "function" then
        return nil
    end
    local attacker = copy_combatant(context.combatant)
    local defender = copy_combatant(target.combatant)
    local difficulty = condition_modifier(
        E.CombatTuning.DEFAULT_ACTION_DIFFICULTY, attacker, defender)
    difficulty = dynamic_status_modifier(difficulty, attacker, defender,
                                         context)
    if difficulty == nil then
        return nil
    end
    difficulty = add_wound_difficulty_penalty(
        difficulty, attacker.physical_meter, attacker.mental_meter)
    difficulty = math.max(difficulty, 2)

    local initial_successes = thresholded_rolls(
        E.CombatTuning.DEFAULT_ACTION_TRIAL_COUNT, difficulty,
        context.ai_random_bound)
    if initial_successes == nil then
        return nil
    end
    attacker.action_result = initial_successes
    attacker.force = E.CombatTuning.DEFAULT_ACTION_TRIAL_COUNT

    local target_trials = defender.attributes.body
    if type(target.id) ~= "string" or type(target_trials) ~= "number" then
        return nil
    end
    local target_successes = thresholded_rolls(
        target_trials, E.CombatTuning.DEFAULT_ACTION_CONTINUATION_DIFFICULTY,
        context.ai_random_bound)
    local result = {
        target_id = target.id,
        physical_meter = defender.physical_meter,
        mental_meter = defender.mental_meter,
        physical = true,
        initial_difficulty = difficulty,
        initial_trial_count = E.CombatTuning.DEFAULT_ACTION_TRIAL_COUNT,
        initial_successes = initial_successes,
        combatant = attacker,
        continuation_difficulty =
            E.CombatTuning.DEFAULT_ACTION_CONTINUATION_DIFFICULTY,
        continuation_trial_count = target_trials,
        changed = false,
    }
    if target_successes == nil then
        result.outcome = "random_service_unavailable"
        return result
    end
    result.continuation_successes = target_successes

    -- 0x559CE..0x559F0 compares the first result with the target roll using
    -- signed-byte arithmetic, then caps the action value at ten.
    local delta = signed_byte(initial_successes - target_successes)
    local action_value = E.CombatTuning.DEFAULT_ACTION_VALUE + delta
    if action_value > E.CombatTuning.ACTION_VALUE_MAX then
        action_value = E.CombatTuning.ACTION_VALUE_MAX
    end
    if action_value <= 0 then
        result.outcome = "action_value_nonpositive"
        return result
    end

    action_value = defense_modifier(action_value, attacker)
    if action_value <= 0 then
        result.outcome = "defense_threshold_failed"
        return result
    end

    -- FFF0CC's default mode selects the physical meter. The source's alternate
    -- meter cases remain owned by their caller-specific combat paths.
    local physical_meter, mental_meter = adjusted_meters(
        defender, true, action_value, context)
    result.physical_meter = physical_meter
    result.mental_meter = mental_meter
    result.changed = commit_combatant_meters(
        target, defender, physical_meter, mental_meter)
    result.outcome = result.changed and "effect_applied" or "effect_no_change"
    return result
end

function Combat.resolve_reaction(context, target)
    if type(context) ~= "table" or type(target) ~= "table" or
       type(context.combatant) ~= "table" or
       type(target.combatant) ~= "table" or
       type(context.ai_random_bound) ~= "function" then
        return nil
    end
    local attacker = copy_combatant(context.combatant)
    local defender = copy_combatant(target.combatant)

    local attacker_average = command_average(attacker)
    local attacker_reduction = posture_reduction(attacker_average,
        attacker.posture_reduction_percent)
    local first_count = math.min(attacker_reduction,
                                 attacker.skills.melee_combat) +
                        attacker.skills.melee_combat
    if first_count == 0 then
        first_count = 1
    end
    local difficulty = condition_modifier(4, attacker, defender)
    difficulty = dynamic_status_modifier(difficulty, attacker, defender,
                                         context)
    if difficulty == nil then
        return nil
    end
    difficulty = math.max(difficulty, 2)
    local first_successes = thresholded_rolls(first_count, difficulty,
                                               context.ai_random_bound)
    if first_successes == nil or first_successes == 0 then
        return nil
    end

    local remaining = first_successes + 3
    remaining = subtract_source_byte(
        remaining, secondary_armor_modifier(defender))
    if signed_byte(remaining) <= 0 then
        return nil
    end
    remaining = defense_modifier(remaining, attacker)
    if signed_byte(remaining) <= 0 then
        return nil
    end

    local defender_average = command_average(defender)
    local defender_reduction = posture_reduction(defender_average,
        defender.posture_reduction_percent)
    local second_count = minimum_plus_attribute(
        defender_average - defender_reduction,
        defender.skills.melee_combat,
        defender.equipped_action_id == nil)
    local second_difficulty = condition_modifier(4, defender, attacker)
    second_difficulty = dynamic_status_modifier(
        second_difficulty, defender, attacker, context)
    if second_difficulty == nil then
        return nil
    end
    second_difficulty = math.max(second_difficulty, 2)
    local second_successes = thresholded_rolls(
        second_count, second_difficulty, context.ai_random_bound)
    if second_successes == nil then
        return nil
    end
    remaining = subtract_source_byte(remaining, second_successes)
    if signed_byte(remaining) <= 0 then
        return nil
    end

    local final_difficulty = attacker.attributes.strength +
        (combat_cyberware(attacker).melee_strength_bonus or 0)
    local final_count = add_source_byte_to_word(
        second_count, defender.attributes.body)
    final_count = final_count +
        (combat_cyberware(defender).melee_defense_bonus or 0)
    final_difficulty = math.max(final_difficulty, 2)
    local final_successes = thresholded_rolls(
        final_count, final_difficulty, context.ai_random_bound)
    if final_successes == nil then
        return nil
    end
    remaining = subtract_source_byte(remaining, final_successes)
    if signed_byte(remaining) <= 0 then
        return nil
    end

    local direct_physical = combat_cyberware(attacker).spurs == true
    local physical_penalty =
        combat_cyberware(attacker).hand_razors == true
    if not direct_physical and physical_penalty then
        remaining = subtract_source_byte(remaining, 2)
        if signed_byte(remaining) <= 0 then
            return nil
        end
    end
    local physical = direct_physical or physical_penalty
    local damage = signed_byte(remaining)
    local physical_meter, mental_meter = adjusted_meters(
        defender, physical, damage, context)
    local changed = commit_combatant_meters(
        target, defender, physical_meter, mental_meter)
    return {
        target_id = target.id,
        physical_meter = physical_meter,
        mental_meter = mental_meter,
        physical = physical,
        changed = changed,
        exhausted = changed and not defender.is_party_member and
            ((physical and physical_meter == 0) or
             (not physical and mental_meter == 0)),
    }
end

local function weapon_action_skill_value(attacker, skill_group,
                                         posture_component)
    local skills = attacker.skills
    if type(skills) ~= "table" or
       type(skills.firearms) ~= "number" or
       type(skills[skill_group]) ~= "number" then
        return nil
    end
    -- Skill ratings are gameplay levels, not wrapped byte values. ROM 0xD920
    -- caps trained skills at 12; the untrained specialty branch at 0xD928..
    -- 0xD99A starts from Firearms + 1, so valid ratings remain at most 13.
    -- Express the rule directly: use the stronger applicable skill, then
    -- combine it with the smaller posture contribution.
    local selected = math.max(skills.firearms, skills[skill_group])
    local posture_contribution = math.min(selected, posture_component)
    return math.max(selected + posture_contribution, 1)
end

local function weapon_action_eligibility(difficulty, attacker, policy)
    if type(policy) ~= "table" or
       type(policy.difficulty_adjustment) ~= "number" or
       type(policy.benefits_from_targeting_assist) ~= "boolean" then
        return nil
    end
    difficulty = difficulty + policy.difficulty_adjustment
    if not policy.benefits_from_targeting_assist then
        return difficulty
    end
    if combat_cyberware(attacker).smartlink == true then
        return difficulty - 2
    end
    if find_inventory_item(attacker, Item.SMART_GOGGLES, true) ~= nil then
        return difficulty - 1
    end
    return difficulty
end

local function source_weapon_action(item_id)
    for _, item in ipairs(game.modules.items.catalog or {}) do
        if item.id == item_id and item.kind == "firearm" and
           type(item.weapon) == "table" and
           type(item.weapon.source_action) == "table" then
            return item.weapon.source_action
        end
    end
    return nil
end

function Combat.resolve_weapon_attack_seed(context)
    if type(context) ~= "table" or type(context.combatant) ~= "table" or
       type(context.ai_random_bound) ~= "function" or
       type(context.projectile_action_id) ~= "string" then
        return nil
    end
    local attacker = copy_combatant(context.combatant)
    local action_id = context.projectile_action_id
    local policies = attacker.weapon_action_policies
    local policy = type(policies) == "table" and policies[action_id] or nil
    if type(policy) ~= "table" or type(policy.skill_group) ~= "string" then
        return nil
    end
    local empty_child = {
        combat_traits = {
            ignores_cover_condition_penalty = false,
            adds_two_condition_difficulty = false,
            is_special_spell_target = false,
            takes_fixed_meter_damage = false,
            qualifies_for_party_support_bonus = false,
            is_ghoul_bounty_target = false,
        },
        actor_facts = { interaction = {
            no_coverage_difficulty_bonus = false,
            defense_difficulty_reduction = false,
        }},
        cyberware = {},
    }
    local average = command_average(attacker)
    local reduced = posture_reduction(
        average, attacker.posture_reduction_percent)
    local trial_count = weapon_action_skill_value(
        attacker, policy.skill_group, reduced)
    if trial_count == nil then
        return nil
    end
    local difficulty = condition_modifier(4, attacker, empty_child)
    difficulty = dynamic_status_modifier(
        difficulty, attacker, nil, context)
    if difficulty == nil then
        return nil
    end
    difficulty = weapon_action_eligibility(
        difficulty, attacker, policy)
    if difficulty == nil then
        return nil
    end
    difficulty = add_wound_difficulty_penalty(
        difficulty, attacker.physical_meter, attacker.mental_meter)
    difficulty = math.max(difficulty, 2)
    local successes = thresholded_rolls(
        trial_count, difficulty, context.ai_random_bound)
    if successes == nil then
        return nil
    end
    -- 0x55640 publishes the same byte to the runner and new primary record.
    return successes
end

local function primary_armor_modifier(target)
    for _, item in ipairs(game.modules.items.catalog or {}) do
        local armor = item.armor
        if type(armor) == "table" and
           item.id == target.equipped_armor_id then
            return armor.primary_modifier
        end
    end
    return target.equipped_armor_id == nil and 0 or nil
end

local function resolve_weapon_action_value(context, projectile,
                                           target, source_action)
    local average = command_average(target)
    local reduction = posture_reduction(
        average, target.posture_reduction_percent)
    local trial_count = add_source_byte_to_word(
        average - reduction, target.attributes.body)
    trial_count = trial_count +
        (combat_cyberware(target).melee_defense_bonus or 0)
    local difficulty = dynamic_status_modifier(
        source_action.difficulty, target, nil, context)
    if difficulty == nil then
        return nil
    end
    local successes = thresholded_rolls(
        trial_count, math.max(difficulty, 2), context.ai_random_bound)
    if successes == nil then
        return nil
    end

    -- ROM 0x559BC subtracts the target roll from the projectile's +0x9F seed
    -- using a signed byte, then adds that delta to 0x55948's effect value.
    local delta = signed_byte(projectile.attack_successes - successes)
    local action_value = source_action.effect_value
    if delta < 0 then
        action_value = signed_byte(add_source_byte(action_value, delta))
        if action_value <= 0 then
            return 0
        end
    else
        action_value = action_value + (delta & 0xff)
        if action_value >= 0x0b then
            action_value = 0x0a
        end
    end

    local armor_modifier = primary_armor_modifier(target)
    if armor_modifier == nil then
        return nil
    end
    action_value = signed_byte(subtract_source_byte(
        action_value, armor_modifier))
    if action_value <= 0 then
        return 0
    end
    -- The 0x14FF8 record is freshly zeroed before the source copies +0x56/+0x57.
    local blank_projectile = {
        actor_facts = { interaction = {
            no_coverage_difficulty_bonus = false,
            defense_difficulty_reduction = false,
        }},
        inventory = {},
    }
    action_value = defense_modifier(action_value, blank_projectile)
    return math.max(action_value, 0)
end

function Combat.resolve_weapon_hit(context, projectile, target)
    if type(context) ~= "table" or type(projectile) ~= "table" or
       type(projectile.attack_successes) ~= "number" or
       projectile.attack_successes < 0 or
       projectile.attack_successes ~= math.floor(projectile.attack_successes) or
       type(target) ~= "table" or type(target.combatant) ~= "table" or
       type(context.ai_random_bound) ~= "function" or
       type(target.id) ~= "string" or target.id == "" then
        return nil
    end
    local source_action = source_weapon_action(context.projectile_action_id)
    if source_action == nil then
        return nil
    end
    local defender = copy_combatant(target.combatant)
    local damage = resolve_weapon_action_value(
        context, projectile, defender, source_action)
    if damage == nil then
        return nil
    end
    local physical_meter = defender.physical_meter
    local mental_meter = defender.mental_meter
    if damage > 0 then
        physical_meter, mental_meter = adjusted_meters(
            defender, true, damage, context)
    end
    local changed = commit_combatant_meters(
        target, defender, physical_meter, mental_meter)
    return {
        target_id = target.id,
        physical_meter = physical_meter,
        mental_meter = mental_meter,
        physical = true,
        changed = changed,
    }
end

return Combat
