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

local function neutral_frames(host, frames)
    local ok, problem = host.advance(frames)
    assert(ok, problem or "neutral frame advance failed")
end

local function wait_panel(host, text)
    for _ = 1, 900 do
        local snapshot = host.snapshot()
        if snapshot.panel_open and snapshot.panel_text:find(text, 1, true) then
            return
        end
        neutral_frames(host, 1)
    end
    local snapshot = host.snapshot()
    error("timed out waiting for dialogue panel text " .. string.format("%q", text) ..
        "; current actor=" .. tostring(snapshot.panel_actor) ..
        " panel=" .. string.format("%q", snapshot.panel_text) ..
            " options=" .. tostring(snapshot.panel_option_count) ..
            " selected=" .. tostring(snapshot.panel_selected_option) ..
            " choices=" .. table.concat(snapshot.panel_options or {}, "|") ..
        " story page=" .. tostring(snapshot.page_key))
end

local function place_and_enter(host, scene, x, y, description)
    local placed, placement_error = host.place_within_scene(scene, x, y)
    assert(placed, placement_error or "within-scene placement failed")
    local opened, movement_error = host.hold_until_modal("UP", 600)
    assert(opened, movement_error or
        ("real Up input did not reach " .. description))
end

local function ride_taxi(host, scene, x, y, current_region,
                         destination_region, destination_scene)
    place_and_enter(host, scene, x, y, "the field taxi terminal")
    tap(host, "A")
    assert(host.wait_for_taxi_selector(180),
        "physical terminal did not open the taxi selector")
    neutral_frames(host, 12)

    local current_cursor = assert(Taxi.region_to_cursor[current_region],
        "missing current taxi region")
    local target_cursor = assert(Taxi.region_to_cursor[destination_region],
        "missing destination taxi region")
    local left_steps = (target_cursor - current_cursor) % #Taxi.cursor_to_region
    for _ = 1, left_steps do tap(host, "LEFT") end

    tap(host, "A")
    assert(host.wait_for_dialogue(120),
        "taxi destination did not open the cabbie dialogue")
    tap(host, "A")
    assert(host.snapshot().dialogue_open,
        "cabbie did not present the fare after accepting the ride")
    tap(host, "A")
    assert(host.snapshot().dialogue_open,
        "cabbie did not present the ride page after the fare")
    wait_panel(host, "Hop in, chum.")
    tap(host, "A", 1, 60)
    assert(host.wait_for_scene(destination_scene, 180),
        "accepted taxi fare did not change to the destination scene")
    neutral_frames(host, 90)
end

local scene_salish = E.Scene.SALISH_SHIDHE
local scene_penumbra = E.Scene.PENUMBRA_DISTRICT
local scene_big_rhino = E.Scene.BIG_RHINO
local scene_mitsuhama_floor = E.Scene.RESOURCE_ACTIVITY_34
local kBigRhinoExitApproachX = 1024
local kBigRhinoExitApproachY = 3300
local kBigRhinoExitInputFrames = 600
local story = E.StoryFact
local mode = E.FieldModeFlagBit
local visa = E.GroupItemId.INDIAN_LANDS_VISA

return {
    resume_from_fixture_step = 1,
    steps = {
        {
            id = "grant_salish_visa_for_story_travel",
            milestone = 0,
            expected = { scene = E.Scene.STOKER_MOTEL,
                group_items = { set = { visa } } },
            run = function(host)
                assert(host.grant_group_item(visa)) -- Route setup grant: provide the Indian Lands visa required for Salish travel.
                return "seeded the travel-only visa while retaining production story state"
            end,
        },
        {
            id = "taxi_redmond_to_salish_shidhe",
            milestone = 0,
            expected = {
                scene = scene_salish,
                nuyen_delta = -100,
                group_items = { clear = { visa } },
            },
            run = function(host)
                ride_taxi(host, E.Scene.STOKER_MOTEL, 29952, 2816,
                    Taxi.Region.REDMOND_BARRENS,
                    Taxi.Region.SALISH_SHIDHE, scene_salish)
                return "used Redmond's physical terminal and paid the Salish fare"
            end,
        },
        {
            id = "open_owlfeather_first_contact_service",
            milestone = 3,
            expected = {
                scene = scene_salish,
                panel_actor = "David Owlfeather",
                panel_text_contains = "D. Owlfeather",
                flags = {
                    mode = { clear = { mode.BIT_8, mode.BIT_9 } },
                },
            },
            run = function(host)
                place_and_enter(host, scene_salish, 21760, 20224,
                    "the Owlfeather lodge service record")
                wait_panel(host, "How can I help you this fine day?")
                return "real scene-6 event-0 service opened Owlfeather's contact menu"
            end,
        },
        {
            id = "ask_owlfeather_about_michael",
            milestone = 3,
            expected = {
                scene = scene_salish,
                karma_delta = { 1, 1, 1 },
                facts = { [story.OWLFEATHER_ASKED] = true },
                flags = {
                    story = { set = { story.OWLFEATHER_ASKED } },
                    mode = { clear = { mode.BIT_8, mode.BIT_9 } },
                },
            },
            run = function(host)
                tap(host, "C")
                wait_panel(host, "You pull his holopix from your pocket")
                return "choice C reached command 0x17 and set the named Owlfeather fact"
            end,
        },
        {
            id = "complete_owlfeather_aragorn_task_briefing",
            milestone = 3,
            expected = {
                scene = scene_salish,
                flags = { mode = { set = { mode.BIT_10 } } },
            },
            run = function(host)
                local pages = {
                    "I am sorry to hear of another senseless death",
                    "I have not made much progress",
                    "After his death, Tabatha gave me many leads",
                    "If you require his help, I must first ask for yours",
                    "Ironically, my own brother, Aragorn, left our land",
                }
                for _, text in ipairs(pages) do
                    tap(host, "A")
                    wait_panel(host, text)
                end
                tap(host, "C") -- Thanks ends the task briefing without entering another topic.
                wait_panel(host, "Sure thing.")
                tap(host, "A")
                wait_panel(host, "Farewell, and may the spirits guide you.")
                tap(host, "A")
                assert(not host.snapshot().dialogue_open,
                    "Owlfeather task briefing did not return to the field")
                return "completed the ROM page sequence and left the service in place"
            end,
        },
        {
            id = "taxi_salish_to_penumbra",
            milestone = 3,
            expected = { scene = scene_penumbra, nuyen_delta = -20 },
            run = function(host)
                ride_taxi(host, scene_salish, 7424, 2304,
                    Taxi.Region.SALISH_SHIDHE,
                    Taxi.Region.PENUMBRA_DISTRICT, scene_penumbra)
                return "paid the Penumbra fare at Salish's physical terminal"
            end,
        },
        {
            id = "enter_big_rhino_and_open_mortimer",
            milestone = 3,
            expected = {
                scene = scene_big_rhino,
                panel_actor = "Mortimer Reed",
                panel_text_contains = "You move into the booth",
            },
            run = function(host)
                local placed, placement_error = host.place_within_scene(
                    scene_penumbra, 9984, 9472)
                assert(placed, placement_error or "Big Rhino placement failed")
                local entered, movement_error = host.hold_until_modal(
                    "UP", 600, scene_big_rhino)
                assert(entered, movement_error or
                    "Penumbra record 98 did not enter Big Rhino")
                assert(host.wait_for_scene(scene_big_rhino, 180),
                    "Big Rhino event-table row 5 did not enter Scene 11")
                place_and_enter(host, scene_big_rhino, 4608, 1536,
                    "the source Mortimer booth event")
                -- ROM 0x209BE arms FFF112 on every Scene 11 visit: the
                -- booth search 0x105544 always precedes root 0x105558.
                wait_panel(host, "You move into the booth")
                return "physical record-98 entry reached Mortimer's scene-11 booth"
            end,
        },
        {
            id = "ask_mortimer_about_aragorn",
            milestone = 3,
            expected = {
                scene = scene_big_rhino,
                panel_actor = "Mortimer Reed",
                panel_text_contains = "fragged a solo run against Mitsuhama",
                flags = {
                    mode = {
                        set = { mode.BIT_12 },
                        clear = { mode.BIT_11 },
                    },
                },
            },
            run = function(host)
                tap(host, "A") -- Advance the first ROM booth-entry page.
                wait_panel(host, "Well, what have we here")
                tap(host, "A") -- Finish the root page text.
                tap(host, "A") -- Information from Mortimer's root menu.
                wait_panel(host, "However, I do know a valuable contact")
                tap(host, "A") -- Finish the information opening.
                tap(host, "C") -- Open the questions branch from the topic menu.
                local question_menu = host.snapshot()
                assert(question_menu.panel_option_count == 2 and
                    question_menu.panel_options[1]:find("Mako", 1, true) and
                    question_menu.panel_options[2]:find("Aragorn", 1, true),
                    "Mortimer's live question choices were not Mako then Aragorn")
                tap(host, "B") -- Select the verified second option, Aragorn.
                wait_panel(host, "fragged a solo run against Mitsuhama")
                return "Mortimer's source question set field-mode lead bit 12"
            end,
        },
        {
            id = "leave_mortimer_and_big_rhino",
            milestone = 3,
            expected = { scene = scene_penumbra },
            run = function(host)
                tap(host, "B") -- Different subject returns to the information categories.
                wait_panel(host, "I could impart you with the name for 1500$")
                tap(host, "C") -- Back to Mortimer's opening.
                wait_panel(host, "You're the perp.")
                tap(host, "C") -- Leave.
                wait_panel(host, "I'll check you later")
                tap(host, "A")
                assert(not host.snapshot().dialogue_open,
                    "Mortimer's leave page did not close")

                -- Scene-11 records 8/9 (0x1D5AF8/0x1D5B02, code 0x0080)
                -- span x 512..1535 and y 3584..4095; enter from below with
                -- real Down input.
                local placed, placement_error = host.place_within_scene(
                    scene_big_rhino, kBigRhinoExitApproachX,
                    kBigRhinoExitApproachY)
                assert(placed, placement_error or "scene-11 exit placement failed")
                local exited, exit_error = host.hold_until_modal(
                    "DOWN", kBigRhinoExitInputFrames, scene_penumbra)
                assert(exited, exit_error or "Big Rhino field return did not open")
                assert(host.wait_for_scene(scene_penumbra, 180),
                    "Big Rhino source exit did not return to Penumbra")
                return "used Scene-11 event 8's physical field return"
            end,
        },
        {
            id = "grant_salish_visa_for_mitsuhama_return",
            milestone = 3,
            expected = {
                scene = scene_penumbra,
                group_items = { set = { visa } },
            },
            run = function(host)
                assert(host.grant_group_item(visa)) -- Route setup grant: restore the visa consumed by the first Salish cab fare.
                return "restored the travel-only visa for the second Salish visit"
            end,
        },
        {
            id = "taxi_penumbra_to_salish",
            milestone = 3,
            expected = {
                scene = scene_salish,
                nuyen_delta = -100,
                group_items = { clear = { visa } },
            },
            run = function(host)
                ride_taxi(host, scene_penumbra, 3328, 3840,
                    Taxi.Region.PENUMBRA_DISTRICT,
                    Taxi.Region.SALISH_SHIDHE, scene_salish)
                return "returned by physical cab to Salish after the Mortimer lead"
            end,
        },
        {
            id = "enter_after_hours_mitsuhama",
            milestone = 4,
            expected = {
                scene = E.Scene.RESOURCE_ACTIVITY_30,
                panel_text_contains = "Hopefully Aragorn is still here",
            },
            run = function(host)
                place_and_enter(host, scene_penumbra, 11520, 3072,
                    "Penumbra record 95 after-hours Mitsuhama entry")
                wait_panel(host, "Hopefully Aragorn is still here")
                tap(host, "A")
                assert(host.wait_for_scene(E.Scene.RESOURCE_ACTIVITY_30, 180),
                    "Mitsuhama after-hours row 3 did not enter resource scene 30")
                return "source-gated scene-3 event-3 entry reached Mitsuhama activity scene 30"
            end,
        },
        {
            id = "ride_mitsuhama_elevator_to_scene_34",
            milestone = 4,
            expected = { scene = scene_mitsuhama_floor },
            run = function(host)
                local placed, placement_error = host.place_within_scene(
                    E.Scene.RESOURCE_ACTIVITY_30, 14592, 17152)
                assert(placed, placement_error or "scene-30 lift placement failed")
                local opened, movement_error = host.hold_until_modal("UP", 600)
                assert(opened, movement_error or "Mitsuhama lift did not open")
                for _ = 1, 4 do tap(host, "UP") end
                tap(host, "A")
                neutral_frames(host, 140)
                tap(host, "START")
                assert(host.wait_for_scene(scene_mitsuhama_floor, 180),
                    "Mitsuhama elevator did not reach floor 4 / scene 34")
                return "used the source elevator selector from scene 30 to scene 34"
            end,
        },
        {
            id = "contact_aragorn_at_mitsuhama",
            milestone = 4,
            expected = {
                scene = scene_mitsuhama_floor,
                panel_text_contains = "Walking into the room",
                facts = { [story.ARAGORN_FOUND] = true },
                flags = {
                    story = { set = { story.ARAGORN_FOUND } },
                    mode = {
                        set = { mode.BIT_13 },
                        clear = { mode.BIT_10, mode.BIT_11, mode.BIT_12 },
                    },
                    auxiliary = { clear = { mode.BIT_4, mode.BIT_5 } },
                },
            },
            run = function(host)
                local placed, placement_error = host.place_within_scene(
                    scene_mitsuhama_floor, 7424, 16640)
                assert(placed, placement_error or "Aragorn contact placement failed")
                neutral_frames(host, 30)
                wait_panel(host, "Walking into the room")
                return "the live Mitsuhama world actor opened the source contact dialogue"
            end,
        },
        {
            id = "finish_aragorn_mitsuhama_contact",
            milestone = 4,
            expected = { scene = scene_mitsuhama_floor },
            run = function(host)
                tap(host, "A")
                wait_panel(host, "get you outta' here")
                tap(host, "A")
                wait_panel(host, "runs from the room")
                tap(host, "A")
                neutral_frames(host, 32)
                assert(not host.snapshot().dialogue_open,
                    "Aragorn contact sequence did not return to Scene 34")
                return "completed Aragorn's real actor contact and departure sequence"
            end,
        },
        {
            id = "return_to_scene_30_from_mitsuhama_lift",
            milestone = 4,
            expected = { scene = E.Scene.RESOURCE_ACTIVITY_30 },
            run = function(host)
                local placed, placement_error = host.place_within_scene(
                    scene_mitsuhama_floor, 14336, 1536)
                assert(placed, placement_error or "scene-34 lift placement failed")
                local opened, movement_error = host.hold_until_modal("UP", 600)
                assert(opened, movement_error or "scene-34 lift did not open")
                for _ = 1, 4 do tap(host, "DOWN") end
                tap(host, "A")
                neutral_frames(host, 140)
                tap(host, "START")
                assert(host.wait_for_scene(E.Scene.RESOURCE_ACTIVITY_30, 180),
                    "Mitsuhama elevator did not return to scene 30")
                return "reverse source elevator trip returned to scene 30"
            end,
        },
        {
            id = "return_from_mitsuhama_to_field",
            milestone = 4,
            expected = { scene = scene_penumbra },
            run = function(host)
                local placed, placement_error = host.place_within_scene(
                    E.Scene.RESOURCE_ACTIVITY_30, 7936, 19712)
                assert(placed, placement_error or "scene-30 return placement failed")
                local exited, exit_error = host.hold_until_modal("DOWN", 600)
                assert(exited, exit_error or "scene-30 source return did not open")
                assert(host.wait_for_scene(scene_penumbra, 180),
                    "Mitsuhama source return did not restore Penumbra")
                return "source activity return restored the originating Penumbra map"
            end,
        },
        {
            id = "grant_salish_visa_for_owlfeather_return",
            milestone = 4,
            expected = {
                scene = scene_penumbra,
                group_items = { set = { visa } },
            },
            run = function(host)
                assert(host.grant_group_item(visa)) -- Route setup grant: provide the visa consumed by the return ride to Salish.
                return "restored the travel-only visa for Owlfeather's return service"
            end,
        },
        {
            id = "taxi_penumbra_to_salish_for_return_visit",
            milestone = 4,
            expected = {
                scene = scene_salish,
                nuyen_delta = -100,
                group_items = { clear = { visa } },
            },
            run = function(host)
                ride_taxi(host, scene_penumbra, 3328, 3840,
                    Taxi.Region.PENUMBRA_DISTRICT,
                    Taxi.Region.SALISH_SHIDHE, scene_salish)
                return "paid the Salish return fare after Aragorn's contact"
            end,
        },
        {
            id = "owlfeather_return_service_sets_bit_4",
            milestone = 5,
            expected = {
                scene = scene_salish,
                karma_delta = { 2, 2, 2 },
                facts = { [story.ARAGORN_FREED_TOLD] = true },
                flags = {
                    story = { set = { story.ARAGORN_FREED_TOLD } },
                    mode = {
                        set = { mode.BIT_14 },
                        clear = {
                            mode.BIT_8, mode.BIT_9, mode.BIT_10, mode.BIT_13,
                        },
                    },
                },
            },
            run = function(host)
                place_and_enter(host, scene_salish, 21760, 20224,
                    "the Owlfeather return service")
                wait_panel(host, "Still no luck, eh?")
                tap(host, "A") -- Freed selects Owlfeather's return service.
                wait_panel(host, "The station doors swing wide")
                return "return service applied the source Karma award and set Aragorn's returned fact"
            end,
        },
        {
            id = "finish_owlfeather_reunion",
            milestone = 5,
            expected = { scene = scene_salish },
            run = function(host)
                local pages = {
                    "Owlfeather quickly clasps Aragorn",
                    "Brother, welcome back to our land",
                    "You must forgive him",
                    "Go to the Medicine Lodge",
                }
                for _, text in ipairs(pages) do
                    tap(host, "A")
                    wait_panel(host, text)
                end
                tap(host, "A")
                assert(not host.snapshot().dialogue_open,
                    "Owlfeather reunion did not return to the field")
                return "completed the reunion pages and reached the Spirit Eyes lead"
            end,
        },
    },
}
