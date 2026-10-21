package.path = "decompilation/gameplay/scripts/?.lua;" .. package.path

local EmployeeRecruitment = require("run_employee_recruitment")
local PackageRecovery = require("run_package_recovery")
local GangContract = require("run_gang_contract")
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
    runs = { registry = { active_offer = active_offer } },
    ui = {
        say_text = function(text, continuation)
            presented = { text = text, continuation = continuation }
        end,
    },
}
assert(EmployeeRecruitment.present_target_entry() == "shown")
assert(presented ~= nil and presented.continuation == false)
assert(presented.text ==
    "<You create a diversion and slip into the office building.>\n\n" ..
    "Now comes the hard part.\n\n" ..
    "Finding your quarry and escaping with everyone intact!>")

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

print("run target event tests passed: recruitment, recovery, and gang entry")
