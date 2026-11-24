local E = require("enums")

-- Scene-7 record 206 (0x1B4592, x 26112..26623, y 21504..22015) is the
-- medicine lodge door, Scene-7/event 0. The event table names field Scene 13,
-- but ROM finalizer 0x1FDBE (FFFBF8 = 7, FFFBF9 = 0) ends every branch at
-- 0x1FE7A..0x1FE80 with D6 = -1, cancelling that route: the lodge only shows
-- its text and the party stays in Scene 7.
local kScene7LonghouseX = 26368
local kScene7ApproachY = 22272
local kInputFrames = 600
local kSettleFrames = 180
local kMaxLodgePages = 64

local story = E.StoryFact

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

-- Walk UP into the lodge door with real pad input. ROM 0x1FDBE opens a
-- profile-0x121E08 dialogue root; A advances every record (single replies
-- and <MORE> continuations alike). Require the root's first and last ROM
-- pages, then require that the cancelled route leaves the party in Scene 7.
local function enter_lodge(host, first_text, last_text, rom_root)
    local placed, placement_error = host.place_within_scene(
        E.Scene.SCENE_7, kScene7LonghouseX, kScene7ApproachY)
    assert(placed, placement_error or
        "Scene-7 longhouse approach placement failed")
    local opened, open_error = host.hold_until_modal("UP", kInputFrames)
    assert(opened, open_error or "Scene-7 lodge door did not open its dialogue")
    local pages = {}
    for _ = 1, kMaxLodgePages do
        local state = host.snapshot()
        if not state.dialogue_open then break end
        if pages[#pages] ~= state.panel_text then
            pages[#pages + 1] = state.panel_text
        end
        local pressed, press_error = host.press("A")
        assert(pressed, press_error or "could not advance lodge dialogue")
    end
    assert(not host.snapshot().dialogue_open,
        "Scene-7 lodge dialogue did not close")
    assert(#pages > 0 and pages[1]:find(first_text, 1, true),
        "Scene-7 lodge did not open ROM root " .. rom_root .. ": " ..
        tostring(pages[1]))
    assert(pages[#pages]:find(last_text, 1, true),
        "Scene-7 lodge root " .. rom_root .. " did not end on its ROM page: " ..
        tostring(pages[#pages]))
    advance(host, kSettleFrames)
    assert(host.snapshot().scene == E.Scene.SCENE_7,
        "Scene-7 lodge dialogue did not cancel the event-table route")
    return #pages
end

return {
    steps = {
        {
            id = "w5_scene7_first_lodge_visit_turns_away",
            milestone = 24,
            expected = {
                scene = E.Scene.SCENE_7,
                facts = {
                    [story.ILENE_MET] = true,
                    [story.MEDICINE_LODGE_TURNED_AWAY] = true,
                },
                flags = {
                    story = {
                        set = { story.ILENE_MET, story.MEDICINE_LODGE_TURNED_AWAY },
                    },
                    -- Command 0x42 (0x21F80) sets FFFC34 bit 18.
                    effect = {
                        set = { E.FieldModeEffectFlagBit.STARK_DYING_TIP },
                    },
                },
            },
            run = function(host)
                set_up_input_scene(host)
                -- ROM 0x54402 sets FFFC1E bit 19 (b19, before the lodge).
                local prerequisite_set, prerequisite_error =
                    host.set_story_fact_for_setup(
                        E.StoryFlagBit.ILENE_MET, true)
                assert(prerequisite_set, prerequisite_error or
                    "Scene-7 lodge prerequisite setup failed")
                assert(host.snapshot().story_facts[
                    story.MEDICINE_LODGE_TURNED_AWAY] == false,
                    "first lodge visit requires FFFC1E bit 24 clear")
                -- ROM 0x1FDCA..0x1FDF4: set bit 24, select 0x11B714, whose
                -- eleven records run from text 0x11B804 to 0x11C014.
                local pages = enter_lodge(host,
                    "<You enter a large, dim building of AmerIndian design.",
                    "If you could fetch help he may yet live.", "0x11B714")
                assert(pages >= 11, "0x11B714 showed only " .. pages .. " pages")
                return "first lodge visit set bit 24, played 0x11B714 through 0x11C014, and stayed in Scene 7"
            end,
        },
        {
            id = "w5_scene7_lodge_revisit_before_hospital",
            milestone = 24,
            expected = {
                scene = E.Scene.SCENE_7,
                facts = {
                    [story.MEDICINE_LODGE_TURNED_AWAY] = true,
                    [story.HOSPITAL_VISITED] = false,
                },
            },
            run = function(host)
                -- ROM 0x1FDFE..0x1FE0C: bit 24 set, bit 25 clear selects
                -- 0x11B7E4 and still cancels the route.
                enter_lodge(host, "The time for Stark to join his ancestors draws nigh.",
                    "You do not have much more time to save him.",
                    "0x11B7E4 -> 0x11C0EA")
                return "lodge revisit showed 0x11B7E4 and stayed in Scene 7"
            end,
        },
        {
            id = "w5_scene7_lodge_visit_after_hospital_heals",
            milestone = 25,
            expected = {
                scene = E.Scene.SCENE_7,
                facts = { [story.HOSPITAL_VISITED] = true },
                flags = { story = { set = { story.HOSPITAL_VISITED } } },
            },
            run = function(host)
                -- Seattle General 0x1F8C2 sets FFFC1E bit 25 when Stark is
                -- admitted; the lodge then heals through 0x59BCC.
                local admitted, admitted_error = host.set_story_fact_for_setup(
                    E.StoryFlagBit.HOSPITAL_VISITED, true)
                assert(admitted, admitted_error or
                    "Stark admission prerequisite setup failed")
                enter_lodge(host, "Thank you for saving one of our own.",
                    "You leave feeling refreshed and alive!",
                    "0x11B7EC -> 0x11C14C")
                return "lodge visit after Stark's admission showed 0x11B7EC and stayed in Scene 7"
            end,
        },
    },
}
