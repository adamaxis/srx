local E = require("enums")

local kInnRecord68X = 21760
local kInnRecord68Y = 3328

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

local function advance(host, frames)
    local ok, problem = host.advance(frames)
    assert(ok, problem or "Tarislar dialogue advance failed")
end

local function wait_panel(host, text)
    for _ = 1, 900 do
        local state = host.snapshot()
        if state.panel_text:find(text, 1, true) then return end
        advance(host, 1)
    end
    error("Tarislar dialogue did not contain " .. string.format("%q", text))
end

return {
    steps = {
        {
            id = "w1_tarislar_ask_shale",
            milestone = 1,
            expected = {
                scene = E.Scene.PUYALLUP_BARRENS,
                karma_delta = { 1, 1, 1 },
                facts = {
                    [E.StoryFact.MOTEL_BILL_PAID] = true,
                    [E.StoryFact.TARISLAR_ASKED_SHALE] = true,
                },
                flags = {
                    story = {
                        set = {
                            E.StoryFact.MOTEL_BILL_PAID,
                            E.StoryFact.TARISLAR_ASKED_SHALE,
                        },
                    },
                    mode = {
                        set = {
                            E.FieldModeFlagBit.BIT_5,
                            E.FieldModeFlagBit.BIT_6,
                            E.FieldModeFlagBit.BIT_8,
                        },
                        clear = {
                            E.FieldModeFlagBit.BIT_2,
                            E.FieldModeFlagBit.BIT_4,
                        },
                    },
                },
            },
            run = function(host)
                -- Scene-1 record 68/event 1 (0x17D9AC) selects the Inn title
                -- service only with story bit 0 set and bit 1 clear.
                local prerequisite_set, prerequisite_error =
                    host.set_story_fact_for_setup(
                        E.StoryFlagBit.MOTEL_BILL_PAID, true)
                assert(prerequisite_set, prerequisite_error or
                    "ROM Tarislar Inn prerequisite setup failed")
                local placed, placement_error = host.place_within_scene(
                    E.Scene.PUYALLUP_BARRENS, kInnRecord68X, kInnRecord68Y)
                assert(placed, placement_error or
                    "Tarislar City Inn setup placement failed")
                local opened, movement_error =
                    host.hold_until_modal("UP", 600)
                assert(opened, movement_error or
                    "real UP did not reach Tarislar City Inn record 68")
                wait_panel(host, "The Tarislar City Inn caters mostly to elves")
                tap(host, "A")
                tap(host, "C")
                wait_panel(host, "Tabatha Shale?  Yeah, she's here.")
                tap(host, "A")
                wait_panel(host, "manager connects you to her room")
                tap(host, "A")
                wait_panel(host, "she concedes that you are Michael's brother")
                tap(host, "A")
                wait_panel(host, "Tabatha Shale opens the door")
                tap(host, "B") -- ROM's one-choice follow-up returns without opening a new topic.
                assert(not host.snapshot().dialogue_open,
                    "Tabatha Shale intro did not return to the field")
                return "asked about and completed Tabatha Shale's source introduction"
            end,
        },
    },
}
