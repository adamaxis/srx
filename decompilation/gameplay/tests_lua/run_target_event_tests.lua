package.path = "decompilation/gameplay/scripts/?.lua;" .. package.path

local EmployeeRecruitment = require("run_employee_recruitment")
local PackageRecovery = require("run_package_recovery")
local PackageDelivery = require("run_package_delivery")
local GangContract = require("run_gang_contract")
local RunSupport = require("run_support")
local active_offer = {
    id = "employee_recruitment",
    route = { target = { location_id = "renraku_offices" } },
}

local matched = EmployeeRecruitment.resolve_field_event(
    active_offer, "renraku_offices")
assert(matched.action == "entry_dialogue")
assert(matched.dialogue_module == "run_employee_recruitment")
assert(matched.dialogue_function == "present_target_entry")

for _, location_id in ipairs({ "little_chiba", "wire_masters", "ork_embassy" }) do
    local ignored = EmployeeRecruitment.resolve_field_event(
        active_offer, location_id)
    assert(ignored.action == "no_action")
end

local presented
game = {
    runs = { registry = {
        active_offer = active_offer,
        resolve_current_offer = function(choice)
            assert(choice == "yes")
            return "accepted:" .. choice
        end,
    } },
    modules = {
        run_support = {
            location_details = RunSupport.location_details,
            client_name = RunSupport.client_name,
            gang_actor_group_id = RunSupport.gang_actor_group_id,
            amount_in_reserved_field = function(amount, width)
                local digits = tostring(amount)
                return string.rep(" ", math.max(0, width - #digits)) .. digits
            end,
            resolve_offer_decision = function(choice) return "accepted:" .. choice end,
        },
    },
    ui = {
        say_text = function(text, continuation)
            presented = { text = text, continuation = continuation }
        end,
        prompt_text = function(id, _, choices)
            assert(id == "run_acceptance")
            assert(choices[1] == "yes" and choices[2] == "no")
            return "yes"
        end,
    },
}
assert(EmployeeRecruitment.present_target_entry() == "shown")
assert(presented ~= nil and presented.continuation == false)
assert(presented.text ==
    "<You create a diversion and slip into the office building.>\n\n" ..
    "Now comes the hard part.\n\n" ..
    "Finding your quarry and escaping with everyone intact!>")

-- Audit the complete Employee Recruitment target/client text matrix. ROM
-- 0x1E91A changes the selected area's terminal period to a comma before the
-- client name; all source initializer pools and all twelve F35A6 clients
-- must use that punctuation in the actual Johnson offer presenter.
local recruitment_target_pools = {
    RunSupport.run_target_pools.four_site,
    RunSupport.run_target_pools.two_site,
    { RunSupport.run_target_pools.renraku },
}
local recruitment_offer_cases = 0
for _, target_pool in ipairs(recruitment_target_pools) do
    for _, location_id in ipairs(target_pool) do
        local details = assert(RunSupport.location_details(location_id))
        local location_name = details.name
        local displayed_area = details.region_suffix:gsub("%.$", ",", 1)
        for _, employee_id in ipairs(RunSupport.client_ids) do
            local employee_name = RunSupport.client_name(employee_id)
            game.runs.registry.current_offer = {
                id = "employee_recruitment",
                route = { target = { location_id = location_id } },
                employee_id = employee_id,
            }
            assert(EmployeeRecruitment.present_offer() == "accepted:yes")
            local expected = "An employee of " .. location_name .. " " ..
                displayed_area .. " " .. employee_name ..
                ", wishes for a change in career.|" ..
                "Unfortunately, his current employer frowns upon this.||" ..
                "Assist my client with his professional growth and return here " ..
                "to receive full compensation for your services."
            assert(presented ~= nil and presented.continuation == false)
            assert(presented.text == expected,
                string.format("wrong recruitment text for %s / %s: %q",
                    location_name, employee_name,
                    presented and presented.text or "<no text>"))
            assert(not presented.text:find("%.,"),
                "recruitment area label retained both its period and comma")
            recruitment_offer_cases = recruitment_offer_cases + 1
        end
    end
end
assert(recruitment_offer_cases == 84,
    "recruitment text matrix did not cover all target/client combinations")

local recovery_offer = {
    id = "package_recovery",
    route = { target = { location_id = "jackals_lantern" } },
}
local recovery_match = PackageRecovery.resolve_field_event(
    recovery_offer, "jackals_lantern")
assert(recovery_match.action == "entry_dialogue")
assert(recovery_match.dialogue_module == "run_package_recovery")
assert(recovery_match.dialogue_function == "present_target_entry")
assert(PackageRecovery.resolve_field_event(recovery_offer, "jump_house").action ==
    "no_action")
game.runs.registry.active_offer = recovery_offer
assert(PackageRecovery.present_target_entry() == "shown")
assert(presented ~= nil and presented.continuation == true)
assert(presented.text ==
    "<You walk into the reception area and single out a lone suit " ..
    "standing off to the side.>\n\n" ..
    "You give him a quick jab in the solar plexus and he drops, " ..
    "wheezing and clutching his chest.>")

local gang_offer = {
    id = "gang_contract",
    route = { target = { location_id = "halloweeners_hideout" } },
}
local gang_match = GangContract.resolve_field_event(
    gang_offer, "halloweeners_hideout")
assert(gang_match.action == "entry_dialogue")
assert(gang_match.dialogue_module == "run_gang_contract")
assert(gang_match.dialogue_function == "present_target_entry")
assert(gang_match.actor_group_id == "gang_contract.halloweeners")
for _, location_id in ipairs({ "ork_gang_hideout", "little_chiba", "eye_fivers_hideout" }) do
    assert(GangContract.resolve_field_event(
        gang_offer, location_id).action == "no_action")
end

gang_offer.route.target = { location_id = "ork_gang_hideout" }
local first_gang_match = GangContract.resolve_field_event(
    gang_offer, "ork_gang_hideout")
assert(first_gang_match.action == "entry_dialogue")
assert(first_gang_match.actor_group_id == "gang_contract.ork_gang")
gang_offer.route.target = { location_id = "eye_fivers_hideout" }
local third_gang_match = GangContract.resolve_field_event(
    gang_offer, "eye_fivers_hideout")
assert(third_gang_match.action == "entry_dialogue")
assert(third_gang_match.actor_group_id == "gang_contract.eye_fivers")

game.runs.registry.active_offer = gang_offer
assert(GangContract.present_target_entry() == "shown")
assert(presented ~= nil and presented.continuation == true)
assert(presented.text ==
    "<After politely discussing the situation with the ornery gang " ..
    "leader, his reply is:>\n\n" ..
    "  Slot off, frag-face!!!\n\n" ..
    "<Guess what happens next.>")

-- ROM 0x1EBAA streams route phrases continuously; only F23B8's two 0x80
-- controls create a blank row before the destination instruction. Exercise
-- every source location-roll pair across all five contexts, including
-- short Council Island/Renraku labels that expose forced-break differences.
local delivery_offer = {
    id = "package_delivery",
    payment = { base_amount = 400, negotiated_amount = 450 },
}
local delivery_route_cases = 0
local function check_delivery_route(origin_location_id,
                                    destination_location_id)
    local origin = assert(RunSupport.location_details(origin_location_id))
    local destination =
        assert(RunSupport.location_details(destination_location_id))
    delivery_offer.route = {
        origin = { location_id = origin_location_id },
        destination = { location_id = destination_location_id },
    }
    game.runs.registry.current_offer = delivery_offer
    assert(PackageDelivery.present_offer() == "accepted:yes")
    assert(presented ~= nil and presented.continuation == true)
    local expected = "Go to " .. origin.name .. " " ..
        origin.region_suffix .. "  " ..
        "A contact there will give you a package.||Take it to " ..
        destination.name .. " " .. destination.region_suffix .. "  " ..
        "You will be paid upon safe delivery of the package."
    assert(presented.text == expected,
        "courier detail changed its ROM text stream for a source route")
    delivery_route_cases = delivery_route_cases + 1
end

local source_contexts = { 0, 1, 2, 3, 4 }
for _, context in ipairs(source_contexts) do
    for _, origin_region_id in ipairs(RunSupport.contract_region_ids) do
        local origin_rolls =
            RunSupport.location_choice_ids_by_region[origin_region_id]
        for _, destination_region_id in ipairs(RunSupport.contract_region_ids) do
            local destination_rolls =
                RunSupport.location_choice_ids_by_region[destination_region_id]
            local jump_house_route = context == 0
                and origin_region_id == "redmond_barrens" and
                destination_region_id == "redmond_barrens"
            local route_is_valid = jump_house_route or
                context ~= 0 and destination_region_id ~= origin_region_id
            if route_is_valid then
                for _, origin_location in ipairs(origin_rolls) do
                    for _, destination_location in ipairs(destination_rolls) do
                        if not jump_house_route or
                           origin_location ~= destination_location then
                            check_delivery_route(
                                origin_location, destination_location)
                        end
                    end
                end
            end
        end
    end
end
assert(delivery_route_cases == 12090,
    "courier route presentation did not cover the full source pool")

print("run target event tests passed: recruitment, recovery, gang entry, and 12090 courier routes")
