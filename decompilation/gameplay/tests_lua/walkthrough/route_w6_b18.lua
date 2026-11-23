local E = require("enums")

local kRedBuffaloApproachX = 0x6100 -- Physical QA start immediately south of record 194 [C]
local kRedBuffaloApproachY = 0x4100 -- Physical QA start immediately south of record 194 [C]

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

local function visit_red_buffalo(host)
    local placed, placement_error = host.place_within_scene(
        E.Scene.SALISH_SHIDHE, kRedBuffaloApproachX,
        kRedBuffaloApproachY)
    assert(placed, placement_error or "Red Buffalo lodge placement failed")
    local opened, movement_error = host.hold_until_modal("UP", 80)
    assert(opened, movement_error or "Scene-6 event 1 did not open")
    assert(host.wait_for_dialogue(120), "Red Buffalo Woman service did not open")
    assert(host.snapshot().panel_actor == "Red Buffalo Woman",
        "Scene-6 event 1 opened a different speaker")
    assert(host.snapshot().panel_text:find("lone AmerIndian woman", 1, true),
        "Red Buffalo Woman's live introduction page was not shown")

    for _ = 1, 4 do
        if not host.snapshot().dialogue_open then return end
        tap(host, "A") -- Advance the live introduction and Lady Gillian gift pages.
    end
    assert(not host.snapshot().dialogue_open,
        "Red Buffalo Woman's gift service did not return to Scene 6")
end

local story = E.StoryFact
local mode = E.FieldModeFlagBit
local red_gift = E.GroupItemId.GIFT_RED_BUFFALO_WOMAN
local lady_gift = E.GroupItemId.GIFT_LADY_GILLIAN

return {
    resume_from_fixture_step = 1,
    steps = {
        {
            id = "deliver_red_buffalo_gift_b18",
            milestone = 18,
            expected = {
                scene = E.Scene.SALISH_SHIDHE,
                facts = { [story.RED_BUFFALO_WOMAN] = true },
                flags = {
                    story = { set = { story.RED_BUFFALO_WOMAN } },
                    mode = {
                        set = { mode.BIT_30 },
                        clear = { mode.BIT_29 },
                    },
                },
                group_items = {
                    clear = { red_gift },
                    set = { lady_gift },
                },
            },
            run = function(host)
                local snapshot = host.snapshot()
                assert(snapshot.scene == E.Scene.SALISH_SHIDHE,
                    "b18 requires a production-reached Scene-6 checkpoint")
                assert(snapshot.story_facts[story.RITUAL_OF_BINDING] == true and
                       snapshot.group_items[red_gift] == true,
                    "live b17 prerequisites or its gift are missing")
                visit_red_buffalo(host)
                return "Red Buffalo Woman consumed her gift and set b18 through live dialogue"
            end,
        },
    },
}
