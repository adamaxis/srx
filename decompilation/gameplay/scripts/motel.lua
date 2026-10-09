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
    return game.ui.menu(game.location.text_profile, Motel.manager_choices())
end

return Motel
