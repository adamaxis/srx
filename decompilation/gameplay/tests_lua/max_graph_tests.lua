package.path = "decompilation/gameplay/tests_lua/?.lua;" .. package.path
local Harness = require("support.vid_phone_graph_harness")
local test = Harness.new("max")

local function reset(called, choices, results)
    test:reset(called, choices, results)
end

-- First calls set the source history bit before presenting Services/Goodbye.
reset(false, { "goodbye" })
test:expect_finished()
assert(test.menus[1].id == "call_max_first_call")
assert(game.state.vid_phone_called_10 == true)
test:expect_page("goodbye")

-- Repeated calls use their own copy, then every category can return to the
-- common Services/Goodbye panel without invoking a transaction.
reset(true, { "services", "hardware", "more", "return_to_services",
              "goodbye" })
test:expect_finished()
assert(test.menus[1].id == "call_max_repeat_call")
test:expect_menu("max_hardware_intro")
test:expect_menu("max_hardware_categories")
test:expect_menu("max_service_return")
assert(#test.operations == 0)
test:expect_page("goodbye")

reset(true, { "services", "corporate_badge", "different_subject",
              "goodbye" })
test:expect_finished()
test:expect_menu("max_badge_offer")
test:expect_menu("max_service_return")
assert(#test.operations == 0)

local items = {
    { category = "smgs", choice = "hk227", operation = "purchase_max_hk227_s" },
    { category = "smgs", choice = "ak97", operation = "purchase_ak97" },
    { category = "passkeys", choice = "level_4",
      operation = "purchase_max_maglock_passkey_level_4" },
    { category = "passkeys", choice = "level_5",
      operation = "purchase_max_maglock_passkey_level_5" },
}

-- All four item commands share explicit success, low-funds, inventory-full,
-- and unresolved-service edges while keeping their distinct ROM command IDs.
for _, item in ipairs(items) do
    reset(true, { "services", "hardware", "more", item.category,
                  item.choice, "goodbye" },
          { [item.operation] = "purchased" })
    test:expect_finished()
    test:expect_transaction(item.operation)
    test:expect_menu("max_purchase_result")
    test:expect_page("goodbye")
end

for _, item in ipairs(items) do
    reset(true, { "services", "hardware", "more", item.category,
                  item.choice },
          { [item.operation] = "insufficient" })
    test:expect_finished()
    test:expect_transaction(item.operation)
    test:expect_page("insufficient_balance")
end

for _, item in ipairs(items) do
    reset(true, { "services", "hardware", "more", item.category,
                  item.choice, "goodbye" },
          { [item.operation] = "out_of_stock" })
    test:expect_finished()
    test:expect_transaction(item.operation)
    test:expect_menu("max_out_of_stock")
    test:expect_page("goodbye")
end

for _, result in ipairs({ "unavailable", "unknown_result" }) do
    reset(true, { "services", "hardware", "more", "smgs", "hk227" },
          { purchase_max_hk227_s = result })
    test:expect_boundary()
    test:expect_transaction("purchase_max_hk227_s")
end

-- Corporate badge has no stock result: its ROM handler charges 6,000 and
-- sets the badge flag only on success.
reset(true, { "services", "corporate_badge", "accept_badge", "goodbye" },
      { issue_max_corporate_badge = "badge_issued" })
test:expect_finished()
test:expect_transaction("issue_max_corporate_badge")
test:expect_menu("max_badge_result")
test:expect_page("goodbye")

reset(true, { "services", "corporate_badge", "accept_badge" },
      { issue_max_corporate_badge = "insufficient" })
test:expect_finished()
test:expect_transaction("issue_max_corporate_badge")
test:expect_page("insufficient_balance")

for _, result in ipairs({ "unavailable", "unknown_result" }) do
    reset(true, { "services", "corporate_badge", "accept_badge" },
          { issue_max_corporate_badge = result })
    test:expect_boundary()
end

-- Unresolved call history or host data never falls through to invented copy.
reset(nil, {})
local boundary = test:expect_boundary()
assert(boundary.reason:match("branch_condition_unknown") ~= nil)

print("Max Vid-Phone JSON graph permutations passed")
