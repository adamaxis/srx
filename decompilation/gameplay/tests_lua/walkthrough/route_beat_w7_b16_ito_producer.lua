local E = require("enums")

local kMatchstickApproachX = 16640
local kMatchstickApproachY = 10496

local function tap(host)
    local ok, problem = host.press("A")
    assert(ok, problem or "pad A did not advance the Matchstick dialogue")
end

return {
    steps = {
        {
            id = "prepare_ito_producer_prerequisites",
            milestone = 16,
            expected = {
                scene = E.Scene.DOWNTOWN_SEATTLE,
                flags = { story = { set = { E.StoryFact.OGAMI_LEFT } } },
            },
            run = function(host)
                -- ROM Scene-0 event 3 handler 0x54222 selects its descriptor
                -- and marks FFFC1E bit 15 at 0x5427A. Seed this prerequisite;
                -- command 0x59 at 0x22452 requires Reputation >= 6.
                local lead_set, lead_error = host.set_story_fact_for_setup(
                    E.StoryFlagBit.OGAMI_LEFT, true)
                assert(lead_set, lead_error or "Ito lead setup failed")
                local reputation_set, reputation_error =
                    host.set_runner_reputation_for_setup(6)
                assert(reputation_set, reputation_error or
                    "Matchstick Reputation setup failed")
                -- Scene-0 record 48 at 0x0D57BE is the real Matchstick door.
                local placed, placement_error = host.place_within_scene(
                    E.Scene.DOWNTOWN_SEATTLE,
                    kMatchstickApproachX, kMatchstickApproachY)
                assert(placed, placement_error or
                    "Matchstick exterior setup placement failed")
                return "seeded only Ogami's lead and Reputation, then placed at Scene-0 record 48"
            end,
        },
        {
            id = "walk_into_matchstick_and_queue_ito",
            milestone = 16,
            expected = {
                panel_text_contains = "You push your way into the club.",
                source_event = "field.world_activity.matchstick_ito_activity_queued",
            },
            run = function(host)
                local entered, movement_error = host.hold_until_modal("UP", 600)
                assert(entered, movement_error or
                    "real UP did not open the Matchstick door interaction")
                assert(host.wait_for_dialogue(180),
                    "Matchstick doorman entry page did not open")
                local first = host.snapshot()
                assert(first.panel_text:find(
                    "This is Matchstick, a high-class nightclub", 1, true),
                    "ROM descriptor 0x110ADA / text 0x112E24 was not shown")
                for _ = 1, 6 do
                    local current = host.snapshot()
                    if current.panel_text:find(
                        "You push your way into the club.", 1, true) then
                        break
                    end
                    tap(host)
                    host.advance(2)
                    assert(host.wait_for_dialogue(180),
                        "Matchstick doorman dialogue closed before the Ito intro")
                end
                local after = host.snapshot()
                assert(after.panel_text:find(
                    "You push your way into the club.", 1, true),
                    "ROM descriptor 0x110B22 / text 0x1130F9 was not shown")
                return "walked through the Scene-0 door, then A reached the Ito producer and ROM intro text"
            end,
        },
    },
}
