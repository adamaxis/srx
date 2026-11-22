local E = require("enums")
local Taxi = require("data.taxi_destinations")

local function tap(host, button, hold_frames, settle_frames)
    local ok, problem
    if hold_frames == nil then
        ok, problem = host.press(button)
    elseif settle_frames == nil then
        ok, problem = host.press(button, hold_frames)
    else
        ok, problem = host.press(button, hold_frames, settle_frames)
    end
    assert(ok, problem or ("pad input failed: " .. button))
end

local function place_and_enter(host, scene, x, y, description, direction)
    local placed, placement_error = host.place_within_scene(scene, x, y)
    assert(placed, placement_error or (description .. " placement failed"))
    direction = direction or "UP"
    local opened, movement_error = host.hold_until_modal(direction, 600)
    assert(opened, movement_error or
        ("real " .. direction .. " did not reach " .. description))
    assert(host.wait_for_dialogue(120), description .. " did not open dialogue")
end

local function ride_taxi(host, destination_region, destination_scene)
    place_and_enter(host, E.Scene.SALISH_SHIDHE, 7424, 2304,
        "Salish physical taxi terminal")
    tap(host, "A")
    assert(host.wait_for_taxi_selector(180), "Salish cab map did not open")
    for _ = 1, (Taxi.region_to_cursor[destination_region] -
        Taxi.region_to_cursor[Taxi.Region.SALISH_SHIDHE]) %
        #Taxi.cursor_to_region do
        tap(host, "LEFT")
    end
    tap(host, "A")
    assert(host.wait_for_dialogue(120), "taxi destination dialogue did not open")
    tap(host, "A")
    tap(host, "A")
    for _ = 1, 120 do
        local snapshot = host.snapshot()
        if snapshot.panel_open and snapshot.panel_text:find("Hop in, chum.", 1, true) then
            tap(host, "A", 1, 60)
            assert(host.wait_for_scene(destination_scene, 180),
                "accepted fare did not reach Council Island")
            return
        end
        local ok, problem = host.advance(1)
        assert(ok, problem or "taxi dialogue wait failed")
    end
    error("taxi never presented its ride page")
end

local function finish_spirit_eyes_conversation(host)
    for _ = 1, 20 do
        if not host.snapshot().dialogue_open then return end
        tap(host, "A")
    end
    local snapshot = host.snapshot()
    error("Spirit Eyes conversation did not return to the field: " ..
        tostring(snapshot.panel_text))
end

local scene = E.Scene.COUNCIL_ISLAND
local fact = E.StoryFact
local mode = E.FieldModeFlagBit
local auxiliary = E.FieldAuxiliaryFlagBit
local horn = E.GroupItemId.GARGOYLE_HORN
local pelt = E.GroupItemId.HELL_HOUND_PELT

return {
    resume_from_fixture_step = 1,
    steps = {
        {
            id = "taxi_salish_to_council_island",
            milestone = 5,
            expected = { scene = scene, nuyen_delta = -75 },
            run = function(host)
                ride_taxi(host, Taxi.Region.COUNCIL_ISLAND, scene)
                return "paid the real Salish-to-Council-Island cab fare"
            end,
        },
        {
            id = "meet_spirit_eyes_and_finish_first_contact",
            milestone = 6,
            expected = {
                scene = scene,
                facts = { [fact.SHAMAN_MET] = true },
                flags = {
                    story = { set = { fact.SHAMAN_MET } },
                    mode = {
                        set = { mode.BIT_15, mode.BIT_16 },
                        clear = { mode.BIT_14 },
                    },
                    auxiliary = { set = { auxiliary.SHAMAN_RITUAL_STAGE_0 } },
                },
            },
            run = function(host)
                place_and_enter(host, scene, 27904, 2304,
                    "Scene-4 Spirit Eyes event 27 record 96")
                assert(host.snapshot().panel_actor == "Spirit Eyes",
                    "event 27 opened a different speaker")
                finish_spirit_eyes_conversation(host)
                return "event 27 completed first contact and the horn request"
            end,
        },
        {
            id = "grant_gargoyle_horn_for_story_turn_in",
            milestone = 6,
            -- Current W4 acceptance ends at the verified b5 ritual checkpoint.
            gate = false,
            expected = { scene = scene, group_items = { set = { horn } } },
            run = function(host)
                assert(host.grant_group_item(horn)) -- Route setup grant: provide the quest Horn without combat grinding.
                return "granted the turn-in item for the source-gated Spirit Eyes service"
            end,
        },
        {
            id = "turn_in_gargoyle_horn",
            milestone = 6,
            expected = {
                scene = scene,
                group_items = { clear = { horn } },
                facts = { [fact.SHAMAN_ITEM4_GIVEN] = true },
                flags = {
                    story = { set = { fact.SHAMAN_ITEM4_GIVEN } },
                    mode = {
                        set = { mode.BIT_17 },
                        clear = { mode.BIT_16 },
                    },
                    auxiliary = {
                        set = { auxiliary.SHAMAN_RITUAL_STAGE_1 },
                        clear = { auxiliary.SHAMAN_RITUAL_STAGE_0 },
                    },
                },
            },
            run = function(host)
                place_and_enter(host, scene, 27904, 2032,
                    "Scene-4 Spirit Eyes Gargoyle Horn service", "DOWN")
                finish_spirit_eyes_conversation(host)
                return "Spirit Eyes consumed the Horn and advanced the ritual"
            end,
        },
        {
            id = "grant_hell_hound_pelt_for_story_turn_in",
            milestone = 7,
            expected = { scene = scene, group_items = { set = { pelt } } },
            run = function(host)
                assert(host.grant_group_item(pelt)) -- Route setup grant: provide the quest pelt without combat grinding.
                return "granted the turn-in item for the source-gated Spirit Eyes service"
            end,
        },
        {
            id = "turn_in_hell_hound_pelt",
            milestone = 7,
            expected = {
                scene = scene,
                group_items = { clear = { pelt } },
                facts = { [fact.SHAMAN_PELT_GIVEN] = true },
                flags = {
                    story = { set = { fact.SHAMAN_PELT_GIVEN } },
                    mode = {
                        set = { mode.BIT_18 },
                        clear = { mode.BIT_17 },
                    },
                    auxiliary = {
                        set = { auxiliary.SHAMAN_RITUAL_STAGE_2 },
                        clear = { auxiliary.SHAMAN_RITUAL_STAGE_1 },
                    },
                },
            },
            run = function(host)
                place_and_enter(host, scene, 27904, 2032,
                    "Scene-4 Spirit Eyes Hell Hound Pelt service", "DOWN")
                finish_spirit_eyes_conversation(host)
                return "Spirit Eyes consumed the pelt; the cave route is next"
            end,
        },
    },
}
