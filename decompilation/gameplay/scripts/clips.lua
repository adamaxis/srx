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

-- ROM-derived field copy uses stable semantic keys; source text remains in
-- dialogue/script-text.lua and native state supplies runner/item names.
function Clips.text_entries()
    return {
        "header_title",
        "header_separator",
        "counter_label",
        "trade_prompt_action",
        "trade_prompt_clips_to",
        "trade_prompt_recipient",
        "exchange_complete_first",
        "exchange_complete_second",
        "reload_prompt_ammo_remaining",
        "reload_prompt_ammo_suffix",
        "reload_prompt_question",
        "reload_success_first",
        "reload_success_second",
        "out_of_ammo_name_suffix",
        "out_of_ammo_clips",
        "item_without_clips_first",
        "item_without_clips_second",
        "item_without_clips_third",
        "no_party_first",
        "no_party_second",
        "no_party_third",
    }
end

return Clips
