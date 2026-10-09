-- Map event conditions are content data. Event flow is ordinary sequential
-- Lua and delegates movement to the native actor controller.
local Events = {}

Events.map_events = {
    {
        id = 0x00020001,
        scene = 2,
        trigger = "scene_entry",
        scope = {
            min_x = 0x3CE0,
            max_x = 0x3CE0,
            min_y = 0x2D00,
            max_y = 0x2D00,
        },
        requires = { "new_game_opening_completed" },
        ["repeat"] = "once_per_session",
        action = "run_lua_handler",
        handler = "new_game_opening_walk",
    },
}

function Events.new_game_opening_walk()
    return game.actor.walk_heading("player", 0)
end

return Events
