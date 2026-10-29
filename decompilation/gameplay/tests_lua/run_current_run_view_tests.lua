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

local function point(location_id)
    return { location_id = location_id }
end

local ghoul = view("ghoul_bounty", E.RunContact.JUMP_HOUSE,
    E.RunId.GHOUL_BOUNTY,
    { region_id = "redmond_barrens", ghoul_kill_count = 19 })
assert(ghoul.johnson_name == "Mr. Gunderson")
assert(ghoul.run_type_name == "Ghoul Bounty")
assert(ghoul.body ==
    "There is a community of Ghouls somewhere " ..
    "in the Redmond Barrens.  " ..
    "They have been preying on the citizens there." ..
    "\n\nKill as many as possible and return here for payment. ",
    string.format("unexpected ghoul text: %q", ghoul.body))
local ghoul_without_kills = offer("ghoul_bounty", E.RunContact.JUMP_HOUSE,
    E.RunId.GHOUL_BOUNTY,
    { region_id = "redmond_barrens", ghoul_kill_count = 0 })
assert(Support.current_run_view(ghoul_without_kills).body == ghoul.body,
    "live ghoul count is not part of the detail text")

local escort = view("bodyguard_escort", E.RunContact.MATCHSTICK,
    E.RunId.BODYGUARD_ESCORT,
    { route = {
        origin = point("boris_greenhouse"),
        destination = point("renraku_offices"),
        client_id = "billy_gibson",
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
        origin = point("stokers_coffin_motel"),
        destination = point("tarislar_gardens_apartments"),
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
        origin = point("ares_weapon_emporium"),
        destination = point("jackals_lantern"),
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
    { contract_variant_id = "renraku_information_leak",
      route = { target = point("eye_fivers_hideout") } })
assert(gang.johnson_name == "Mortimer Reed")
assert(gang.body ==
    "Go see the Eye Fivers in the Penumbra District.  " ..
    "They have been feeding Renraku a lot of information.\n\n" ..
    "Renraku has been using this info to find and eliminate Johnsons.  " ..
    "Needless to say, this makes me uneasy.\n\n" ..
    "\"Discuss\" this matter with them and return here to collect your fee.")
for _, variant in ipairs(Support.gang_contract_variants) do
    local current = view("gang_contract", E.RunContact.BIG_RHINO,
        E.RunId.GANG_CONTRACT,
        { contract_variant_id = variant.id,
          route = { target = point("halloweeners_hideout") } })
    assert(#current.body_lines <= 16,
        "gang variant must fit the source detail text panel")
end

local recovery = view("package_recovery", E.RunContact.WANDERER,
    E.RunId.PACKAGE_RECOVERY,
    { route = { target = point("space_needle") } })
assert(recovery.johnson_name == "Vigore and Jarl")
assert(recovery.body ==
    "Go to the Space Needle in Downtown Seattle.  " ..
    "A package was taken from a courier of mine.\n\n" ..
    "Get the package and bring it back to me.\n\n" ..
    "I will then pay you for your services.",
    string.format("unexpected recovery text: %q", recovery.body))

local extraction = view("employee_recruitment", E.RunContact.ICARUS_DESCENDING,
    E.RunId.EMPLOYEE_RECRUITMENT,
    { employee_id = "mike_nana",
      route = { target = point("big_rhino") } })
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
        return { region_id = "redmond_barrens" }
    elseif run_id == E.RunId.BODYGUARD_ESCORT or
           run_id == E.RunId.PACKAGE_DELIVERY then
        return { route = {
            origin = point("hollywood_correctional_facilities"),
            destination = point("little_chiba"),
            client_id = "mike_nana",
        } }
    elseif run_id == E.RunId.GANG_CONTRACT then
        return { contract_variant_id = "building_vandalism",
                 route = { target = point("ork_gang_hideout") } }
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
        local target_location_id = pool[1]
        if run_id == E.RunId.EMPLOYEE_RECRUITMENT then
            return { employee_id = "mike_nana",
                     route = { target = point(target_location_id) } }
        end
        return { route = { target = point(target_location_id) } }
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
        local region_id = "redmond_barrens"
        local choices = Support.location_choice_ids_by_region[region_id]
        for _, origin_id in ipairs(choices) do
            for _, destination_id in ipairs(choices) do
                if destination_id ~= origin_id then
                    callback(contact_id, point(origin_id),
                             point(destination_id))
                end
            end
        end
        return
    end
    for _, origin_region_id in ipairs(Support.contract_region_ids) do
        for _, destination_region_id in ipairs(Support.contract_region_ids) do
            if destination_region_id ~= origin_region_id then
                for _, origin_id in ipairs(
                    Support.location_choice_ids_by_region[origin_region_id]) do
                    for _, destination_id in ipairs(
                        Support.location_choice_ids_by_region[
                            destination_region_id]) do
                        callback(contact_id, point(origin_id),
                                 point(destination_id))
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
            Support.locations[origin.location_id].name, 1, true))
        assert(courier_view.body:find(
            Support.locations[destination.location_id].name, 1, true))
        for _, line in ipairs(courier_view.body_lines) do
            assert(#line <= 26)
        end
        route_cases = route_cases + 1

        for _, client_id in ipairs(Support.client_ids) do
            local bodyguard_view = Support.current_run_view(offer(
                "bodyguard_escort", route_contact,
                E.RunId.BODYGUARD_ESCORT,
                { route = { origin = origin, destination = destination,
                            client_id = client_id } }))
            assert(bodyguard_view ~= nil)
            assert(bodyguard_view.body:find(
                Support.client_name(client_id), 1, true))
            assert(bodyguard_view.body:find(
                Support.locations[origin.location_id].name, 1, true))
            assert(bodyguard_view.body:find(
                Support.locations[destination.location_id].name,
                1, true))
            assert(#bodyguard_view.body_lines <= 16,
                "route/client combination must fit the detail text panel")
        end
    end)
end
assert(route_cases == 12090, "every legal source route pair is covered")

local bad = offer("package_delivery", E.RunContact.JUMP_HOUSE,
    E.RunId.PACKAGE_DELIVERY, {
        route = {
            origin = point("stokers_coffin_motel"),
            destination = point("stokers_coffin_motel"),
        },
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
      tostring(route_cases * #Support.client_ids) .. " escort client routes")
