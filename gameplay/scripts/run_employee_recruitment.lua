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
        "to receive full compensation for your services.", true)
    return game.modules.run_support.present_run_acceptance(
        offer, "for assisting my client's professional growth.")
end

function EmployeeRecruitment.resolve_field_event()
    -- The ROM's target contact and event-30 run settlement have not been
    -- connected to the field event facts yet.
    return { action = "no_action" }
end

return EmployeeRecruitment
