-- Actor packages are keyed by the source template identity plus its declared
-- action-table/loadout code. ROM 0x44BC copies the template kind/action into
-- the live entity; Lua chooses the package and policy, while the native field
-- engine performs the requested animation/audio and owns physics/rendering.
local ActorAI = {}

local function entry_then_hold(context)
    if context.entry_delay == 0 and
       not context.entry_started and
       not context.entry_complete then
        return "play_entry"
    end
    return "hold"
end

local function make_decision(profile, context)
    local action = profile.decide(context)
    if action == "play_entry" then
        return {
            profile_id = profile.profile_id,
            package_id = profile.package_id,
            action = action,
            animation = profile.entry_animation,
            audio_event = profile.entry_audio_event,
            action_kind = profile.action_kind,
            action_code = profile.action_code,
            weapon_item_id = profile.weapon_item_id,
        }
    end

    return {
        profile_id = profile.profile_id,
        package_id = profile.package_id,
        action = "hold",
        animation = "none",
        action_kind = profile.action_kind,
        action_code = profile.action_code,
        weapon_item_id = profile.weapon_item_id,
    }
end

-- These are the only profile actors currently activated by the native field
-- runtime. The source templates identify the guard's D4540 item action (6,
-- Max-Power/Heavy Pistol) and the pedestrian's lack of an active action.
-- Their later combat/route behavior is deliberately left at the source
-- boundary until its conditions and collision hand-off have been ported.
local profiles = {
    ["corp_guard_k05_item06"] = {
        profile_id = "corp_guard_k05_item06",
        package_id = "corp_security.max_power_heavy_pistol",
        entity_kind = 5,
        action_kind = "item",
        action_code = 6,
        weapon_item_id = 6,
        entry_animation = "entry",
        entry_audio_event = 0x2e,
        decide = entry_then_hold,
    },
    ["pedestrian_k0b_none00"] = {
        profile_id = "pedestrian_k0b_none00",
        package_id = "pedestrian.no_active_action",
        entity_kind = 11,
        action_kind = "none",
        action_code = 0,
        entry_animation = "entry",
        entry_audio_event = 0x2f,
        decide = entry_then_hold,
    },
}

function ActorAI.behavior(context)
    if type(context) ~= "table" then
        return nil
    end

    local profile = profiles[context.profile_id]
    if profile == nil or
       context.entity_kind ~= profile.entity_kind or
       context.action_kind ~= profile.action_kind or
       context.action_code ~= profile.action_code then
        return nil
    end

    return make_decision(profile, context)
end

return ActorAI
