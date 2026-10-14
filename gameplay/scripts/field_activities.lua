-- Shared map/resource activity menus. Lua owns cursor state and the action
-- selected by the player; the Qt panel only presents each yielded menu frame.
local FieldActivities = {}

local function run_cursor_menu(menu_id, choices)
    local selected = 1
    while true do
        local input = game.ui.cursor_menu(menu_id, choices, selected)
        if input == "up" then
            selected = (selected - 2) % #choices + 1
        elseif input == "down" then
            selected = selected % #choices + 1
        elseif input == "confirm" then
            return choices[selected]
        elseif input == "cancel" then
            return "exit"
        end
    end
end

function FieldActivities.resource_scene_main_menu()
    local choice = run_cursor_menu("resource_main", {
        "cyberspace",
        "exit",
    })

    if choice == "exit" then
        return "return_from_world_activity"
    end
    if choice == "cyberspace" then
        -- The ROM has a separate prerequisite and Matrix-entry route. Keep
        -- the source choice visible while that independent screen remains a
        -- boundary; do not turn it into a false field return.
        return "cyberspace_not_converted"
    end
    return nil
end

return FieldActivities
