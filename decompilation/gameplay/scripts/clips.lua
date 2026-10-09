-- Stable IDs for the Clips selector and its nested trade selector.
-- FieldClipsMenuRuntime owns focus, input cadence, and item/party changes.
local Clips = {}

function Clips.commands()
    return { "reload", "trade", "distribute", "cancel" }
end

function Clips.trade_commands()
    return {
        "increase_clips",
        "decrease_clips",
        "trade_clips",
        "change_recipient",
    }
end

return Clips
