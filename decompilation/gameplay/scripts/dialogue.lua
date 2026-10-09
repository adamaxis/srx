-- Speaker properties are supplied by gameplay content and resolved from the
-- speaker id carried by the active map/location context.
local Dialogue = {}

local speakers = {
    stoker = {
        name = "Stoker",
        text_profile = "stoker.motel",
        portrait_alias = "dialogue.portraits.stoker.motel",
        events = {
            arrival = {
                title_key = "stoker.motel.arrival.title",
                continuation_key = "stoker.motel.arrival.more",
            },
            leave = {
                title_key = "stoker.motel.leave.title",
                continuation_key = "",
            },
            ["room.paid"] = {
                title_key = "stoker.motel.room.paid.title",
                continuation_key = "",
            },
            ["room.insufficient"] = {
                title_key = "stoker.motel.room.insufficient.title",
                continuation_key = "",
            },
            ["package-handoff"] = {
                title_key = "stoker.motel.package-handoff.title",
                continuation_key = "",
            },
            ["jump-house"] = {
                title_key = "stoker.motel.jump-house.title",
                continuation_key = "",
            },
        },
    },
}

-- Menu copy belongs to the location's content profile. A shared building
-- menu can therefore be shown with or without a named speaker portrait.
local menu_profiles = {
    ["stoker.motel"] = {
        manager = {
            title_key = "stoker.motel.manager.title",
            choice_label_keys = {
                crash = "stoker.motel.option.crash",
                leave = "stoker.motel.option.leave",
                package = "stoker.motel.option.package",
            },
        },
        room = {
            title_key = "stoker.motel.room.title",
            choice_label_keys = {
                pay = "stoker.motel.option.pay",
                leave = "stoker.motel.option.leave",
            },
        },
        debt = {
            title_key = "stoker.motel.debt.title",
            choice_label_keys = {
                ["debt-payment"] = "stoker.motel.option.debt-payment",
                ["jump-house"] = "stoker.motel.option.jump-house",
            },
        },
    },
}

function Dialogue.speaker_profile(speaker_id)
    return speakers[speaker_id]
end

function Dialogue.event_text_keys(speaker_id, text_profile, event_id)
    local speaker = speakers[speaker_id]
    if speaker == nil or speaker.text_profile ~= text_profile or
       speaker.events == nil then
        return nil
    end
    return speaker.events[event_id]
end

function Dialogue.menu_text_keys(speaker_id, text_profile, menu_id, choice_ids)
    if speaker_id ~= "" then
        local speaker = speakers[speaker_id]
        if speaker == nil or speaker.text_profile ~= text_profile then
            return nil
        end
    end
    local profile = menu_profiles[text_profile]
    local menu = profile ~= nil and profile[menu_id] or nil
    if menu == nil then
        return nil
    end

    local choice_label_keys = {}
    for index, choice_id in ipairs(choice_ids) do
        local label_key = menu.choice_label_keys[choice_id]
        if label_key == nil then
            return nil
        end
        choice_label_keys[index] = label_key
    end
    return {
        title_key = menu.title_key,
        choice_label_keys = choice_label_keys,
    }
end

return Dialogue
