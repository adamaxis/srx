-- Scene 4/event 4 uses the shared field-mode prompt service, not a separate
-- hospital interior. ROM 0x1FA02 selects one of three text roots from the
-- alternate-result byte. The default root offers Hospital Options or leave.
local Hospital = {}

local LOCATION_ID = "council_island_hospital"
local NARRATOR = "council_island_hospital_narrator"
local TEXT_PROFILE = "council_island_hospital"

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
        elseif input == "back" or (input == "cancel" and not ignore_cancel) then
            return input, selected
        end
    end
end

local function ceil_thirds_minus_one(value)
    return math.floor((value + 2) / 3) - 1
end

local function medical_attention_value(runner, base_service_level)
    local class_delta = (runner.archetype - 2) % 256
    local service_level = base_service_level
    if class_delta >= 128 then
        service_level = service_level + 2
    end
    service_level = service_level -
        ceil_thirds_minus_one(runner.attributes[0]) -
        ceil_thirds_minus_one(runner.attributes[5])
    service_level = math.max(service_level, 2)

    local tier_delta = service_level - 7
    local factor = tier_delta >= 0 and 10 or -tier_delta * 20
    local scaled_body = runner.attributes[0] * factor
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
    assert(game.location.type == "building" and
           game.location.id == LOCATION_ID,
        "Council Island Hospital prompt ran outside its linked building")
    assert(game.field.mode == 4 and game.field.event_code == 4,
        "Council Island Hospital prompt requires ROM field mode/event 4/4")
end

function Hospital.present_field_exit_prompt()
    assert_hospital_field_context()

    local alternate = game.field.alternate_result
    if alternate < 0 then
        game.ui.say("package_handoff")
        return "field_return"
    elseif alternate > 0 then
        game.ui.say("package_payment")
        return "field_return"
    end

    local choice = game.ui.prompt_text_key(
        "hospital_service_offer",
        "council_island_hospital.entry.default",
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
        if choice == "back" then
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
        elseif input == "cancel" then
            -- Start is not part of the ROM's accepted-button mask.
        end
    end
end

return Hospital
