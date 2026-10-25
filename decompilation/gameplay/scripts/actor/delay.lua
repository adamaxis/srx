-- actor/delay.lua: shared source action-delay math for actors and the player.
local E = require("enums")
local I = require("lib.int68k")
local Weapons = require("data.weapons")
local u8 = I.u8

local M = {}
local Mode = E.ActionDelayMode
local Capability = E.DelayCapability
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

-- ROM 0xD7EA takes the first set +0x90 bit from 12 through 14, then adds
-- (bit-11)*4 to the wrapped average of +0x78 and +0x7B before subtracting one. [C]
local function delay_factor(primary_attribute, secondary_attribute,
                            capability_flags)
    local modifier = 0
    for bit = Capability.FIRST_BIT, Capability.THIRD_BIT do
        if (capability_flags & (1 << bit)) ~= 0 then
            modifier = (bit - Capability.MODIFIER_ORIGIN) *
                E.ActionDelay.FACTOR_STEP
            break
        end
    end

    -- ROM 0xD81C adds the two bytes before shifting, so the sum wraps at one byte. [C]
    local wrapped_sum = u8(primary_attribute + secondary_attribute)
    local factor = (wrapped_sum >> 1) + modifier - 1
    if factor < 0 then
        factor = 0
    end
    return factor
end

-- ROM 0xABFC computes 0x6A-factor*4, applies D0, then clamps signed D2 to 0x0F. [C]
function M.calculate(primary_attribute, secondary_attribute,
                      capability_flags, mode)
    mode = mode == nil and Mode.NORMAL or mode
    local delay = E.ActionDelay.BASE -
        delay_factor(primary_attribute, secondary_attribute, capability_flags) *
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

-- Actor behavior passes the record-backed attributes through this shared path. [C]
function M.actor_action_delay(context, state, mode)
    return M.calculate(
        context.delay_primary_attribute or state.delay_primary_attribute or 0,
        context.delay_secondary_attribute or state.delay_secondary_attribute or 0,
        context.delay_capability_flags or state.delay_capability_flags or 0,
        mode)
end

local function weapon_mode(weapon_code, use)
    local weapon = Weapons.get(weapon_code)
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

    if use == Use.SHARED_ACTION then
        return Mode.NORMAL
    end
    return nil
end

-- The host passes typed runner facts and an action purpose; Lua selects the
-- weapon-specific source mode and runs the same 0xABFC / 0xD7EA calculation. [C]
function M.player_weapon_action_delay(primary_attribute, secondary_attribute,
                                      capability_flags, weapon_code, use)
    local mode = weapon_mode(weapon_code, use)
    if mode == nil then
        return nil
    end
    return M.calculate(primary_attribute, secondary_attribute,
                       capability_flags, mode)
end

return M
