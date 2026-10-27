-- Employee-recruitment offers use the same source location pools as package
-- recovery, then select one of the twelve source client records.
local EmployeeRecruitment = {}

function EmployeeRecruitment.prepare_offer(context, source_record)
    local support = game.modules.run_support
    local payment = support.negotiate_payment(source_record)
    local target = support.choose_facility_target(
        source_record.source_initializer)
    local client_id = support.bounded_random(#support.client_names)
    local offer = support.new_offer(
        "employee_recruitment",
        "extraction",
        "recruit_the_selected_employee",
        "return_to_johnson_after_the_recruitment",
        { target = target },
        payment,
        source_record.runner_progress_delta)
    offer.context = context
    offer.client_id = client_id
    offer.employee_name = support.client_names[client_id + 1]
    game.runs.registry.current_offer = offer
    return offer.id
end

function EmployeeRecruitment.present_offer()
    local offer = game.runs.registry.current_offer
    assert(offer ~= nil and offer.id == "employee_recruitment")
    local target = offer.route.target
    game.ui.say_text(
        "An employee of " .. target.location_name ..
        " " .. target.area_suffix .. ", " .. offer.employee_name ..
        ", wishes for a change in career.|" ..
        "Unfortunately, his current employer frowns upon this.||" ..
        "Assist my client with his professional growth and return here " ..
        "to receive full compensation for your services.", false)
    -- ROM 0x1E69C copies the selected F3EBC initializer to FFFBED and calls
    -- F3DC6[FFFBED]. Base initializer 5 uses 0x1E810; Employee Recruitment
    -- variants 11/12 use 0x1E81C/0x1E7D0 and their fallback branches store 5
    -- at 0x1E848/0x1E804 before 0xA800 reads it. Later 0x214CE dispatches
    -- F3E1E row 5 to 0x1E9AC, which prints F39EC and returns text-only
    -- descriptor 0x1058B8, never the 0x214F6 payment screen [C].
    -- Accepting without a Yes/No mirrors the bodyguard row; what state 0x0A
    -- after that descriptor does beyond accepting is not yet traced [H].
    return game.runs.registry.resolve_current_offer("yes")
end

function EmployeeRecruitment.resolve_field_event(offer, map_id, event_id)
    local route = type(offer) == "table" and offer.route or nil
    local target = type(route) == "table" and route.target or nil
    if type(target) ~= "table" or
       type(target.area_id) ~= "number" or
       type(target.location_id) ~= "number" then
        return { action = "no_action" }
    end

    -- ROM 0x1F106 compares FFFBF8/FFFBF9 with FFFBEE/FFFBEF. For the
    -- Renraku initializer, 0x1E7D0 sets that pair to area 5/event 6.
    if map_id == target.area_id and event_id == target.location_id then
        return {
            action = "entry_dialogue",
            dialogue_module = "run_employee_recruitment",
            dialogue_function = "present_target_entry",
        }
    end
    return { action = "no_action" }
end

function EmployeeRecruitment.present_target_entry()
    local offer = game.runs.registry.active_offer
    assert(offer ~= nil and offer.id == "employee_recruitment")
    -- ROM text at 0xFE169, with its 0x80 row breaks and one 0xFF page end.
    game.ui.say_text(
        "<You create a diversion and slip into the office building.>\n\n" ..
        "Now comes the hard part.\n\n" ..
        "Finding your quarry and escaping with everyone intact!>", false)
    return "shown"
end

return EmployeeRecruitment
