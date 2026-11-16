-- Same-map hospital service (ROM 0x1F830 -> 0x1F246 -> 0xC928), shared by
-- every site in data.hospital_sites (Council Island Hospital 4/4, Little
-- Chiba's 2/1). Each site's handler selects one of three text roots from the
-- alternate-result byte; the default root offers Hospital Options or leave.
-- No site loads a hospital interior.
local Sites = require("data.hospital_sites")
local E = require("enums")
local StoryFacts = require("story_facts")
local GroupItems = require("group_items")
local DialogueActions = require("dialogue_actions")
local Hospital = {}
local EFFECTS = DialogueActions.new_default_registry()

-- The active site's narrator/profile; set by assert_hospital_field_context.
local NARRATOR, TEXT_PROFILE

local function run_cursor_menu(menu_id, choices, initial_selection,
                               inline_prompt, ignore_cancel)
    local selected = initial_selection
    while true do
        local input
        if inline_prompt ~= nil then
            input = game.ui.cursor_menu(
                menu_id, choices, selected, NARRATOR, TEXT_PROFILE,
                inline_prompt)
        else
            input = game.ui.cursor_menu(
                menu_id, choices, selected, NARRATOR, TEXT_PROFILE)
        end
        if input == "up" and #choices > 0 then
            selected = (selected - 2) % #choices + 1
        elseif input == "down" and #choices > 0 then
            selected = selected % #choices + 1
        elseif input == "confirm" or input == "page" then
            local choice = choices[selected]
            if type(choice) == "table" then
                return choice.id
            elseif choice ~= nil then
                return choice
            end
        elseif input == "menu_cancel" then
            return input, selected
        elseif input == "back" or (input == "cancel" and not ignore_cancel) then
            return input, selected
        end
    end
end

local function ceil_thirds_minus_one(value)
    return math.floor((value + 2) / 3) - 1
end

local function medical_attention_value(runner, base_service_level)
    local service_level = base_service_level
    if E.RunnerClass.CASTERS[runner.character_class] then
        service_level = service_level + 2
    end
    service_level = service_level -
        ceil_thirds_minus_one(runner.attributes.body) -
        ceil_thirds_minus_one(runner.attributes.willpower)
    service_level = math.max(service_level, 2)

    local tier_delta = service_level - 7
    local factor = tier_delta >= 0 and 10 or -tier_delta * 20
    local scaled_body = runner.attributes.body * factor
    local amount = math.floor(scaled_body / 100)
    if scaled_body % 100 >= 50 then
        amount = amount + 1
    end
    return math.max(amount, 1)
end

local function source_price_increment(dividend, medical_value,
                                      round_threshold, minimum_units)
    local units = math.floor(dividend / medical_value)
    if dividend % medical_value >= round_threshold then
        units = units + 1
    end
    units = math.max(units, minimum_units)
    return units * 10
end

local function medical_attention_choices(runner)
    local physical = runner.physical
    if physical == 9 or physical >= 10 then
        return {}
    end

    -- ROM 0xCAAC..0xCAD4 stores the base-4, base-6, and base-8 values in
    -- FFF0C0/FFF0C2/FFF0C4 as recovery permits. 0xCAE2..0xCB7C converts
    -- those values into ten-nuyen increments, including its three distinct
    -- remainder thresholds and the 20/30-nuyen lower bounds.
    local increment_100 = source_price_increment(
        10, medical_attention_value(runner, 4), 0xAAAA, 0)
    local increment_70
    local increment_40
    if physical < 7 then
        increment_70 = source_price_increment(
            15, medical_attention_value(runner, 6), 7, 2)
    end
    if physical < 4 then
        increment_40 = source_price_increment(
            20, medical_attention_value(runner, 8), 10, 3)
    end

    local total = 20 + increment_100 + (increment_70 or 0) +
        (increment_40 or 0)
    local rows = {
        {
            id = "level_100",
            label = string.format("%d", total),
            price = total,
            physical = 10,
            label_key = "hospital.medical.row.100",
        },
        {
            id = "level_90",
            label = string.format("%d", increment_100 +
                (increment_70 or 0) + (increment_40 or 0)),
            price = increment_100 + (increment_70 or 0) +
                (increment_40 or 0),
            physical = 9,
            label_key = "hospital.medical.row.90",
        },
    }
    if increment_70 ~= nil then
        rows[#rows + 1] = {
            id = "level_70",
            label = string.format("%d", increment_70 +
                (increment_40 or 0)),
            price = increment_70 + (increment_40 or 0),
            physical = 7,
            label_key = "hospital.medical.row.70",
        }
    end
    if increment_40 ~= nil then
        rows[#rows + 1] = {
            id = "level_40",
            label = string.format("%d", increment_40),
            price = increment_40,
            physical = 4,
            label_key = "hospital.medical.row.40",
        }
    end
    return rows
end

local function assert_hospital_field_context()
    local site = game.location.type == "building" and
        Sites[game.location.id] or nil
    assert(site ~= nil, "hospital prompt ran outside a linked hospital site")
    NARRATOR, TEXT_PROFILE = site.narrator, site.text_profile
    return site
end

local function entry_text(site)
    for _, variant in ipairs(site.entry_text_variants or {}) do
        if game.state[variant.fact] == true then
            return variant.text
        end
    end
    return site.entry_text
end

local function apply_action(action_id, arguments)
    local ok, result, failure = EFFECTS:execute(action_id, arguments)
    assert(ok and result == nil,
        failure or ("hospital script action failed: " .. action_id))
end

-- Seattle General Hospital (ROM handler 0x1F830). Every page of this handler
-- uses Ivanna Haversheen's portrait (a5 = 0x121E68 at 0x1F848).
local function seattle_say(site, event_id)
    game.ui.say(event_id, site.entry_speaker, site.entry_text_profile)
end

local function seattle_prompt(site, menu_id, text_key, choices)
    return game.ui.prompt_text_key(menu_id, text_key, choices,
                                   site.entry_speaker,
                                   site.entry_text_profile)
end

local function seattle_choice_result(site, choice)
    if choice == "services" then
        -- Command 0x3D at 0x21EE8 sets transient FFF0D5 = 1 and selects
        -- page 0x111916 (text 0x11191E) before Hospital Options.
        seattle_say(site, "seattle_hospital_services")
        return "hospital-options"
    elseif choice == "leave" or choice == "later" then
        -- Both leave labels (0x105101, 0x104FEB) target page 0x110FE2
        -- (text 0x112323).
        seattle_say(site, "seattle_hospital_farewell")
    end
    return "field_return"
end

-- Pages 0x111042, 0x1110D2, 0x110A46 and 0x110FCA end on the same pair:
-- 0x1044F2 -> command 0x3D, 0x104FEB -> page 0x110FE2.
local function seattle_closing_offer(site, text_key)
    return seattle_choice_result(site, seattle_prompt(
        site, "seattle_hospital_closing_offer", text_key,
        { "services", "later" }))
end

-- ROM 0x1F8AE..0x1F8CC and command 0x61 at 0x2265E..0x2267C apply the same
-- writes: FFFC34 bit 18 clear and bit 19 set (Notebook tips "Stark is dying"
-- -> "Stark's heart is failing"), then FFDD64 = 0x19 and 0x2196C set story
-- bit 25.
local function record_stark_admitted()
    apply_action("field.set_mode_effect_flag", {
        flagBit = E.FieldModeEffectFlagBit.STARK_DYING_TIP,
        value = false,
    })
    apply_action("field.set_mode_effect_flag", {
        flagBit = E.FieldModeEffectFlagBit.STARK_HEART_FAILING_TIP,
        value = true,
    })
    apply_action("story.set_fact", {
        factId = E.StoryFact.HOSPITAL_VISITED,
        value = true,
    })
end

-- Pages 0x11106E..0x1110D2, reached through the "I've found Stark" label
-- 0x10417A. None of these records carries a command.
local function seattle_stark_rescue(site)
    seattle_say(site, "seattle_hospital_stark_alive")        -- 0x11106E
    if seattle_prompt(site, "seattle_hospital_stark_waiting_room",
                      "seattle_general_hospital.stark.rescued",
                      { "wait" }) ~= "wait" then      -- 0x111082
        return "field_return"
    end
    seattle_say(site, "seattle_hospital_stark_examined")     -- 0x111096
    if seattle_prompt(site, "seattle_hospital_stark_whatever_it_takes",
                      "seattle_general_hospital.stark.hope",
                      { "agree" }) ~= "agree" then    -- 0x1110AA
        return "field_return"
    end
    seattle_say(site, "seattle_hospital_stark_request")      -- 0x1110BE
    return seattle_closing_offer(site,
        "seattle_general_hospital.stark.cyberheart")         -- 0x1110D2
end

local function seattle_stark_found_offer(site, text_key)
    if seattle_prompt(site, "seattle_hospital_stark_found", text_key,
                      { "stark_found" }) ~= "stark_found" then
        return "field_return"
    end
    return seattle_stark_rescue(site)
end

-- Command 0x3E at 0x21EF8 and the pages it selects.
local function seattle_show_holopix(site)
    -- 0x21EF8..0x21F12: FFFC1E bit 23 set, FFFC30 bit 3 clear.
    apply_action("story.set_fact", {
        factId = E.StoryFact.HOLOPIX_SHOWN,
        value = true,
    })
    apply_action("field.set_mode_flag", {
        flagBit = E.FieldModeFlagBit.SEATTLE_HOLOPIX_RECOGNITION,
        value = false,
    })
    seattle_say(site, "seattle_hospital_holopix_shown")      -- 0x111006
    -- Page 0x11101A: text 0x111D21 with the single label 0x104545, whose
    -- target is command 0x61.
    if seattle_prompt(site, "seattle_hospital_stark_question",
                      "seattle_general_hospital.michael.remembrance",
                      { "ask_stark" }) ~= "ask_stark" then
        return "field_return"
    end
    -- Command 0x61 at 0x22648 tests FFFC1E bit 24.
    if StoryFacts.is_set(E.StoryFact.MEDICINE_LODGE_TURNED_AWAY) ~= true then
        return seattle_closing_offer(site,
            "seattle_general_hospital.stark.best_friend")    -- 0x111042
    end
    record_stark_admitted()
    return seattle_stark_found_offer(site,
        "seattle_general_hospital.stark.best_friend")        -- 0x11102E
end

-- ROM 0x1F8EA..0x1F954: the prototype cyber-heart turn-in.
local function seattle_heart_turn_in(site)
    if GroupItems.has(E.GroupItemId.PROTOTYPE_CYBERHEART) ~= true then
        return seattle_closing_offer(site,
            "seattle_general_hospital.heart.missing")        -- 0x110A46
    end
    -- 0x1F8F8: FFFC22 bit 0 clear (the heart is handed over).
    apply_action("item.set_group_item", {
        itemId = E.GroupItemId.PROTOTYPE_CYBERHEART,
        value = false,
    })
    -- 0x1F908: FFFC2A bit 9 set (Notebook shadowrunner record 9, Stark).
    game.state["runner_information_bit_" ..
        E.NotebookShadowrunnerBit.STARK] = true
    -- 0x1F918: FFFC34 bit 20 clear ("You have the prototype cyber-heart").
    apply_action("field.set_mode_effect_flag", {
        flagBit = E.FieldModeEffectFlagBit.CYBERHEART_CARRIED_TIP,
        value = false,
    })
    -- 0x1F928..0x1F930: FFFC38 bits 2, 3 and 11 clear (the Fuchi
    -- cyber-heart memos).
    for _, flag_bit in ipairs({
        E.NotebookTipFlagBit.FUCHI_HEART_TRANSPORT_MEMO,
        E.NotebookTipFlagBit.FUCHI_HEART_STORAGE_MEMO,
        E.NotebookTipFlagBit.FUCHI_HEART_ESCORT_MEMO,
    }) do
        apply_action("field.set_notebook_tip_flag", {
            flagBit = flag_bit,
            value = false,
        })
    end
    -- 0x1F940: FFFC1E bit 29 set.
    apply_action("story.set_fact", {
        factId = E.StoryFact.HEART_DELIVERED,
        value = true,
    })
    seattle_say(site, "seattle_hospital_heart_received")     -- 0x110A5E
    seattle_say(site, "seattle_hospital_stark_recovered")    -- 0x110FA2
    seattle_say(site, "seattle_hospital_stark_walked_out")   -- 0x110FB6
    return seattle_closing_offer(site,
        "seattle_general_hospital.heart.underground_93")     -- 0x110FCA
end

-- ROM 0x1F878..0x1F8E0 selects the Seattle page once FFF0D5 is zero.
local function present_seattle_entry(site)
    if game.state["field_mode_flag_bit_" ..
            E.FieldModeFlagBit.SEATTLE_HOLOPIX_RECOGNITION] == true then
        -- 0x1F886: page 0x110A72 (text 0x111A7B) -> 0x110FEA.
        seattle_say(site, "seattle_hospital_entry_michael_holopix")
        local choice = seattle_prompt(site, "seattle_hospital_holopix_offer",
            "seattle_general_hospital.prompt.help",
            { "show_holopix", "services", "leave" })
        if choice == "show_holopix" then
            return seattle_show_holopix(site)
        end
        return seattle_choice_result(site, choice)
    end

    local holopix_shown = StoryFacts.is_set(E.StoryFact.HOLOPIX_SHOWN) == true
    local stark_found =
        StoryFacts.is_set(E.StoryFact.MEDICINE_LODGE_TURNED_AWAY) == true
    local stark_admitted =
        StoryFacts.is_set(E.StoryFact.HOSPITAL_VISITED) == true
    if holopix_shown and stark_found then
        if not stark_admitted then
            -- 0x1F8AE..0x1F8D0: page 0x11105A (text 0x11199C).
            record_stark_admitted()
            return seattle_stark_found_offer(site,
                "seattle_general_hospital.entry.default")
        elseif StoryFacts.is_set(E.StoryFact.HEART_DELIVERED) ~= true then
            return seattle_heart_turn_in(site)              -- 0x1F8E4
        end
    end

    -- 0x1F8DA: page 0x110A32 (text 0x11199C) -> 0x110F8A.
    seattle_say(site, "seattle_hospital_entry_default")
    return seattle_choice_result(site, seattle_prompt(
        site, "seattle_hospital_service_offer",
        "seattle_general_hospital.prompt.help", { "services", "leave" }))
end

function Hospital.present_field_exit_prompt()
    local site = assert_hospital_field_context()

    local package_run_status = game.field.package_run_status
    if package_run_status == E.PackageRunStatus.HANDOFF_DUE then
        if site.package_handoff_event ~= nil then
            game.ui.say(site.package_handoff_event,
                        site.package_result_speaker, TEXT_PROFILE)
        else
            game.ui.say("package_handoff")
        end
        return "field_return"
    elseif package_run_status == E.PackageRunStatus.PAYMENT_DUE then
        if site.package_payment_event ~= nil then
            game.ui.say(site.package_payment_event,
                        site.package_result_speaker, TEXT_PROFILE)
        else
            game.ui.say("package_payment")
        end
        return "field_return"
    end

    if site.entry_speaker ~= nil then
        return present_seattle_entry(site)
    end

    local choice = game.ui.prompt_text_key(
        "hospital_service_offer",
        entry_text(site),
        { "services", "leave" })
    if choice == "services" then
        return "hospital-options"
    end
    return "field_return"
end

function Hospital.present_options()
    assert_hospital_field_context()

    local selected = 1
    while true do
        local choice
        choice, selected = run_cursor_menu("hospital_options", {
            "medical_attention", "cyberware", "exit",
        }, selected, nil, true)
        if choice == "menu_cancel" then
            return "field_return"
        elseif choice == "back" then
            -- ROM 0xC9EA calls 0x391A and returns to the same menu loop with
            -- its cursor unchanged. Run the shared typed host action, then
            -- reopen the Lua menu at that row. Start remains ignored by the
            -- source menu loop.
            assert(game.services.call("field", "mode_action") == "applied",
                "hospital runner-selection service is unavailable")
        elseif choice == "medical_attention" then
            return "hospital_medical_attention_requested"
        elseif choice == "cyberware" then
            return "hospital_cyberware_requested"
        elseif choice == "exit" then
            return "hospital_leave_confirmation_requested"
        else
            return "field_return"
        end
    end
end

function Hospital.present_medical_attention()
    assert_hospital_field_context()

    local party = game.party()
    local runner = party[game.active_slot()]
    assert(runner ~= nil and runner.attributes ~= nil,
        "hospital medical attention requires an active runner record")
    local funds = game.nuyen()
    assert(type(funds) == "number",
        "hospital medical attention requires the field wallet")

    if runner.physical == 10 and runner.stun >= 10 then
        game.ui.say("medical_attention_not_needed")
        return "hospital-options"
    end

    local choices = medical_attention_choices(runner)
    local prompt = string.format(
        " Choose level of|medical attention.|$%d", funds)
    local choice_id = run_cursor_menu(
        "hospital_medical_attention", choices,
        #choices > 0 and 1 or 0, prompt, true)
    if choice_id == "back" then
        return "hospital-options"
    end
    if choice_id == "cancel" then
        return "hospital-options"
    end
    if choice_id == "menu_cancel" then
        return "hospital-options"
    end

    local selected
    for _, choice in ipairs(choices) do
        if choice.id == choice_id then
            selected = choice
            break
        end
    end
    if selected == nil then
        return "hospital-options"
    end

    if funds < selected.price then
        game.ui.say("medical_attention_insufficient_funds")
        return "hospital-options"
    end

    game.set_nuyen(funds - selected.price)
    runner.physical = selected.physical
    runner.stun = 10
    game.commit_party(party)
    return "hospital-options"
end

function Hospital.present_leave_confirmation()
    assert_hospital_field_context()

    -- ROM 0xCE7C initializes the Yes/No selector on No. B, A, and C accept;
    -- Start does not accept and therefore leaves the prompt waiting.
    local selected = 2
    while true do
        local input = game.ui.cursor_menu(
            "hospital_leave_confirmation", { "yes", "no" }, selected,
            NARRATOR, TEXT_PROFILE)
        if input == "up" then
            selected = (selected - 2) % 2 + 1
        elseif input == "down" then
            selected = selected % 2 + 1
        elseif input == "confirm" or input == "page" or input == "back" then
            return selected == 1 and "hospital_left" or "hospital-options"
        elseif input == "menu_cancel" then
            return "hospital-options"
        elseif input == "cancel" then
            -- Start is not part of the ROM's accepted-button mask.
        end
    end
end

-- Hospital Options -> Cyberware (ROM 0xCA16) enters the shared 0x56AC0
-- service; the host opens the site's catalog view returned here.
function Hospital.cyberware_catalog_id()
    local site = assert_hospital_field_context()
    return assert(site.cyberware_catalog,
                  "hospital site has no cyberware catalog")
end

return Hospital
