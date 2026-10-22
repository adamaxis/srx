package.path = "decompilation/gameplay/tests_lua/?.lua;" .. package.path
local Harness = require("support.vid_phone_graph_harness")
local test = Harness.new("professor_jefferson")

local function reset(called, choices, results)
    test:reset(called, choices, results)
end

reset(false, { "goodbye" })
test:expect_finished()
assert(test.menus[1].id == "call_professor_jefferson_first_call")
assert(game.state.vid_phone_called_12 == true)
test:expect_page("goodbye")

-- The lesson is a three-page ordered chain. Its final page can return to
-- Services for either offer or end the call.
reset(true, { "services", "paranormal_creatures", "more", "more",
              "services", "former_student", "decline", "club", "more",
              "goodbye" })
test:expect_finished()
assert(test.menus[1].id == "call_professor_jefferson_repeat_call")
for _, menu_id in ipairs({ "professor_creature_page_1",
                           "professor_creature_page_2",
                           "professor_creature_page_3" }) do
    test:expect_menu(menu_id)
end
test:expect_menu("professor_services")
test:expect_menu("professor_student_offer")
test:expect_page("goodbye")
assert(#test.operations == 0)

-- Club and former-student offers each support decline and Goodbye directly.
reset(true, { "services", "club", "more", "decline", "club", "more",
              "goodbye" })
test:expect_finished()
test:expect_menu("professor_club_offer")
test:expect_menu("professor_services")
test:expect_page("goodbye")

reset(true, { "services", "former_student", "goodbye" })
test:expect_finished()
test:expect_menu("professor_student_offer")
test:expect_page("goodbye")

-- The club transaction is command 0x88. Its success updates Riannon's
-- discount flag and presents the ROM acknowledgement page.
reset(true, { "services", "club", "more", "accept" },
      { purchase_riannons_discount = "discount_enabled" })
test:expect_finished()
test:expect_transaction("purchase_riannons_discount")
test:expect_page("club_success")

-- The former-student offer is command 0x89; success opens its own Services /
-- Goodbye continuation so the new Gregory contact can be followed up.
reset(true, { "services", "former_student", "accept", "services", "club",
              "more", "goodbye" },
      { hire_gregory_wilns = "hired" })
test:expect_finished()
test:expect_transaction("hire_gregory_wilns")
test:expect_menu("professor_student_result")
test:expect_menu("professor_services")
test:expect_page("goodbye")

for _, operation in ipairs({ "purchase_riannons_discount",
                             "hire_gregory_wilns" }) do
    local choice = operation == "purchase_riannons_discount" and "club" or
        "former_student"
    local prefix = { "services", choice }
    if choice == "club" then prefix[#prefix + 1] = "more" end
    prefix[#prefix + 1] = "accept"
    reset(true, prefix, { [operation] = "insufficient" })
    local boundary = test:expect_boundary()
    test:expect_transaction(operation)
    assert(boundary.reason:match("payment_text_boundary") ~= nil)
    assert(#test.pages == 0,
        "unreadable ROM pointer must not be presented as dialogue text")
end

for _, operation in ipairs({ "purchase_riannons_discount",
                             "hire_gregory_wilns" }) do
    local choice = operation == "purchase_riannons_discount" and "club" or
        "former_student"
    local prefix = { "services", choice }
    if choice == "club" then prefix[#prefix + 1] = "more" end
    prefix[#prefix + 1] = "accept"
    for _, result in ipairs({ "unavailable", "unknown_result" }) do
        local choices = {}
        for _, choice_id in ipairs(prefix) do
            choices[#choices + 1] = choice_id
        end
        reset(true, choices, { [operation] = result })
        local boundary = test:expect_boundary()
        test:expect_transaction(operation)
        assert(boundary.reason:match("service_boundary") ~= nil)
    end
end

reset(nil, {})
local boundary = test:expect_boundary()
assert(boundary.reason:match("branch_condition_unknown") ~= nil)

print("Professor Jefferson Vid-Phone JSON graph permutations passed")
