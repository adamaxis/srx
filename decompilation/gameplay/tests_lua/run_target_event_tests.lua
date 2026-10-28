package.path = "decompilation/gameplay/scripts/?.lua;" .. package.path

local EmployeeRecruitment = require("run_employee_recruitment")
local PackageRecovery = require("run_package_recovery")
local PackageDelivery = require("run_package_delivery")
local GangContract = require("run_gang_contract")
local RunSupport = require("run_support")
local active_offer = {
    id = "employee_recruitment",
    route = { target = { area_id = 5, location_id = 6 } },
}

local matched = EmployeeRecruitment.resolve_field_event(active_offer, 5, 6)
assert(matched.action == "entry_dialogue")
assert(matched.dialogue_module == "run_employee_recruitment")
assert(matched.dialogue_function == "present_target_entry")

for _, route in ipairs({ { 2, 1 }, { 5, 5 }, { 4, 6 } }) do
    local ignored = EmployeeRecruitment.resolve_field_event(
        active_offer, route[1], route[2])
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
    for _, target_record in ipairs(target_pool) do
        local location_name =
            RunSupport.location_names[target_record.area_id][target_record.location_id]
        local area_suffix = RunSupport.area_suffixes[target_record.area_id]
        assert(location_name ~= nil and area_suffix ~= nil)
        local displayed_area = area_suffix:gsub("%.$", ",", 1)
        for _, employee_name in ipairs(RunSupport.client_names) do
            game.runs.registry.current_offer = {
                id = "employee_recruitment",
                route = { target = {
                    location_name = location_name,
                    area_suffix = area_suffix,
                } },
                employee_name = employee_name,
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
    route = { target = { area_id = 2, location_id = 5 } },
}
local recovery_match = PackageRecovery.resolve_field_event(
    recovery_offer, 2, 5)
assert(recovery_match.action == "entry_dialogue")
assert(recovery_match.dialogue_module == "run_package_recovery")
assert(recovery_match.dialogue_function == "present_target_entry")
assert(PackageRecovery.resolve_field_event(recovery_offer, 2, 4).action ==
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
    route = { target = { area_id = 2, location_id = 9 } },
}
local gang_match = GangContract.resolve_field_event(gang_offer, 2, 9)
assert(gang_match.action == "entry_dialogue")
assert(gang_match.dialogue_module == "run_gang_contract")
assert(gang_match.dialogue_function == "present_target_entry")
assert(gang_match.actor_group_area == 2)
assert(gang_match.actor_group_event == 9)
assert(gang_match.activity_scene_id == 15)
assert(gang_match.actor_list_offset == 0x1D6D5E)
assert(gang_match.actor_resource_profile == 0x121E38)
for _, route in ipairs({ { 1, 9 }, { 2, 8 }, { 3, 4 } }) do
    assert(GangContract.resolve_field_event(
        gang_offer, route[1], route[2]).action == "no_action")
end

gang_offer.route.target = { area_id = 1, location_id = 9 }
local first_gang_match = GangContract.resolve_field_event(gang_offer, 1, 9)
assert(first_gang_match.action == "entry_dialogue")
assert(first_gang_match.actor_list_offset == 0x1D6D4A)
assert(first_gang_match.actor_resource_profile == 0x122054)
gang_offer.route.target = { area_id = 3, location_id = 4 }
local third_gang_match = GangContract.resolve_field_event(gang_offer, 3, 4)
assert(third_gang_match.action == "entry_dialogue")
assert(third_gang_match.actor_list_offset == 0x1D6D72)
assert(third_gang_match.actor_resource_profile == 0x122018)

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
local function check_delivery_route(origin_area, origin_location,
                                    destination_area, destination_location)
    local origin_name = RunSupport.location_names[origin_area][origin_location]
    local destination_name =
        RunSupport.location_names[destination_area][destination_location]
    local origin_suffix = RunSupport.area_suffixes[origin_area]
    local destination_suffix = RunSupport.area_suffixes[destination_area]
    assert(origin_name ~= nil and destination_name ~= nil and
           origin_suffix ~= nil and destination_suffix ~= nil,
        "source courier route pool has an unmapped text field")
    delivery_offer.route = {
        origin = {
            area_id = origin_area,
            location_id = origin_location,
            location_name = origin_name,
            area_suffix = origin_suffix,
        },
        destination = {
            area_id = destination_area,
            location_id = destination_location,
            location_name = destination_name,
            area_suffix = destination_suffix,
        },
    }
    game.runs.registry.current_offer = delivery_offer
    assert(PackageDelivery.present_offer() == "accepted:yes")
    assert(presented ~= nil and presented.continuation == true)
    local expected = "Go to " .. origin_name .. " " .. origin_suffix .. "  " ..
        "A contact there will give you a package.||Take it to " ..
        destination_name .. " " .. destination_suffix .. "  " ..
        "You will be paid upon safe delivery of the package."
    assert(presented.text == expected,
        "courier detail changed its ROM text stream for a source route")
    delivery_route_cases = delivery_route_cases + 1
end

local source_contexts = { 0, 1, 2, 3, 4 }
for _, context in ipairs(source_contexts) do
    for origin_area = 0, 5 do
        local origin_rolls = RunSupport.location_rolls_by_area[origin_area]
        for destination_area = 0, 5 do
            local jump_house_route = context == 0
                and origin_area == 2 and destination_area == 2
            local route_is_valid = jump_house_route or
                context ~= 0 and destination_area ~= origin_area
            if route_is_valid then
                local destination_rolls =
                    RunSupport.location_rolls_by_area[destination_area]
                for _, origin_location in ipairs(origin_rolls) do
                    for _, destination_location in ipairs(destination_rolls) do
                        if not jump_house_route or
                           origin_location ~= destination_location then
                            check_delivery_route(
                                origin_area, origin_location,
                                destination_area, destination_location)
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
