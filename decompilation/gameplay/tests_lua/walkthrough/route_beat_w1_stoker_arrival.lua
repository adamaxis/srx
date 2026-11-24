local E = require("enums")

local function advance(host, frames)
    local ok, problem = host.advance(frames)
    assert(ok, problem or "Stoker arrival frame advance failed")
end

local function wait_panel(host, text)
    for _ = 1, 1800 do
        if host.snapshot().panel_text:find(text, 1, true) then return end
        advance(host, 1)
    end
    error("Stoker arrival panel did not contain " ..
        string.format("%q", text))
end

return {
    steps = {
        {
            id = "w1_new_game_reaches_stoker",
            milestone = 0,
            expected = { scene = E.Scene.STOKER_MOTEL },
            run = function(host)
                assert(host.wait_for_scene(E.Scene.STOKER_MOTEL, 180),
                    "production opening did not reach Stoker's scene")
                wait_panel(host, "battered sign reads Stoker's Coffin Motel")
                assert(host.snapshot().panel_actor == "Stoker",
                    "opening arrival page belongs to a different actor")
                local pressed, press_error = host.press("A")
                assert(pressed, press_error or
                    "real A input did not advance Stoker's arrival page")
                assert(host.snapshot().dialogue_open,
                    "Stoker's arrival dialogue closed while advancing")
                return "production opening arrived at Stoker; real A advanced his page"
            end,
        },
    },
}
