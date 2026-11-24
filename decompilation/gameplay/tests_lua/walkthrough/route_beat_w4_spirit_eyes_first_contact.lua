local E = require("enums")

local kSpiritEyesEventX = 27904
local kSpiritEyesEventY = 2304
local kEventInputFrames = 600
local kConversationInputLimit = 20

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

return {
    steps = {
        {
            id = "w4_spirit_eyes_first_contact",
            milestone = 6,
            expected = {
                scene = E.Scene.COUNCIL_ISLAND,
                facts = { [E.StoryFact.SHAMAN_MET] = true },
                flags = {
                    story = { set = { E.StoryFact.SHAMAN_MET } },
                    mode = {
                        set = {
                            E.FieldModeFlagBit.BIT_15,
                            E.FieldModeFlagBit.BIT_16,
                        },
                        clear = { E.FieldModeFlagBit.BIT_14 },
                    },
                    auxiliary = {
                        set = { E.FieldAuxiliaryFlagBit.SHAMAN_RITUAL_STAGE_0 },
                    },
                },
            },
            run = function(host)
                -- Scene-4 event 27 / collision record 96 is Spirit Eyes'
                -- first-contact event in the production W4 route.
                local placed, placement_error = host.place_within_scene(
                    E.Scene.COUNCIL_ISLAND,
                    kSpiritEyesEventX, kSpiritEyesEventY)
                assert(placed, placement_error or
                    "Spirit Eyes first-contact placement failed")
                local opened, movement_error = host.hold_until_modal(
                    "UP", kEventInputFrames)
                assert(opened, movement_error or
                    "real UP did not open Spirit Eyes first contact")
                assert(host.snapshot().panel_actor == "Spirit Eyes",
                    "Scene-4 event 27 opened the wrong speaker")

                for _ = 1, kConversationInputLimit do
                    if not host.snapshot().dialogue_open then
                        return "completed Spirit Eyes' first-contact dialogue"
                    end
                    tap(host, "A")
                end
                error("Spirit Eyes first-contact dialogue did not finish")
            end,
        },
    },
}
