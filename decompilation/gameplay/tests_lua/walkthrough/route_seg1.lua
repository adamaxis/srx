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
    tap(host, "A") -- Select Call Taxi from the physical terminal menu.
    assert(host.wait_for_taxi_selector(180),
        "physical terminal did not open the taxi selector")
    neutral_frames(host, 12)

    local current_cursor = assert(Taxi.region_to_cursor[current_region],
        "missing current taxi region")
    local target_cursor = assert(Taxi.region_to_cursor[destination_region],
        "missing destination taxi region")
    local left_steps = (target_cursor - current_cursor) % #Taxi.cursor_to_region
    for _ = 1, left_steps do tap(host, "LEFT") end

    tap(host, "A") -- Select the destination and call the cabbie.
    assert(host.wait_for_dialogue(120),
        "taxi destination did not open the cabbie dialogue")
    tap(host, "A") -- Accept the ride.
    assert(host.snapshot().dialogue_open,
        "cabbie did not present the fare after accepting the ride")
    tap(host, "A") -- Accept the quoted fare.
    assert(host.snapshot().dialogue_open,
        "cabbie did not present the ride page after the fare")
    wait_panel(host, "Hop in, chum.")
    tap(host, "A", 1, 60) -- Dismiss "Hop in, chum." and allow the cab handoff.
    assert(host.wait_for_scene(destination_scene, 180),
        "accepted taxi fare did not change to the destination scene")
    neutral_frames(host, 90)
end

local scene2 = E.Scene.STOKER_MOTEL
local scene1 = E.Scene.PUYALLUP_BARRENS
local story = E.StoryFact
local STOKER_DEBT_NUYEN = 250

return {
    resume_from_fixture_step = 2,
    steps = {
        {
            id = "new_game_reaches_stoker",
            milestone = 0,
            gate = true,
            expected = {
                scene = scene2,
                panel_actor = "Stoker",
                panel_text_contains = "battered sign reads Stoker's Coffin Motel",
            },
            run = function(host)
                wait_panel(host, "battered sign reads Stoker's Coffin Motel")
                return "production New Game auto-walk opened Stoker's source-gated entry"
            end,
        },
        {
            id = "stoker_pay_motel_bill",
            milestone = 1,
            gate = true,
            expected = {
                scene = scene2,
                panel_actor = "Stoker",
                panel_text_contains = "crumpled plastic box",
                -- The route grants 250, then the ROM nets another 250.
                nuyen_delta = STOKER_DEBT_NUYEN * 2,
                karma_delta = { 1, 1, 1 },
                facts = { [story.MOTEL_BILL_PAID] = true },
                flags = {
                    story = { set = { story.MOTEL_BILL_PAID } },
                    mode = {
                        set = {
                            E.FieldModeFlagBit.BIT_2,
                            E.FieldModeFlagBit.BIT_3,
                            E.FieldModeFlagBit.BIT_4,
                        },
                        clear = {
                            E.FieldModeFlagBit.BIT_0,
                            E.FieldModeFlagBit.BIT_1,
                        },
                    },
                },
                runner_features = {
                    set = { E.RunnerFeatureFlagBit.CYBERDECK_PRESENT },
                },
            },
            run = function(host)
                assert(host.grant_nuyen(STOKER_DEBT_NUYEN)) -- Route setup grant: cover Stoker's 250-nuyen debt payment.
                local funds = host.snapshot().nuyen
                assert(funds >= STOKER_DEBT_NUYEN,
                    "production New Game has " .. tostring(funds) ..
                    " nuyen; the source debt command requires 250 before it can set MOTEL_BILL_PAID")
                tap(host, "A") -- Advance the motel arrival to its manager menu.
                wait_panel(host, "The manager yawns.")
                tap(host, "C") -- Ask for Michael's package and the debt offer.
                wait_panel(host, "You pay up the 250$ he owes")
                tap(host, "A") -- Pay the source 250-nuyen debt.
                wait_panel(host, "crumpled plastic box")
                return "source payment sets the bill fact and cyberdeck-present flag; the ROM nets +250 nuyen"
            end,
        },
        {
            id = "read_michael_package_pages",
            milestone = 1,
            gate = true,
            expected = { scene = scene2 },
            run = function(host)
                local pages = {
                    "letter addressed to",
                    "Allegiance Alpha",
                    "DocWagon homing beacon",
                    "10% of the wearer's credstick balance",
                    "Dr. Ivanna",
                    "Mr. Caleb",
                    "credstick hidden under the holopix",
                }
                for _, text in ipairs(pages) do
                    tap(host, "A")
                    wait_panel(host, text)
                end
                tap(host, "A") -- Close the final source page.
                if host.snapshot().dialogue_open then
                    tap(host, "A") -- Advance the Caleb credstick page to the field.
                end
                local snapshot = host.snapshot()
                assert(not snapshot.dialogue_open,
                    "final Michael package page did not return to the field; " ..
                    "panel=" .. tostring(snapshot.panel_actor) .. "#" ..
                    tostring(snapshot.panel_page) .. ":" ..
                    tostring(snapshot.panel_text))
                return "all eight source package pages completed; runner item state remained unchanged"
            end,
        },
        {
            id = "taxi_redmond_to_puyallup",
            milestone = 1,
            gate = true,
            expected = {
                scene = scene1,
                nuyen_delta = -40,
            },
            run = function(host)
                ride_taxi(host, scene2, 29952, 2816,
                    Taxi.Region.REDMOND_BARRENS,
                    Taxi.Region.PUYALLUP_BARRENS, scene1)
                return "approached Redmond's physical terminal and paid the Puyallup cab fare"
            end,
        },
        {
            id = "tarislar_ask_shale",
            milestone = 2,
            gate = true,
            expected = {
                scene = scene1,
                panel_actor = "Inn Manager",
                panel_text_contains = "Tabatha Shale?  Yeah, she's here.",
                facts = { [story.TARISLAR_ASKED_SHALE] = true },
                flags = {
                    story = { set = { story.TARISLAR_ASKED_SHALE } },
                },
            },
            run = function(host)
                place_and_enter(host, scene1, 21760, 3328,
                    "Tarislar City Inn record 68")
                wait_panel(host, "The Tarislar City Inn caters mostly to elves")
                tap(host, "A") -- Advance the inn entry to its choice menu.
                tap(host, "C") -- Ask the manager about Tabatha Shale.
                wait_panel(host, "Tabatha Shale?  Yeah, she's here.")
                return "real record-68 contact selected Shale and set the named story fact"
            end,
        },
        {
            id = "finish_tabatha_shale_intro",
            milestone = 2,
            gate = true,
            expected = {
                scene = scene1,
                karma_delta = { 1, 1, 1 },
                flags = {
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
                tap(host, "A")
                wait_panel(host, "manager connects you to her room")
                tap(host, "A")
                wait_panel(host, "she concedes that you are Michael's brother")
                tap(host, "A") -- Source command 0x14 follows this MORE.
                wait_panel(host, "Tabatha Shale opens the door")
                tap(host, "B") -- No selection at the one-choice follow-up.
                assert(not host.snapshot().dialogue_open,
                    "no-selection did not return from Tabatha's intro")
                return "completed the source Shale acceptance pages and returned without selecting another topic"
            end,
        },
        {
            id = "taxi_puyallup_to_redmond",
            milestone = 2,
            gate = true,
            expected = {
                scene = scene2,
                nuyen_delta = -40,
            },
            run = function(host)
                ride_taxi(host, scene1, 1280, 3328,
                    Taxi.Region.PUYALLUP_BARRENS,
                    Taxi.Region.REDMOND_BARRENS, scene2)
                return "approached Puyallup's physical terminal and paid the Redmond cab fare"
            end,
        },
        {
            id = "reach_boris_greenhouse",
            milestone = 2,
            gate = true,
            expected = { scene = scene2 },
            run = function(host)
                place_and_enter(host, scene2, 21248, 1792,
                    "Boris's scene-2 event-6 service")
                wait_panel(host, "only illumination in the greenhouse")
                return "real Up input reached Boris's source event-6 service"
            end,
        },
        {
            id = "boris_reveal_michael_alias",
            milestone = 3,
            gate = true,
            expected = {
                scene = scene2,
                panel_actor = "Boris",
                panel_text_contains = "using the name \"Owlfeather\"",
                nuyen_delta = 0,
                karma_delta = { 1, 1, 1 },
                facts = { [story.MICHAEL_ALIAS_LEARNED] = true },
                flags = {
                    story = { set = { story.MICHAEL_ALIAS_LEARNED } },
                    mode = {
                        set = { E.FieldModeFlagBit.BIT_20 },
                        clear = {
                            E.FieldModeFlagBit.BIT_6,
                            E.FieldModeFlagBit.BIT_7,
                        },
                    },
                    dialogue = {
                        set = {
                            E.FieldDialogueRegisterFlagBit.BIT_1,
                            E.FieldDialogueRegisterFlagBit.BIT_2,
                        },
                    },
                },
            },
            run = function(host)
                assert(host.grant_nuyen(500)) -- Route setup grant: cover Boris's 500-nuyen information fee.
                tap(host, "A") -- Advance Boris's greeting to the service menu.
                tap(host, "B") -- Buy information, then choose the 500-nuyen offer.
                wait_panel(host, "I charge a flat fee of 500$")
                tap(host, "A")
                wait_panel(host, "Gives one a warm feeling all over")
                tap(host, "A") -- Choose Michael from the paid topics.
                wait_panel(host, "using the name \"Owlfeather\"")
                return "command 0x2B learned Michael's alias after the real $500 information transaction"
            end,
        },
    },
}
