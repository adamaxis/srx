package.path = "decompilation/gameplay/scripts/?.lua;" .. package.path

local E = require("enums")
local OfferData = require("data.johnson_run_offers")
local Catalog = require("data.matrix_run_catalog")
local Support = require("run_support")

local function offer(variant_id, contact_id, run_id, extra)
    local resolved = OfferData.run_by_variant_id[variant_id]
    assert(resolved.id == run_id,
        "offer variant must map to its semantic run identity")
    local result = {
        id = resolved.id,
        offer_variant_id = variant_id,
        contact_id = contact_id,
        payment = { base_amount = 40, negotiated_amount = 55 },
    }
    for key, value in pairs(extra or {}) do result[key] = value end
    return result
end

local function view(variant_id, contact_id, run_id, extra)
    local result = Support.current_run_view(
        offer(variant_id, contact_id, run_id, extra))
    assert(result ~= nil, "source offer should produce a Current Run view")
    assert(result.offer_variant_id == variant_id)
    assert(result.contact_id == contact_id)
    assert(result.run_id == run_id)
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

local ghoul = view("ghoul_bounty", E.RunContact.JUMP_HOUSE,
    E.RunId.GHOUL_BOUNTY, { area_id = 2, ghoul_kill_count = 19 })
assert(ghoul.johnson_name == "Mr. Gunderson")
assert(ghoul.run_type_name == "Ghoul Bounty")
assert(ghoul.body ==
    "There is a community of Ghouls somewhere " ..
    "in the Redmond Barrens.  " ..
    "They have been preying on the citizens there." ..
    "\n\nKill as many as possible and return here for payment. ",
    string.format("unexpected ghoul text: %q", ghoul.body))
local ghoul_without_kills = offer("ghoul_bounty", E.RunContact.JUMP_HOUSE,
    E.RunId.GHOUL_BOUNTY, { area_id = 2, ghoul_kill_count = 0 })
assert(Support.current_run_view(ghoul_without_kills).body == ghoul.body,
    "live ghoul count is not part of the detail text")

local escort = view("bodyguard_escort", E.RunContact.MATCHSTICK,
    E.RunId.BODYGUARD_ESCORT,
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

local courier = view("package_delivery", E.RunContact.JUMP_HOUSE,
    E.RunId.PACKAGE_DELIVERY,
    { route = {
        origin = pair(2, 2), destination = pair(1, 7),
    } })
assert(courier.body ==
    "Go to Stoker's Coffin Motel in the Redmond Barrens.  " ..
    "A contact there will give you a package." ..
    "\n\nTake it to the Tarislar Gardens Apartment Complex " ..
    "in the Puyallup Barrens.  " ..
    "You will be paid upon safe delivery of the package.")

local video_courier = view("package_delivery", E.RunContact.JUMP_HOUSE,
    E.RunId.PACKAGE_DELIVERY,
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

local gang = view("gang_contract", E.RunContact.BIG_RHINO,
    E.RunId.GANG_CONTRACT,
    { variant = 1, route = { target = pair(3, 4) } })
assert(gang.johnson_name == "Mortimer Reed")
assert(gang.body ==
    "Go see the Eye Fivers in the Penumbra District.  " ..
    "They have been feeding Renraku a lot of information.\n\n" ..
    "Renraku has been using this info to find and eliminate Johnsons.  " ..
    "Needless to say, this makes me uneasy.\n\n" ..
    "\"Discuss\" this matter with them and return here to collect your fee.")
for variant = 0, #Support.gang_contract_texts - 1 do
    local current = view("gang_contract", E.RunContact.BIG_RHINO,
        E.RunId.GANG_CONTRACT,
        { variant = variant, route = { target = pair(2, 9) } })
    assert(#current.body_lines <= 16,
        "gang variant must fit the source detail text panel")
end

local recovery = view("package_recovery", E.RunContact.WANDERER,
    E.RunId.PACKAGE_RECOVERY,
    { route = { target = pair(0, 6) } })
assert(recovery.johnson_name == "Vigore and Jarl")
assert(recovery.body ==
    "Go to the Space Needle in Downtown Seattle.  " ..
    "A package was taken from a courier of mine.\n\n" ..
    "Get the package and bring it back to me.\n\n" ..
    "I will then pay you for your services.",
    string.format("unexpected recovery text: %q", recovery.body))

local extraction = view("employee_recruitment", E.RunContact.ICARUS_DESCENDING,
    E.RunId.EMPLOYEE_RECRUITMENT,
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
local matrix = view("matrix_simple", E.RunContact.BIG_RHINO,
    E.RunId.MATRIX_OBJECTIVE, {
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
    local current = view("matrix_simple", E.RunContact.JUMP_HOUSE,
        E.RunId.MATRIX_OBJECTIVE, {
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

local expected_variants = {
    ghoul_bounty = E.RunId.GHOUL_BOUNTY,
    bodyguard_escort = E.RunId.BODYGUARD_ESCORT,
    package_delivery = E.RunId.PACKAGE_DELIVERY,
    gang_contract = E.RunId.GANG_CONTRACT,
    package_recovery = E.RunId.PACKAGE_RECOVERY,
    employee_recruitment = E.RunId.EMPLOYEE_RECRUITMENT,
    matrix_simple = E.RunId.MATRIX_OBJECTIVE,
    matrix_moderate = E.RunId.MATRIX_OBJECTIVE,
    matrix_expert = E.RunId.MATRIX_OBJECTIVE,
    package_recovery_two_site_pool = E.RunId.PACKAGE_RECOVERY,
    package_recovery_renraku = E.RunId.PACKAGE_RECOVERY,
    employee_recruitment_two_site_pool = E.RunId.EMPLOYEE_RECRUITMENT,
    employee_recruitment_renraku = E.RunId.EMPLOYEE_RECRUITMENT,
}
local variant_contacts = {}
for contact_id, rows in pairs(OfferData.contacts) do
    for _, row in ipairs(rows) do
        variant_contacts[row.variant_id] = contact_id
    end
end
local function fixture_for(variant_id, contact_id, run_id)
    if run_id == E.RunId.GHOUL_BOUNTY then
        return { area_id = 2 }
    elseif run_id == E.RunId.BODYGUARD_ESCORT or
           run_id == E.RunId.PACKAGE_DELIVERY then
        return { route = {
            origin = pair(2, 0), destination = pair(2, 1), client_id = 0,
        } }
    elseif run_id == E.RunId.GANG_CONTRACT then
        return { variant = 0,
                 route = { target = pair(1, 9) } }
    elseif run_id == E.RunId.PACKAGE_RECOVERY or
           run_id == E.RunId.EMPLOYEE_RECRUITMENT then
        local pool = Support.run_target_pools.four_site
        if variant_id == "package_recovery_two_site_pool" or
           variant_id == "employee_recruitment_two_site_pool" then
            pool = Support.run_target_pools.two_site
        elseif variant_id == "package_recovery_renraku" or
               variant_id == "employee_recruitment_renraku" then
            pool = { Support.run_target_pools.renraku }
        end
        local target = pool[1]
        if run_id == E.RunId.EMPLOYEE_RECRUITMENT then
            return { client_id = 0,
                     route = { target = pair(target.area_id, target.location_id) } }
        end
        return { route = { target = pair(target.area_id, target.location_id) } }
    elseif run_id == E.RunId.MATRIX_OBJECTIVE then
        local objective = Catalog.objectives[1]
        local system_id = objective.systems[1]
        return { objective_index = 0, target_system_id = system_id,
                 matrix_run_state = { target_system_id = system_id } }
    end
end

local variant_count = 0
for variant_id, run_id in pairs(expected_variants) do
    local contact_id = assert(variant_contacts[variant_id])
    local current = view(variant_id, contact_id, run_id,
        fixture_for(variant_id, contact_id, run_id))
    assert(current.offer_variant_id == variant_id)
    variant_count = variant_count + 1
end
assert(variant_count == 13, "all thirteen source offer variants are tested")

local function each_contract_route(contact_id, callback)
    if contact_id == E.RunContact.JUMP_HOUSE then
        local area = 2
        local rolls = Support.location_rolls_by_area[area]
        for _, origin_id in ipairs(rolls) do
            for _, destination_id in ipairs(rolls) do
                if destination_id ~= origin_id then
                    callback(contact_id, pair(area, origin_id),
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
                        callback(contact_id, pair(origin_area, origin_id),
                                 pair(destination_area, destination_id))
                    end
                end
            end
        end
    end
end

local route_cases = 0
local contacts = {
    E.RunContact.JUMP_HOUSE,
    E.RunContact.MATCHSTICK,
    E.RunContact.BIG_RHINO,
    E.RunContact.WANDERER,
    E.RunContact.ICARUS_DESCENDING,
}
for _, contact_id in ipairs(contacts) do
    each_contract_route(contact_id, function(route_contact, origin, destination)
        local courier_view = Support.current_run_view(offer(
            "package_delivery", route_contact,
            E.RunId.PACKAGE_DELIVERY,
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
                "bodyguard_escort", route_contact,
                E.RunId.BODYGUARD_ESCORT,
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

local bad = offer("package_delivery", E.RunContact.JUMP_HOUSE,
    E.RunId.PACKAGE_DELIVERY, {
        route = { origin = pair(2, 2), destination = pair(2, 2) },
    })
assert(Support.current_run_view(bad) ~= nil,
    "the display follows saved active fields without silently rerolling them")
bad.payment.negotiated_amount = 65536
assert(Support.current_run_view(bad) == nil,
    "payment must fit the source word field")
bad.payment.negotiated_amount = 55
bad.id = E.RunId.GANG_CONTRACT
assert(Support.current_run_view(bad) == nil,
    "the semantic offer variant and run identity must agree")

print("current run views passed: 13 offer variants, 24 Matrix objectives, " ..
      tostring(route_cases) .. " route pairs and " ..
      tostring(route_cases * #Support.client_names) .. " escort client routes")
