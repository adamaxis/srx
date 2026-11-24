local E = require("enums")

local kScene7LonghouseX = 26368
local kScene7ApproachY = 22272
local kScene13ReturnX = 1280
local kScene13ApproachY = 3328
local kInputFrames = 600
local kSceneTransitionFrames = 180

local function advance(host, frames)
    local ok, problem = host.advance(frames)
    assert(ok, problem or "setup frame advance failed")
end

local function set_up_input_scene(host)
    for _ = 1, 600 do
        if not host.snapshot().opening_auto_walk then break end
        advance(host, 1)
    end
    assert(not host.snapshot().opening_auto_walk,
        "New Game opening walk did not settle before Scene-7 setup")
end

return {
    steps = {
        {
            id = "w5_scene7_first_exit_turns_away",
            milestone = 24,
            expected = {
                scene = E.Scene.FIELD_SCENE_13,
                facts = {
                    [E.StoryFact.ILENE_MET] = true,
                    [E.StoryFact.MEDICINE_LODGE_TURNED_AWAY] = true,
                },
                flags = {
                    story = {
                        set = {
                            E.StoryFact.ILENE_MET,
                            E.StoryFact.MEDICINE_LODGE_TURNED_AWAY,
                        },
                    },
                },
            },
            run = function(host)
                set_up_input_scene(host)

                -- ROM 0x54402 sets FFFC1E bit 19; Scene-7 record 206 at
                -- 0x1B4592 only selects the field path when this bit is set.
                local prerequisite_set, prerequisite_error =
                    host.set_story_fact_for_setup(
                        E.StoryFlagBit.ILENE_MET, true)
                assert(prerequisite_set, prerequisite_error or
                    "Scene-7 field-path prerequisite setup failed")

                local before = host.snapshot()
                assert(before.story_facts[
                    E.StoryFact.MEDICINE_LODGE_TURNED_AWAY] == false,
                    "first Scene-7 exit test requires its ROM flag clear")

                -- ROM record 206 (0x1B4592) lies at x 26112..26623,
                -- y 21504..22015; real UP establishes the saved source exit.
                local placed, placement_error = host.place_within_scene(
                    E.Scene.SCENE_7, kScene7LonghouseX, kScene7ApproachY)
                assert(placed, placement_error or
                    "Scene-7 longhouse approach placement failed")
                local entered, entry_error = host.hold_until_modal(
                    "UP", kInputFrames, E.Scene.FIELD_SCENE_13)
                assert(entered, entry_error or
                    "Scene-7 longhouse did not enter field Scene 13")
                assert(host.wait_for_scene(E.Scene.FIELD_SCENE_13,
                    kSceneTransitionFrames),
                    "Scene-7 record 206 did not load field Scene 13")

                -- Scene-13 records 9/10 at 0x1B3CE4/0x1B3CEE are the physical
                -- south-edge return. The mode-exit beat is the tested action.
                local return_placed, return_error = host.place_within_scene(
                    E.Scene.FIELD_SCENE_13,
                    kScene13ReturnX, kScene13ApproachY)
                assert(return_placed, return_error or
                    "Scene-13 exit approach placement failed")
                local exited, exit_error = host.hold_until_modal(
                    "DOWN", kInputFrames, E.Scene.SCENE_7)
                assert(exited, exit_error or
                    "Scene-13 physical exit did not begin")
                assert(host.wait_for_dialogue(120),
                    "first Scene-7 mode-exit text did not open")
                assert(host.snapshot().page_key ==
                    "field.mode.medicine_lodge.first_visit",
                    "Scene-7 exit did not select ROM text 0x11B714")
                local dismissed, dismiss_error = host.press("A")
                assert(dismissed, dismiss_error or
                    "could not dismiss first-visit mode-exit text")
                assert(host.wait_for_scene(E.Scene.SCENE_7,
                    kSceneTransitionFrames),
                    "Scene-13 exit did not restore Scene 7")
                return "left field Scene 13 by pad and received the first-visit Scene-7 text"
            end,
        },
    },
}
