-- Source-derived combat policy for close actor reactions. The host supplies
-- typed actor records and bounded ROM random draws; this module owns the
-- 0x55C76 trial sequence and returns the resulting meter snapshot.
local Combat = {}

local function signed_byte(value)
    value = value & 0xff
    return value >= 0x80 and value - 0x100 or value
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

local function command_average(record)
    local total = (record.command_attribute_78 +
                   record.command_attribute_7b +
                   record.command_attribute_7c) & 0xff
    local value = total >> 1
    for slot = 1, 8 do
        if record.item_codes[slot] == 0x28 then
            return (value + record.item_phases[slot]) & 0xff
        end
    end
    return value
end

local function posture_reduction(value, posture)
    return math.floor((value * posture * 25) / 100)
end

local function first_cyberware_modifier(flags, first_bit, bit_count)
    for offset = 0, bit_count - 1 do
        if (flags & (1 << (first_bit + offset))) ~= 0 then
            return offset + 1
        end
    end
    return 0
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
    local has_cyberware = (attacker.cyberware_flags & 0x0002) ~= 0
    if has_cyberware then
        difficulty = difficulty - 1
    end
    if target.class_code == 0x16 then
        return difficulty + 2
    end
    if has_cyberware or attacker.race_or_state == 0x04 or
       attacker.race_or_state == 0x02 or
       (target.interaction_flags & 0x80) == 0 then
        return difficulty
    end
    return difficulty + 4
end

local function ready_party_bonus(context)
    local bonus = 0
    for _, candidate in ipairs(context.ai_target_candidates or {}) do
        local record = candidate.combatant
        if not candidate.world_activity_actor and type(record) == "table" and
           (record.source_flags & 0x01) ~= 0 and
           signed_byte(record.class_code) <= 1 then
            local item18 = record.item18_base
            for slot = 7, 1, -1 do
                if record.item_codes[slot] == 0x18 then
                    item18 = (item18 + record.item_phases[slot]) & 0xff
                    break
                end
            end
            bonus = bonus + math.floor(item18 / 4)
            if (item18 % 4) >= 3 then
                bonus = bonus + 1
            end
        end
    end
    return bonus
end

local function dynamic_status_modifier(difficulty, attacker, linked, context)
    local flags = attacker.action_profile_modifier_flags
    local primary = (flags & 0x01) ~= 0
    local secondary = not primary and (flags & 0x02) ~= 0
    if not primary and not secondary then
        return difficulty
    end

    local profiles = context.action_profile_modifiers
    local profile_index = linked.linked_action_profile_index + 1
    local profile = type(profiles) == "table" and profiles[profile_index] or nil
    if type(profile) ~= "table" then
        return nil
    end

    local roll_count
    local roll_difficulty
    if primary then
        roll_count = profile.primary_roll_count
        roll_difficulty = profile.primary_difficulty
    else
        roll_count = profile.secondary_roll_count
        roll_difficulty = profile.secondary_difficulty
    end

    -- The source D4540 selector bytes at rows 5 and 12 are both signed -1,
    -- so these two profile branches use the attacker's +0x77 attribute.
    local selectors = context.action_attribute_selectors
    local selector = type(selectors) == "table" and
        selectors[primary and 1 or 2] or nil
    if type(selector) ~= "number" then
        return nil
    end
    local attribute = selector < 0 and attacker.command_attribute_77 or
        attacker.command_attribute_7c
    if attacker.record_type < 3 then
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
    if (flags & 0x02) ~= 0 then
        remaining = math.floor(remaining / 2)
    end
    return difficulty + remaining
end

local function secondary_armor_modifier(target)
    local items = game.modules.items
    for _, item in ipairs(type(items) == "table" and items.catalog or {}) do
        local armor = item.armor
        if type(armor) == "table" and
           armor.armor_index == target.armor_index then
            return armor.secondary_modifier
        end
    end
    return 0
end

local function defense_modifier(value, attacker)
    if (attacker.interaction_flags & 0x20) ~= 0 then
        return value - 2
    end
    for slot = 1, 8 do
        if attacker.item_codes[slot] == 0x27 then
            return value - math.floor(attacker.item_phases[slot] / 2)
        end
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
    if record.record_type < 3 then
        if context.combat_meter_service_blocked then
            return physical_meter, mental_meter
        end
        local function advance_delay(delta)
            if (record.source_flags & 0x01) == 0 then
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
            if (record.source_flags & 0x01) == 0 then
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

    local delta = record.race_or_state < 0 and -1 or -damage
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
        if type(value) == "table" then
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
                                                  attacker.posture)
    local first_count = math.min(attacker_reduction,
                                 attacker.command_attribute_85) +
                        attacker.command_attribute_85
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
                                                  defender.posture)
    local second_count = minimum_plus_attribute(
        defender_average - defender_reduction,
        defender.command_attribute_85,
        defender.equipped_action_code == 0)
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

    local final_difficulty = attacker.command_attribute_79 +
        first_cyberware_modifier(attacker.cyberware_flags, 5, 4)
    local final_count = add_source_byte_to_word(
        second_count, defender.command_attribute_77)
    final_count = final_count + first_cyberware_modifier(
        defender.cyberware_flags, 9, 3)
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

    local direct_physical = (attacker.cyberware_flags & 0x0008) ~= 0
    local physical_penalty = (attacker.cyberware_flags & 0x0004) ~= 0
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
    local changed = physical_meter ~= defender.physical_meter or
                    mental_meter ~= defender.mental_meter
    if changed then
        local state = game.actors[target.id]
        if type(state) ~= "table" then
            state = { actor_id = target.id }
            game.actors[target.id] = state
        end
        state.combatant = copy_combatant(defender)
        state.combatant.physical_meter = physical_meter
        state.combatant.mental_meter = mental_meter
    end
    return {
        target_id = target.id,
        record_type = defender.record_type,
        physical_meter = physical_meter,
        mental_meter = mental_meter,
        physical = physical,
        changed = changed,
        exhausted = changed and defender.record_type >= 3 and
            ((physical and physical_meter == 0) or
             (not physical and mental_meter == 0)),
    }
end

return Combat
