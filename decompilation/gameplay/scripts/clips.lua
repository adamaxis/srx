-- Stable IDs for the Clips selector and its nested trade selector.
-- FieldClipsMenuRuntime owns focus, input cadence, and item/party changes.
local Clips = {}

local function label_keys(ids, prefix)
    local keys = {}
    for index, id in ipairs(ids) do
        keys[index] = prefix .. id
    end
    return keys
end

function Clips.commands()
    local ids = { "reload", "trade", "distribute", "cancel" }
    return ids, nil, label_keys(ids, "clips.option.")
end

function Clips.trade_commands()
    local ids = {
        "increase_clips",
        "decrease_clips",
        "trade_clips",
        "change_recipient",
    }
    return ids, nil, label_keys(ids, "clips.trade.option.")
end

-- ROM-derived field copy uses stable semantic keys; source text remains in
-- dialogue/script-text.json and native state supplies runner/item names.
function Clips.text_entries()
    local ids = {
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
        "function_aborted",
    }
    return ids, nil, label_keys(ids, "clips.text.option.")
end

return Clips
