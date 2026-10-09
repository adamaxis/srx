-- Lua selects actor packages from the source entity kind and its initial
-- action code. Native C++ owns movement, collision, timing, rendering and
-- sound playback; the package decides which named actions to request.
local ActorAI = {}

local packages = {
    [5] = {
        -- ROM 0x44BC copies template +9 to live +0x57. This guard's action
        -- code 6 resolves to item 6 (Max-Power, Heavy Pistol) in D4540.
        [6] = {
            id = "corp_security.max_power_heavy_pistol",
            weapon_item_id = 6,
            entry_animation = "entry",
            entry_audio_event = 0x2e,
        },
    },
    [11] = {
        -- Scene-listed pedestrians have +0x57 = 0 and no active item action.
        [0] = {
            id = "pedestrian.no_active_action",
            entry_animation = "entry",
            entry_audio_event = 0x2f,
        },
    },
}

function ActorAI.behavior(entity_kind, action_code, entry_delay,
                         entry_started, entry_complete)
    local kind_packages = packages[entity_kind]
    local package = kind_packages and kind_packages[action_code]
    if package == nil then
        return nil
    end

    if entry_delay == 0 and not entry_started and not entry_complete then
        return {
            package_id = package.id,
            action = "play_entry",
            animation = package.entry_animation,
            audio_event = package.entry_audio_event,
            weapon_item_id = package.weapon_item_id,
        }
    end

    return {
        package_id = package.id,
        action = "hold",
        animation = "none",
        weapon_item_id = package.weapon_item_id,
    }
end

return ActorAI
