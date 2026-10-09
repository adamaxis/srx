-- Shared motel service. Quest modules can extend the common manager menu by
-- returning stable choice ids from motel_manager_choices().
local Motel = {}

function Motel.manager_choices()
    local choices = { "crash", "leave" }

    local modules = {}
    for name in pairs(game.modules) do
        modules[#modules + 1] = name
    end
    table.sort(modules)

    for _, name in ipairs(modules) do
        local quest = game.modules[name]
        if type(quest.motel_manager_choices) == "function" then
            for _, choice in ipairs(quest.motel_manager_choices()) do
                choices[#choices + 1] = choice
            end
        end
    end

    return choices
end

function Motel.manager_interaction()
    local choice = game.ui.menu("manager", Motel.manager_choices())

    if choice == "leave" and game.location.speaker_id ~= nil then
        game.ui.say("leave")
        return choice
    end
    if choice == "crash" then
        return Motel.room_interaction()
    end

    return choice
end

function Motel.room_interaction()
    local choice = game.ui.menu("room", { "pay", "leave" })
    if choice == "leave" then
        game.ui.say("leave")
        return choice
    end
    if choice == "pay" then
        local result = game.services.call("motel", "pay_room")
        if result == "paid" then
            if game.location.speaker_id ~= nil then
                game.ui.say("room.paid")
            end
            return "hotel-options"
        end
        if result == "insufficient" then
            if game.location.speaker_id ~= nil then
                game.ui.say("room.insufficient")
            end
        end
    end
    return choice
end

return Motel
