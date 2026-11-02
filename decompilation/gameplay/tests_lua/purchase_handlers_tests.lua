package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local E = require("enums")
local DialogueActions = require("dialogue_actions")

local action_ids = {
    heavy_combat_armor = "building.purchase_heavy_combat_armor",
    power_focus_phase4 = "building.purchase_power_focus_phase4",
}
local registry = DialogueActions.new_default_registry()

local function copy_slots(values)
    local copy = {}
    for slot = 0, 7 do copy[slot] = values and values[slot] or 0 end
    return copy
end

local function new_game(character_class, funds, inventory, phases)
    local state = { unrelated_story_flag = true }
    local instance = {
        state = state,
        funds = funds,
        inventory = copy_slots(inventory),
        phases = copy_slots(phases),
        service_calls = {},
        party = function()
            return { [0] = { character_class = character_class } }
        end,
        active_slot = function() return 0 end,
        services = {},
    }

    instance.services.call = function(domain, operation)
        assert(domain == "building", "purchase used the wrong host service")
        local item_id, phase, amount = operation:match(
            "^purchase_runner_item_(%d+)_(%d+)_(%d+)$")
        assert(item_id ~= nil, "unexpected purchase operation: " .. operation)
        item_id, phase, amount = tonumber(item_id), tonumber(phase),
            tonumber(amount)
        instance.service_calls[#instance.service_calls + 1] = {
            item_id = item_id,
            phase = phase,
            amount = amount,
        }

        local free_slot
        for slot = 0, 7 do
            if instance.inventory[slot] == E.ItemId.NONE then
                free_slot = slot
                break
            end
        end
        -- The host service follows the source 0x22874 slot-before-0x21A4E
        -- funds order and applies the already-typed transaction atomically.
        if free_slot == nil then
            return E.BuildingPurchaseResult.INVENTORY_FULL
        end
        if instance.funds < amount then
            return E.BuildingPurchaseResult.INSUFFICIENT_FUNDS
        end

        instance.funds = instance.funds - amount
        instance.inventory[free_slot] = item_id
        instance.phases[free_slot] = phase
        return E.BuildingPurchaseResult.PURCHASED
    end

    game = instance
    return instance
end

local function run(action_id)
    local ok, result, reason = registry:execute(action_id, {}, {})
    assert(ok, reason or ("purchase action failed: " .. action_id))
    return result
end

local function assert_slots_unchanged(state, inventory, phases, funds)
    assert(state.funds == funds, "rejected purchase changed nuyen")
    for slot = 0, 7 do
        assert(state.inventory[slot] == inventory[slot],
            "rejected purchase changed item slot " .. slot)
        assert(state.phases[slot] == phases[slot],
            "rejected purchase changed phase slot " .. slot)
    end
    assert(state.state.unrelated_story_flag,
        "purchase changed an unrelated story flag")
end

assert(registry.operations[action_ids.heavy_combat_armor] ~= nil,
    "ROM command 0xEB purchase handler is not registered")
assert(registry.operations[action_ids.power_focus_phase4] ~= nil,
    "ROM command 0xEC purchase handler is not registered")

-- 0x24A1E has no class restriction. The lowest empty slot is used, and the
-- source clears its +8 phase byte when it writes item 0x32.
local state = new_game(E.RunnerClass.SAMURAI, 30000,
    { [0] = E.ItemId.AMERICAN_L36, [1] = E.ItemId.ARMOR_CLOTHING },
    { [0] = 7, [1] = 6 })
assert(run(action_ids.heavy_combat_armor) ==
       E.BuildingPurchaseResult.PURCHASED)
assert(state.funds == 0 and state.inventory[2] ==
       E.ItemId.HEAVY_COMBAT_ARMOR and state.phases[2] == 0,
       "0xEB did not charge 30,000 and write item 0x32/phase 0")
assert(state.service_calls[1].item_id == E.ItemId.HEAVY_COMBAT_ARMOR and
       state.service_calls[1].phase == 0 and
       state.service_calls[1].amount == 30000)

-- With a free slot, insufficient funds keep both wallet and inventory intact.
state = new_game(E.RunnerClass.DECKER, 29999)
local before_inventory, before_phases = copy_slots(state.inventory),
    copy_slots(state.phases)
assert(run(action_ids.heavy_combat_armor) ==
       E.BuildingPurchaseResult.INSUFFICIENT_FUNDS)
assert(#state.service_calls == 1)
assert_slots_unchanged(state, before_inventory, before_phases, 29999)

-- ROM checks slot availability before funds; a full bag returns its own page.
local full_bag = {}
for slot = 0, 7 do full_bag[slot] = E.ItemId.MEDKIT end
state = new_game(E.RunnerClass.MAGE, 0, full_bag)
before_inventory, before_phases = copy_slots(state.inventory),
    copy_slots(state.phases)
assert(run(action_ids.heavy_combat_armor) ==
       E.BuildingPurchaseResult.INVENTORY_FULL)
assert_slots_unchanged(state, before_inventory, before_phases, 0)

-- Command 0xEC admits Mage/Shaman (class bytes 0/1), and writes phase 4.
state = new_game(E.RunnerClass.MAGE, 80000,
    { [0] = E.ItemId.AMERICAN_L36, [1] = E.ItemId.ARMOR_CLOTHING,
      [2] = E.ItemId.MEDKIT, [3] = E.ItemId.STIM_PATCH,
      [4] = E.ItemId.TRAUMA_PATCH })
assert(run(action_ids.power_focus_phase4) ==
       E.BuildingPurchaseResult.PURCHASED)
assert(state.funds == 0 and state.inventory[5] == E.ItemId.POWER_FOCUS and
       state.phases[5] == 4,
       "0xEC did not charge 80,000 and write item 0x18/phase 4")
assert(state.service_calls[1].item_id == E.ItemId.POWER_FOCUS and
       state.service_calls[1].phase == 4 and
       state.service_calls[1].amount == 80000)

state = new_game(E.RunnerClass.SHAMAN, 79999)
before_inventory, before_phases = copy_slots(state.inventory),
    copy_slots(state.phases)
assert(run(action_ids.power_focus_phase4) ==
       E.BuildingPurchaseResult.INSUFFICIENT_FUNDS)
assert(#state.service_calls == 1)
assert_slots_unchanged(state, before_inventory, before_phases, 79999)

-- Class 2/3 returns the ROM's class message before slot/funds processing.
state = new_game(E.RunnerClass.SAMURAI, 80000)
before_inventory, before_phases = copy_slots(state.inventory),
    copy_slots(state.phases)
assert(run(action_ids.power_focus_phase4) ==
       E.BuildingPurchaseResult.CLASS_RESTRICTED)
assert(#state.service_calls == 0,
    "0xEC must reject class before checking inventory or nuyen")
assert_slots_unchanged(state, before_inventory, before_phases, 80000)

state = new_game(E.RunnerClass.DECKER, 0, full_bag)
before_inventory, before_phases = copy_slots(state.inventory),
    copy_slots(state.phases)
assert(run(action_ids.power_focus_phase4) ==
       E.BuildingPurchaseResult.CLASS_RESTRICTED)
assert(#state.service_calls == 0,
    "0xEC class rejection must precede its full-inventory branch")
assert_slots_unchanged(state, before_inventory, before_phases, 0)

state = new_game(E.RunnerClass.MAGE, 0, full_bag)
before_inventory, before_phases = copy_slots(state.inventory),
    copy_slots(state.phases)
assert(run(action_ids.power_focus_phase4) ==
       E.BuildingPurchaseResult.INVENTORY_FULL)
assert_slots_unchanged(state, before_inventory, before_phases, 0)

print("Purchase handler tests passed: ROM 0xEB/0xEC success, class, slot, funds, item and phase branches")
