local E = require("enums")

local kHospitalEventCenterX = 26880 -- Scene-0 record 52 bounds 0x0680..0x069F [C]
local kHospitalEventCenterY = 14080 -- Scene-0 record 52 bounds 0x0360..0x037F [C]
local kInteractionFrames = 600
local kDialogueWaitFrames = 120
local kConversationInputLimit = 40

local fact = E.StoryFact
local story = E.StoryFlagBit
local effect = E.FieldModeEffectFlagBit
local heart = E.GroupItemId.PROTOTYPE_CYBERHEART

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

return {
    steps = {
        {
            id = "prepare_rom_state_for_cyberheart_turn_in",
            milestone = 29,
            expected = {
                facts = {
                    [fact.HOLOPIX_SHOWN] = true,
                    [fact.MEDICINE_LODGE_TURNED_AWAY] = true,
                    [fact.HOSPITAL_VISITED] = true,
                },
                group_items = { set = { heart } },
                flags = {
                    story = { set = {
                        fact.HOLOPIX_SHOWN,
                        fact.MEDICINE_LODGE_TURNED_AWAY,
                        fact.HOSPITAL_VISITED,
                    } },
                    effect = { set = { effect.CYBERHEART_CARRIED_TIP } },
                },
            },
            run = function(host)
                -- ROM Seattle General path: bit 23 at 0x21EF8 records the
                -- holopix; service 0x1FDEA sets bit 24; 0x1F8C2 sets bit 25.
                for _, bit in ipairs({ story.HOLOPIX_SHOWN,
                        story.MEDICINE_LODGE_TURNED_AWAY,
                        story.HOSPITAL_VISITED }) do
                    local ok, problem = host.set_story_fact_for_setup(bit, true)
                    assert(ok, problem or "hospital prerequisite setup failed")
                end
                -- ROM service 0x1F8F8 consumes the prototype cyberheart;
                -- 0x1F918 clears carried-tip effect bit 20.
                assert(host.grant_group_item(heart))
                local tip, tip_error = host.set_field_mode_effect_flag_for_setup(
                    effect.CYBERHEART_CARRIED_TIP, true)
                assert(tip, tip_error or "cyberheart carried-tip setup failed")
                return "seeded hospital prerequisites and granted the quest heart"
            end,
        },
        {
            id = "deliver_cyberheart_to_seattle_general_b29",
            milestone = 29,
            expected = {
                scene = E.Scene.DOWNTOWN_SEATTLE,
                group_items = { clear = { heart } },
                facts = { [fact.HEART_DELIVERED] = true },
                flags = {
                    story = { set = { fact.HEART_DELIVERED } },
                    effect = { clear = { effect.CYBERHEART_CARRIED_TIP } },
                },
            },
            run = function(host)
                local placed, placement_error = host.place_within_scene(
                    E.Scene.DOWNTOWN_SEATTLE,
                    kHospitalEventCenterX, kHospitalEventCenterY)
                assert(placed, placement_error or
                    "Seattle General event placement failed")
                local opened, movement_error = host.hold_until_modal(
                    "UP", kInteractionFrames)
                assert(opened, movement_error or
                    "real UP did not open Seattle General")
                assert(host.wait_for_dialogue(kDialogueWaitFrames),
                    "Seattle General heart-turn-in dialogue did not open")
                assert(host.snapshot().panel_actor == "Ivanna Haversheen",
                    "Scene-0 record 52 opened a different speaker")
                for _ = 1, kConversationInputLimit do
                    local state = host.snapshot()
                    if not state.dialogue_open then
                        return "Ivanna accepted the heart through the live b29 service"
                    end
                    tap(host, state.panel_option_count == 2 and "B" or "A")
                end
                error("Seattle General b29 dialogue did not finish")
            end,
        },
    },
}
