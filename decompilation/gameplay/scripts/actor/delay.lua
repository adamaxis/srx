-- actor/delay.lua: shared source action-delay math for actors and the player.
local E = require("enums")
local Weapons = require("data.weapons")

local M = {}
local Mode = E.ActionDelayMode
local Modifier = E.ActionDelayModifier
local Capability = E.ActionDelayCapability
local Use = {
    ATTACK = "attack",
    RELOAD_ACTION = "reload_action",
    SHARED_ACTION = "shared_action",
}
M.Use = Use

local weapon_scale_modes = {
    normal = Mode.NORMAL,
    half = Mode.HALF,
    double = Mode.DOUBLE,
}

-- The importer translates the source capability word into a semantic action
-- delay modifier. Lua combines that value with named actor attributes. [C]
local function delay_factor(primary_attribute, secondary_attribute,
                            action_delay_modifier)
    -- ROM 0xD81C averages the named Quickness and Intelligence values [C].
    local attribute_average = math.floor(
        (primary_attribute + secondary_attribute) / 2)
    local factor = attribute_average + action_delay_modifier - 1
    if factor < 0 then
        factor = 0
    end
    return factor
end

-- ROM 0xABFC computes 0x6A-factor*4, applies D0, then clamps signed D2 to 0x0F. [C]
function M.calculate(primary_attribute, secondary_attribute,
                      action_delay_modifier, mode)
    mode = mode == nil and Mode.NORMAL or mode
    local delay = E.ActionDelay.BASE -
        delay_factor(primary_attribute, secondary_attribute,
                     action_delay_modifier) *
            E.ActionDelay.FACTOR_STEP

    if mode == Mode.DOUBLE then
        delay = delay * 2
    elseif mode ~= Mode.NORMAL then
        -- ASR rounds negative odd values down, which math.floor preserves. [C]
        delay = math.floor(delay / 2)
    end
    if delay < E.ActionDelay.MINIMUM then
        delay = E.ActionDelay.MINIMUM
    end
    return delay
end

-- Actor behavior uses the combatant's named attributes and imported modifier.
function M.actor_action_delay(context, state, mode)
    local timing = type(context.action_timing) == "table" and
                   context.action_timing or state.action_timing
    timing = type(timing) == "table" and timing or {}
    return M.calculate(
        timing.quickness or 0,
        timing.intelligence or 0,
        timing.action_delay_modifier or 0,
        mode)
end

local function weapon_mode(weapon_id, use)
    if use == Use.SHARED_ACTION then
        return Mode.NORMAL
    end

    local weapon = Weapons.get(weapon_id)
    if weapon == nil then
        return nil
    end

    if use == Use.ATTACK then
        return weapon_scale_modes[weapon.attack_delay_scale]
    end

    if use == Use.RELOAD_ACTION then
        -- ROM 0x14226 decrements +0x57 first and selects D0=2 only for
        -- equipped codes 5, 11, and 12; data/weapons.lua carries that per-row
        -- reload_delay_scale and is the single source (delay_tests checks it). [C]
        return weapon_scale_modes[weapon.reload_delay_scale]
    end

    return nil
end

local function has_delay_capability(capability_flags, flag)
    return math.floor(capability_flags / flag) % 2 == 1
end

local function player_delay_modifier(capability_flags)
    -- ROM 0xD7EA reads +0x90 and checks bits 12, 13, then 14. The player
    -- bridge supplies that source word, so convert it here before passing a
    -- semantic modifier to the shared calculation. Lower bits are unrelated
    -- flags and do not change the action delay. [C]
    capability_flags = capability_flags or 0
    if has_delay_capability(capability_flags,
                             Capability.FACTOR_BONUS_4) then
        return Modifier.FACTOR_BONUS_4
    elseif has_delay_capability(capability_flags,
                                Capability.FACTOR_BONUS_8) then
        return Modifier.FACTOR_BONUS_8
    elseif has_delay_capability(capability_flags,
                                Capability.FACTOR_BONUS_12) then
        return Modifier.FACTOR_BONUS_12
    end
    return Modifier.NONE
end

-- Lua selects the weapon-specific mode and translates the source capability
-- flags into the semantic modifier used by the shared calculation. [C]
function M.player_weapon_action_delay(primary_attribute, secondary_attribute,
                                      capability_flags, weapon_id, use)
    local mode = weapon_mode(weapon_id, use)
    if mode == nil then
        return nil
    end
    return M.calculate(primary_attribute, secondary_attribute,
                       player_delay_modifier(capability_flags), mode)
end

return M
