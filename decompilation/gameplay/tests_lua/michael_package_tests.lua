package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local Json = require("json")
local function load_json(path)
    local file = assert(io.open("decompilation/gameplay/" .. path .. ".json", "rb"))
    local document = Json.decode(file:read("*a"))
    file:close()
    return document
end

local MichaelPackage = require("michael_package")
local E = require("enums")
local content = {
    speakers = load_json("dialogue/speakers"),
    menus = load_json("dialogue/menus"),
    conditions = load_json("dialogue/conditions"),
    cinematics = load_json("dialogue/cinematics"),
    conversations = {
        stoker_motel = load_json("dialogue/conversations/stoker_motel"),
    },
}
local text_file = assert(io.open(
    "decompilation/assets/dialogue/script-text.json", "rb"))
local script_text = Json.decode(text_file:read("*a")).texts
text_file:close()

local michael_contents_pages = {
    {
        id = "michael-package-contents",
        key = "stoker.motel.michael-package.contents.title",
        text = [[<The creds roll off your stick and in return you are handed a crumpled plastic box.||Opening it you find|a scented letter,|a cyberdeck,|a wristband,|and two holopix.>]],
    },
    {
        id = "michael-package-letter",
        key = "stoker.motel.michael-package.letter.title",
        text = [[<The letter addressed to|Michael reads as follows:||  M--I don't trust our|Johnson, and neither|should you!  His story|just doesn't wash and you|know it.  I'm out on this|one and I hope you are|too.  I'll wait for you|at the Tarislar City Inn|in Puyallup Barrens.||      Tabatha Shale >]],
    },
    {
        id = "michael-package-cyberdeck",
        key = "stoker.motel.michael-package.cyberdeck.title",
        text = [[<The cyberdeck is a small Allegiance Alpha that hasn't seen much use.||These decks are fine to dabble in cyberspace but lack real power.>]],
    },
    {
        id = "michael-package-wristband",
        key = "stoker.motel.michael-package.wristband.title",
        text = [[The wristband is a DocWagon homing beacon.||When its wearer's condition becomes critical, a DocWagon Trauma Team is dispatched to retrieve him.||The wearer is then resuscitated and dropped off at Seattle General Hospital.]],
    },
    {
        id = "michael-package-fee",
        key = "stoker.motel.michael-package.fee.title",
        text = [[The fee is around 10% of the wearer's credstick balance, guaranteed not to exceed 5,000$.||You slip it on your wrist and lock the clasp.]],
    },
    {
        id = "michael-package-holopix-ivanna",
        key = "stoker.motel.michael-package.holopix-ivanna.title",
        text = [[|<Holopix>|||        Dr. Ivanna|     Haversheen, M.D.||         Seattle|     General Hospital||    Haversheen Clinic]],
    },
    {
        id = "michael-package-holopix-caleb",
        key = "stoker.motel.michael-package.holopix-caleb.title",
        text = [[|<Holopix>|||        Mr. Caleb|     Brightmore, Esq.||     Problems solved.||   Worries eliminated.]],
    },
    {
        id = "michael-package-credstick",
        key = "stoker.motel.michael-package.credstick.title",
        text = [[Finally, you uncover a credstick hidden under the holopix.||It has a balance of 500$!||The irony of this discovery wears off after a moment.]],
    },
}

local queued_choices, choices, pages, services = {}, {}, {}, {}
local service_results = {}
local function reset(...)
    queued_choices = { ... }
    choices, pages, services = {}, {}, {}
end

local function menu(menu_id, available, speaker_id, text_profile)
    assert(speaker_id == "stoker" and text_profile == "stoker.motel")
    local menu_data = content.menus.profiles[text_profile][menu_id]
    assert(menu_data ~= nil, "unmapped Stoker menu " .. menu_id)
    local labels = {}
    for _, id in ipairs(available) do
        local key = menu_data.choice_label_keys[id]
        assert(key ~= nil and script_text[key] ~= nil,
            "unmapped Stoker choice " .. id .. " in " .. menu_id)
        labels[#labels + 1] = script_text[key]
    end
    choices[#choices + 1] = { id = menu_id, options = available, labels = labels }
    local selected = table.remove(queued_choices, 1)
    assert(selected ~= nil, "unexpected Stoker menu " .. menu_id)
    for _, id in ipairs(available) do
        if id == selected then return selected end
    end
    error("choice " .. selected .. " is not available in " .. menu_id)
end

game = {
    modules = {world_content_data = {dynamic_world_activity_actors = {}}},
    content = content,
    state = { motel_bill_paid = false },
    location = { type = "motel", id = "stoker_motel" },
    field = { package_run_status = "none" },
    ui = {
        menu = menu,
        prompt_text_key = function()
            error("Stoker graph unexpectedly split inline prompt text")
        end,
        say = function(event_id, speaker_id, text_profile)
            local speaker = content.speakers.profiles[speaker_id]
            local event = speaker and speaker.events[event_id]
            assert(event ~= nil, "unknown Stoker event " .. event_id)
            assert(text_profile == "stoker.motel")
            local key = event.title_key
            assert(script_text[key] ~= nil, "missing Stoker text " .. key)
            pages[#pages + 1] = {
                id = event_id,
                speaker_id = speaker_id,
                portrait_alias = speaker.portrait_alias or "none",
                key = key,
                text = script_text[key],
                continuation_key = event.continuation_key,
            }
            return true
        end,
    },
    services = {
        call = function(domain, operation)
            services[#services + 1] = { domain = domain, operation = operation }
            local key = domain .. "." .. operation
            return service_results[key]
        end,
    },
}

for index, profile_id in ipairs(E.StokerMotelArrival.AMBUSH_PROFILE_IDS) do
    game.modules.world_content_data.dynamic_world_activity_actors[index] = {
        scene_id = E.Scene.STOKER_MOTEL,
        sequence_order = index - 1,
        sequence_id = E.WorldEffectSequence.STOKER_MOTEL_AMBUSH,
        selection_kind = "weighted_encounter",
        placement_rule =
            "open screen-edge candidates based on the current viewport and terrain",
        actor_name = index == 1 and "weenr2" or "weenr1",
        profile_id = profile_id,
        visual_profile_id = E.ActorVisualProfile.YAKUZA_WEENR,
        action_kind = index == 1 and "item" or "none",
        action_id = index == 1 and "streetline_special" or nil,
        actor_facts = {interaction = {}},
        physical_meter = index == 1 and 4 or 3,
        palette_line = 1,
        sprite_stem = "yakuza_weenr_k08_item0a",
        activity_probe_enabled = true,
        action_delay_modifier = 0,
        combat_cyberware = {},
        combat_mental_meter = 10,
        posture_reduction_percent = 0,
    }
end

local function outcome(result)
    assert(result.type == "continue_in_place" or
           result.type == "open_service" or
           result.type == "source_boundary",
        "unexpected Stoker outcome " .. tostring(result.type))
    return result
end

local function expect_page(index, event_id)
    assert(pages[index] ~= nil and pages[index].id == event_id,
        "expected Stoker page " .. event_id)
end

local function expect_michael_contents(first_index)
    for offset, expected in ipairs(michael_contents_pages) do
        local page = pages[first_index + offset - 1]
        assert(page ~= nil and page.id == expected.id,
            "expected Michael contents page " .. expected.id)
        assert(page.key == expected.key and page.text == expected.text,
            "wrong ROM text for Michael contents page " .. expected.id)
        local continuation_key =
            offset == #michael_contents_pages and "" or "shared.more"
        assert(page.continuation_key == continuation_key,
            "wrong ROM continuation for " .. expected.id)
    end
    -- 0x219C8 (callback 0x22) loads profile 0x121E68 for page 0xF46D1;
    -- 0x219B6 (callback 0x21) loads 0x121E98 for 0xF4746. Record 0xF4908
    -- (credstick 0xF47AA) has no callback, so Caleb's picture stays.
    local ivanna_page = pages[first_index + 5]
    assert(ivanna_page.speaker_id == "ivanna_haversheen" and
           ivanna_page.portrait_alias ==
               "dialogue.portraits.ivanna.haversheen",
        "Ivanna holopix page did not select the ROM portrait")
    local credstick_page = pages[first_index + 7]
    assert(credstick_page.portrait_alias ==
               "dialogue.portraits.caleb.brightmore",
        "credstick page must keep Caleb's 0x121E98 picture")
    for index = first_index, first_index + 4 do
        assert(pages[index].portrait_alias == "dialogue.portraits.stoker.motel",
            "pre-holopix package pages keep Stoker's portrait")
    end
    local caleb_page = pages[first_index + 6]
    assert(caleb_page.speaker_id == "caleb_brightmore" and
           caleb_page.portrait_alias ==
               "dialogue.portraits.caleb.brightmore",
        "Caleb holopix page did not select the ROM portrait")
end

-- Source gate and ordered A/B/C menu: the package choice is present while
-- FFFC1E trigger bit 0 is clear. There is no invented opening_dialogue_seen
-- save fact controlling the page sequence.
reset("crash", "pay")
service_results["building.charge_20"] = "paid"
local result = outcome(MichaelPackage.stoker_interaction())
assert(result.type == "open_service" and result.serviceId == "hotel.options" and
       result.returnPolicy == "origin")
assert(game.state.motel_bill_paid == false)
expect_page(1, "arrival")
expect_page(2, "room.paid")
assert(choices[1].id == "manager" and #choices[1].options == 3 and
       choices[1].options[1] == "crash" and
       choices[1].options[2] == "leave" and
       choices[1].options[3] == "package")
assert(choices[1].labels[3] == "I'm here to pick up Michael's stuff.")
assert(#services == 1 and services[1].domain == "building" and
       services[1].operation == "charge_20")

-- Room insufficient funds and both Leave edges stay on the same field map.
service_results["building.charge_20"] = "insufficient"
reset("crash", "pay")
result = outcome(MichaelPackage.stoker_interaction())
assert(result.type == "continue_in_place")
expect_page(2, "room.insufficient")

reset("leave")
result = outcome(MichaelPackage.stoker_interaction())
assert(result.type == "continue_in_place")
expect_page(2, "leave")

reset("crash", "leave")
result = outcome(MichaelPackage.stoker_interaction())
assert(result.type == "continue_in_place")
expect_page(2, "leave")

-- Michael branch: command 0x30 precedes the debt question, and command 0x05
-- selects Michael's eight-page contents sequence or Jump House low-funds page.
service_results["michael_package.prepare_debt_offer"] = "ready"
service_results["michael_package.pay_debt"] = "paid"
reset("package", "debt-payment")
result = outcome(MichaelPackage.stoker_interaction())
assert(result.type == "continue_in_place" and
       game.state.motel_bill_paid == true)
expect_michael_contents(2)
assert(#pages == 1 + #michael_contents_pages)
assert(services[1].operation == "prepare_debt_offer" and
       services[2].operation == "pay_debt")

reset()
result = outcome(MichaelPackage.present_package_handoff())
assert(result.type == "continue_in_place")
expect_michael_contents(1)
assert(#pages == #michael_contents_pages)

game.state.motel_bill_paid = false
service_results["michael_package.pay_debt"] = "insufficient"
reset("package", "debt-payment")
result = outcome(MichaelPackage.stoker_interaction())
assert(result.type == "continue_in_place" and
       game.state.motel_bill_paid == false)
expect_page(2, "jump-house")

service_results["michael_package.jump_house_referral"] = "ready"
reset("package", "jump-house")
result = outcome(MichaelPackage.stoker_interaction())
assert(result.type == "continue_in_place")
expect_page(2, "jump-house")
assert(services[2].operation == "jump_house_referral")

-- Unknown source facts and unavailable host transactions stop at explicit
-- boundaries rather than choosing a branch or claiming a successful return.
game.state.motel_bill_paid = nil
reset()
result = outcome(MichaelPackage.stoker_interaction())
assert(result.type == "source_boundary" and
       result.reason:find("story.motel_bill_paid", 1, true))
assert(#choices == 0 and #pages == 0 and #services == 0)

game.state.motel_bill_paid = false
service_results["building.charge_20"] = nil
reset("crash", "pay")
result = outcome(MichaelPackage.stoker_interaction())
assert(result.type == "source_boundary" and
       result.reason:find("wallet result", 1, true))

-- The scene-2/submode-2 selector keeps signed alternate results distinct.
game.field.package_run_status = "payment_due"
reset()
result = outcome(MichaelPackage.present_field_exit_prompt())
assert(result.type == "continue_in_place" and #choices == 0)
expect_page(1, "package-question")

game.field.package_run_status = "handoff_due"
reset()
result = outcome(MichaelPackage.present_field_exit_prompt())
assert(result.type == "continue_in_place" and #choices == 0)
expect_page(1, "package-handoff")
assert(pages[1].key == "stoker.motel.package-handoff.title" and
       pages[1].text ==
           "<Stoker squints at you with his one good eye.>||Here's the package.  Don't slot this up!")

game.field.package_run_status = "none"
service_results["random.bounded_100"] = "roll_79"
reset("leave")
result = outcome(MichaelPackage.present_field_exit_prompt())
assert(result.type == "continue_in_place")
expect_page(1, "arrival")
assert(choices[1].id == "arrival_manager" and
       #choices[1].options == 2)
expect_page(2, "leave")

service_results["random.bounded_100"] = "roll_80"
service_results["stoker_motel.spawn_sequence_" ..
                E.WorldEffectSequence.STOKER_MOTEL_AMBUSH] = "spawned"
reset()
result = outcome(MichaelPackage.present_field_exit_prompt())
assert(result.type == "continue_in_place")
expect_page(2, "street-ambush")
assert(services[1].domain == "random" and
       services[1].operation == "bounded_100" and
       services[2].domain == "stoker_motel" and
       services[2].operation == "spawn_sequence_" ..
           E.WorldEffectSequence.STOKER_MOTEL_AMBUSH)

service_results["random.bounded_100"] = "roll_99"
service_results["stoker_motel.spawn_sequence_" ..
                E.WorldEffectSequence.STOKER_MOTEL_AMBUSH] = "no_effect"
reset("leave")
result = outcome(MichaelPackage.present_field_exit_prompt())
assert(result.type == "continue_in_place")
expect_page(1, "arrival")
assert(choices[1].id == "arrival_manager")
expect_page(2, "leave")

service_results["random.bounded_100"] = nil
reset()
result = outcome(MichaelPackage.present_field_exit_prompt())
assert(result.type == "source_boundary" and
       result.reason:find("arrival command", 1, true))
expect_page(1, "arrival")

game.field.package_run_status = nil
reset()
result = outcome(MichaelPackage.present_field_exit_prompt())
assert(result.type == "source_boundary" and
       result.reason:find("field.package_run_status", 1, true))
assert(#pages == 0 and #choices == 0)

game.field.package_run_status = "none"
game.location = nil
reset()
result = outcome(MichaelPackage.present_field_exit_prompt())
assert(result.type == "source_boundary" and
       result.reason:find("precondition", 1, true))

print("Stoker dialogue graph permutations passed")
