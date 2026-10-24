package.path = "decompilation/gameplay/scripts/?.lua;" .. package.path

local E = require("enums")
local OfferData = require("data.johnson_run_offers")
local Catalog = require("data.matrix_run_catalog")
local Support = require("run_support")

local function offer(initializer, context, run_type, extra)
    local result = {
        id = OfferData.run_by_initializer[initializer].id,
        source_initializer = initializer,
        source_offer_type = initializer,
        source_offer_context = context,
        source_run_type = run_type,
        context = context,
        payment = { base_amount = 40, negotiated_amount = 55 },
    }
    for key, value in pairs(extra or {}) do result[key] = value end
    return result
end

local function view(initializer, context, run_type, extra)
    local result = Support.current_run_view(
        offer(initializer, context, run_type, extra))
    assert(result ~= nil, "source offer should produce a Current Run view")
    assert(result.source_initializer == initializer)
    assert(result.context == context)
    assert(result.run_type == run_type)
    assert(result.payment_amount == 55,
        "Current Run must use the accepted negotiated amount")
    for _, line in ipairs(result.body_lines) do
        assert(#line <= 27, "Current Run text row exceeds source scan width")
    end
    return result
end

local function pair(area_id, location_id)
    return { area_id = area_id, location_id = location_id }
end

local ghoul = view(E.RunOfferInitializer.GHOUL_BOUNTY, 0,
    E.RunType.GHOUL_BOUNTY, { area_id = 2, ghoul_kill_count = 19 })
assert(ghoul.johnson_name == "Mr. Gunderson")
assert(ghoul.run_type_name == "Ghoul Bounty")
assert(ghoul.body ==
    "There is a community of Ghouls somewhere " ..
    "in the Redmond Barrens.  " ..
    "They have been preying on the citizens there." ..
    "\n\nKill as many as possible and return here for payment. ",
    string.format("unexpected ghoul text: %q", ghoul.body))
local ghoul_without_kills = offer(E.RunOfferInitializer.GHOUL_BOUNTY, 0,
    E.RunType.GHOUL_BOUNTY, { area_id = 2, ghoul_kill_count = 0 })
assert(Support.current_run_view(ghoul_without_kills).body == ghoul.body,
    "live ghoul count is not part of the detail text")

local escort = view(E.RunOfferInitializer.BODYGUARD_ESCORT, 1,
    E.RunType.BODYGUARD_ESCORT,
    { route = {
        origin = pair(2, 6), destination = pair(5, 6), client_id = 7,
    } })
assert(escort.johnson_name == "Julius Strouther")
assert(escort.body ==
    "Go to Boris' Greenhouse in the Redmond Barrens.  " ..
    "There you will meet my client, Billy Gibson  " ..
    "\n\nEscort my client to the Renraku Offices " ..
    "in the Renraku Arcology.  " ..
    "You will be paid by my client upon safe arrival at the specified destination.",
    string.format("unexpected escort text: %q", escort.body))

local courier = view(E.RunOfferInitializer.PACKAGE_DELIVERY, 0,
    E.RunType.PACKAGE_DELIVERY,
    { route = {
        origin = pair(2, 2), destination = pair(1, 7),
    } })
assert(courier.body ==
    "Go to Stoker's Coffin Motel in the Redmond Barrens.  " ..
    "A contact there will give you a package." ..
    "\n\nTake it to the Tarislar Gardens Apartment Complex " ..
    "in the Puyallup Barrens.  " ..
    "You will be paid upon safe delivery of the package.")

local video_courier = view(E.RunOfferInitializer.PACKAGE_DELIVERY, 0,
    E.RunType.PACKAGE_DELIVERY,
    { route = {
        origin = pair(2, 11), destination = pair(2, 5),
    } })
assert(table.concat(video_courier.body_lines, "\n") ==
    "Go to Ares Weapon Emporium\n" ..
    "in the Redmond Barrens.  A\n" ..
    "contact there will give\n" ..
    "you a package.\n\n" ..
    "Take it to the Jackal's\n" ..
    "Lantern in the Redmond\n" ..
    "Barrens.  You will be paid\n" ..
    "upon safe delivery of the\n" ..
    "package.",
    "Current Run wrap follows the captured Section 03 Courier sequence")

local gang = view(E.RunOfferInitializer.GANG_CONTRACT, 2,
    E.RunType.GANG_CONTRACT,
    { variant = 1, route = { target = pair(3, 4) } })
assert(gang.johnson_name == "Mortimer Reed")
assert(gang.body ==
    "Go see the Eye Fivers in the Penumbra District.  " ..
    "They have been feeding Renraku a lot of information.\n\n" ..
    "Renraku has been using this info to find and eliminate Johnsons.  " ..
    "Needless to say, this makes me uneasy.\n\n" ..
    "\"Discuss\" this matter with them and return here to collect your fee.")
for variant = 0, #Support.gang_contract_texts - 1 do
    local current = view(E.RunOfferInitializer.GANG_CONTRACT, 2,
        E.RunType.GANG_CONTRACT,
        { variant = variant, route = { target = pair(2, 9) } })
    assert(#current.body_lines <= 16,
        "gang variant must fit the source detail text panel")
end

local recovery = view(E.RunOfferInitializer.PACKAGE_RECOVERY, 3,
    E.RunType.PACKAGE_RECOVERY,
    { route = { target = pair(0, 6) } })
assert(recovery.johnson_name == "Vigore and Jarl")
assert(recovery.body ==
    "Go to the Space Needle in Downtown Seattle.  " ..
    "A package was taken from a courier of mine.\n\n" ..
    "Get the package and bring it back to me.\n\n" ..
    "I will then pay you for your services.",
    string.format("unexpected recovery text: %q", recovery.body))

local extraction = view(E.RunOfferInitializer.EMPLOYEE_RECRUITMENT, 4,
    E.RunType.EMPLOYEE_RECRUITMENT,
    { client_id = 0, route = { target = pair(3, 5) } })
assert(extraction.johnson_name == "Caleb Brightmore")
assert(extraction.body ==
    "An employee of the Big Rhino in the Penumbra District, Mike Nana, " ..
    "wishes for a change in career.  Unfortunately, his current employer " ..
    "frowns upon this.\n\nAssist my client with his professional growth and " ..
    "return here to receive full compensation for your services.")

local matrix_index = 2
local matrix_objective = Catalog.objectives[matrix_index + 1]
local matrix_system = matrix_objective.systems[1]
local matrix = view(E.RunOfferInitializer.MATRIX_SIMPLE, 2,
    E.RunType.MATRIX_OBJECTIVE, {
        objective_index = matrix_index,
        target_system_id = matrix_system,
        matrix_run_state = { target_system_id = matrix_system },
    })
assert(matrix.johnson_name == "Mortimer Reed")
assert(matrix.body == Catalog.systems[matrix_system] .. " " ..
                         matrix_objective.description)

for index = 0, #Catalog.objectives - 1 do
    local objective = Catalog.objectives[index + 1]
    local system_id = objective.systems[1]
    local current = view(E.RunOfferInitializer.MATRIX_SIMPLE, 0,
        E.RunType.MATRIX_OBJECTIVE, {
            objective_index = index,
            target_system_id = system_id,
            matrix_run_state = { target_system_id = system_id },
        })
    if index < 17 then
        assert(current.body == Catalog.systems[system_id] .. " " ..
                              objective.description)
    else
        assert(current.body == objective.description,
            "source objective indices 17+ omit the system line")
    end
end

local expected_initializers = {
    [0] = E.RunType.GHOUL_BOUNTY,
    [1] = E.RunType.BODYGUARD_ESCORT,
    [2] = E.RunType.PACKAGE_DELIVERY,
    [3] = E.RunType.GANG_CONTRACT,
    [4] = E.RunType.PACKAGE_RECOVERY,
    [5] = E.RunType.EMPLOYEE_RECRUITMENT,
    [6] = E.RunType.MATRIX_OBJECTIVE,
    [7] = E.RunType.MATRIX_OBJECTIVE,
    [8] = E.RunType.MATRIX_OBJECTIVE,
    [9] = E.RunType.PACKAGE_RECOVERY,
    [10] = E.RunType.PACKAGE_RECOVERY,
    [11] = E.RunType.EMPLOYEE_RECRUITMENT,
    [12] = E.RunType.EMPLOYEE_RECRUITMENT,
}
local initializer_contexts = {}
for context, rows in pairs(OfferData.contexts) do
    for _, row in ipairs(rows) do
        initializer_contexts[row.source_initializer] = context
    end
end
local function fixture_for(initializer, context, run_type)
    if run_type == E.RunType.GHOUL_BOUNTY then
        return { area_id = 2 }
    elseif run_type == E.RunType.BODYGUARD_ESCORT or
           run_type == E.RunType.PACKAGE_DELIVERY then
        return { route = {
            origin = pair(2, 0), destination = pair(2, 1), client_id = 0,
        } }
    elseif run_type == E.RunType.GANG_CONTRACT then
        return { variant = 0,
                 route = { target = pair(1, 9) } }
    elseif run_type == E.RunType.PACKAGE_RECOVERY or
           run_type == E.RunType.EMPLOYEE_RECRUITMENT then
        local pool = Support.run_target_pools.four_site
        if initializer == E.RunOfferInitializer.PACKAGE_RECOVERY_TWO_SITE_POOL or
           initializer == E.RunOfferInitializer.EMPLOYEE_RECRUITMENT_TWO_SITE_POOL then
            pool = Support.run_target_pools.two_site
        elseif initializer == E.RunOfferInitializer.PACKAGE_RECOVERY_RENRAKU or
               initializer == E.RunOfferInitializer.EMPLOYEE_RECRUITMENT_RENRAKU then
            pool = { Support.run_target_pools.renraku }
        end
        local target = pool[1]
        if run_type == E.RunType.EMPLOYEE_RECRUITMENT then
            return { client_id = 0,
                     route = { target = pair(target.area_id, target.location_id) } }
        end
        return { route = { target = pair(target.area_id, target.location_id) } }
    elseif run_type == E.RunType.MATRIX_OBJECTIVE then
        local objective = Catalog.objectives[1]
        local system_id = objective.systems[1]
        return { objective_index = 0, target_system_id = system_id,
                 matrix_run_state = { target_system_id = system_id } }
    end
end

local initializer_count = 0
for initializer, run_type in pairs(expected_initializers) do
    local context = assert(initializer_contexts[initializer])
    local current = view(initializer, context, run_type,
        fixture_for(initializer, context, run_type))
    assert(current.source_initializer == initializer)
    initializer_count = initializer_count + 1
end
assert(initializer_count == 13, "all thirteen source initializer IDs are tested")

local function each_contract_route(context, callback)
    if context == E.RunOfferContext.JUMP_HOUSE then
        local area = 2
        local rolls = Support.location_rolls_by_area[area]
        for _, origin_id in ipairs(rolls) do
            for _, destination_id in ipairs(rolls) do
                if destination_id ~= origin_id then
                    callback(context, pair(area, origin_id),
                             pair(area, destination_id))
                end
            end
        end
        return
    end
    for origin_area = 0, 5 do
        for destination_area = 0, 5 do
            if destination_area ~= origin_area then
                for _, origin_id in ipairs(
                    Support.location_rolls_by_area[origin_area]) do
                    for _, destination_id in ipairs(
                        Support.location_rolls_by_area[destination_area]) do
                        callback(context, pair(origin_area, origin_id),
                                 pair(destination_area, destination_id))
                    end
                end
            end
        end
    end
end

local route_cases = 0
for context = E.RunOfferContext.JUMP_HOUSE,
              E.RunOfferContext.ICARUS_DESCENDING do
    each_contract_route(context, function(route_context, origin, destination)
        local courier_view = Support.current_run_view(offer(
            E.RunOfferInitializer.PACKAGE_DELIVERY, route_context,
            E.RunType.PACKAGE_DELIVERY,
            { route = { origin = origin, destination = destination } }))
        assert(courier_view ~= nil)
        assert(courier_view.body:find(
            Support.location_names[origin.area_id][origin.location_id], 1, true))
        assert(courier_view.body:find(
            Support.location_names[destination.area_id][destination.location_id], 1, true))
        for _, line in ipairs(courier_view.body_lines) do
            assert(#line <= 26)
        end
        route_cases = route_cases + 1

        for client_id = 0, #Support.client_names - 1 do
            local bodyguard_view = Support.current_run_view(offer(
                E.RunOfferInitializer.BODYGUARD_ESCORT, route_context,
                E.RunType.BODYGUARD_ESCORT,
                { route = { origin = origin, destination = destination,
                            client_id = client_id } }))
            assert(bodyguard_view ~= nil)
            assert(bodyguard_view.body:find(
                Support.client_names[client_id + 1], 1, true))
            assert(bodyguard_view.body:find(
                Support.location_names[origin.area_id][origin.location_id], 1, true))
            assert(bodyguard_view.body:find(
                Support.location_names[destination.area_id][destination.location_id],
                1, true))
            assert(#bodyguard_view.body_lines <= 16,
                "route/client combination must fit the detail text panel")
        end
    end)
end
assert(route_cases == 12090, "every legal source route pair is covered")

local bad = offer(E.RunOfferInitializer.PACKAGE_DELIVERY, 0,
    E.RunType.PACKAGE_DELIVERY, {
        route = { origin = pair(2, 2), destination = pair(2, 2) },
    })
assert(Support.current_run_view(bad) ~= nil,
    "the display follows saved active fields without silently rerolling them")
bad.payment.negotiated_amount = 65536
assert(Support.current_run_view(bad) == nil,
    "payment must fit the source word field")
bad.payment.negotiated_amount = 55
bad.source_run_type = E.RunType.GANG_CONTRACT
assert(Support.current_run_view(bad) == nil,
    "the source initializer and canonical run type must agree")

print("current run views passed: 13 initializers, 24 Matrix objectives, " ..
      tostring(route_cases) .. " route pairs and " ..
      tostring(route_cases * #Support.client_names) .. " escort client routes")
