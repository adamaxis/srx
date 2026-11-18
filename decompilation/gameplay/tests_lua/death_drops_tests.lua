package.path = "decompilation/gameplay/scripts/?.lua;" .. package.path

local DeathDrops = require("actor.death_drops")
local TEST_ACTOR_WORLD_X = 8192
local TEST_ACTOR_WORLD_Y = 9216
local TEST_PICKUP_TILE_OFFSET = 128
local passed, total = 0, 0
local function check(condition, message)
    total = total + 1
    if condition then passed = passed + 1 else print("FAIL " .. message) end
end

local function key(request)
    if request == nil then return "none" end
    if request.kind == "item" then return "item:" .. request.item_id end
    if request.kind == "clips" or request.kind == "nuyen" then
        return request.kind .. ":" .. request.amount
    end
    return request.kind
end

local function context(mode, actor_kind, region, record_gate, roll)
    return {
        actor_id = "drop-table-actor",
        world_position = {
            x = TEST_ACTOR_WORLD_X,
            y = TEST_ACTOR_WORLD_Y,
        },
        death_drop = {
            field_mode = mode,
            actor_kind = actor_kind,
            region_index = region,
            record_gate_nonzero = record_gate,
            pending_scene_record = false,
            pickup_slot_available = true,
            equipped_drop_item_id = "model_101t",
            special_drop_flags = {
                gargoyle_horn = false,
                hell_hound_pelt = false,
            },
        },
        ai_random_bound = function(bound)
            check(bound == 100, "death table requests the source 0-99 roll")
            return roll
        end,
    }
end

local function check_distribution(label, mode, actor_kind, region,
                                  record_gate, expected)
    local counts = {}
    for roll = 0, 99 do
        local request = DeathDrops.resolve(
            context(mode, actor_kind, region, record_gate, roll))
        local outcome = key(request)
        counts[outcome] = (counts[outcome] or 0) + 1
        if request ~= nil then
            check(request.world_x ==
                      TEST_ACTOR_WORLD_X + TEST_PICKUP_TILE_OFFSET and
                  request.world_y ==
                      TEST_ACTOR_WORLD_Y + TEST_PICKUP_TILE_OFFSET,
                  label .. " offsets the pickup by one scene tile")
        end
    end
    for outcome, amount in pairs(expected) do
        check(counts[outcome] == amount,
              label .. " selects " .. outcome .. " for " .. amount ..
              " of 100 source rolls")
    end
end

check_distribution("resource-scene table", "resource_scene", "other", nil,
    false, {
        ["item:model_101t"] = 15, ["clips:3"] = 10,
        ["item:medkit"] = 10, ["item:frag"] = 10, none = 55,
    })
check_distribution("region zero table", "ordinary", "other", 0, false, {
    ["item:model_101t"] = 15, ["clips:2"] = 10,
    ["item:medkit"] = 10, ["nuyen:20"] = 10, none = 55,
})
for _, region in ipairs({ 1, 2 }) do
    check_distribution("region " .. region .. " table", "ordinary", "other",
        region, false, {
            ["nuyen:25"] = 15, ["item:medkit"] = 10,
            ["nuyen:10"] = 15, none = 60,
        })
end
for _, region in ipairs({ 3, 5 }) do
    check_distribution("region " .. region .. " table", "ordinary", "other",
        region, false, {
            ["item:model_101t"] = 15, ["clips:2"] = 10,
            ["nuyen:25"] = 10, ["nuyen:50"] = 15, none = 50,
        })
end
for _, region in ipairs({ 4, 6, 7, 8, 9 }) do
    check_distribution("region " .. region .. " table", "ordinary", "other",
        region, false, {
            ["item:model_101t"] = 15, ["item:medkit"] = 10,
            ["nuyen:5"] = 10, ["item:fetish"] = 15, none = 50,
        })
end
check_distribution("default actor table", "ordinary", "default_actor", nil,
    false, {
        ["nuyen:10"] = 20, ["nuyen:15"] = 15,
        ["nuyen:25"] = 10, none = 55,
    })
check_distribution("kind-seven region table", "ordinary", "default_actor",
    0, true, {
        ["item:model_101t"] = 15, ["clips:2"] = 10,
        ["item:medkit"] = 10, ["nuyen:20"] = 10, none = 55,
    })

local forbidden_rolls = 0
local function no_roll_context(changes)
    local value = context("ordinary", "other", 0, false, 0)
    value.ai_random_bound = function()
        forbidden_rolls = forbidden_rolls + 1
        return 0
    end
    for name, replacement in pairs(changes or {}) do
        if name == "special_drop_flags" then
            value.death_drop.special_drop_flags = replacement
        else
            value.death_drop[name] = replacement
        end
    end
    return value
end

check(DeathDrops.resolve(no_roll_context({ field_mode = "other_mode" })) == nil,
      "unsupported field mode does not drop")
check(DeathDrops.resolve(no_roll_context({ pending_scene_record = true })) == nil,
      "pending scene record gate blocks ordinary drops")
check(DeathDrops.resolve(no_roll_context({ pickup_slot_available = false })) == nil,
      "full pickup record pool blocks ordinary drops")
check(DeathDrops.resolve(no_roll_context({ actor_kind = "wendigo" })) == nil,
      "Wendigo ordinary drop table is excluded")
local gated_gargoyle = no_roll_context({ actor_kind = "gargoyle" })
check(DeathDrops.resolve(gated_gargoyle) == nil,
      "Gargoyle Horn remains gated by its world flag")
local gargoyle = no_roll_context({ actor_kind = "gargoyle",
    special_drop_flags = { gargoyle_horn = true, hell_hound_pelt = false } })
check(key(DeathDrops.resolve(gargoyle)) == "gargoyle_horn",
      "Gargoyle flag creates its guaranteed Horn pickup without a roll")
local hound = no_roll_context({ actor_kind = "hell_hound",
    special_drop_flags = { gargoyle_horn = false, hell_hound_pelt = true } })
local hound_request = DeathDrops.resolve(hound)
check(hound_request ~= nil and hound_request.kind == "hell_hound_pelt" and
      hound_request.linked_actor_id == hound.actor_id,
      "Hell Hound flag creates a linked guaranteed Pelt pickup")
check(forbidden_rolls == 0,
      "special and gated branches do not consume the source RNG")

local inventory = { 1, 0, 0, 0, 0, 0, 0, 0 }
local item_pickup = { kind = "item", item_id = "frag" }
check(DeathDrops.needs_pickup_phase_roll(item_pickup, inventory),
      "an available grenade pickup requests its source phase roll")
local item_result = DeathDrops.collect_pickup(item_pickup, {
    inventory_item_codes = inventory,
}, 3)
check(item_result ~= nil and item_result.collected == true and
      item_result.inventory_slot == 2 and item_result.phase == 5,
      "item pickup inserts into the first empty slot and stores draw plus two")
local full_inventory = { 1, 1, 1, 1, 1, 1, 1, 1 }
check(not DeathDrops.needs_pickup_phase_roll(item_pickup, full_inventory),
      "a full inventory does not consume a phase roll")
local retained = DeathDrops.collect_pickup(item_pickup, {
    inventory_item_codes = full_inventory,
}, nil)
check(retained ~= nil and retained.collected == false,
      "a full inventory refuses the pickup so it can remain in the world")
local clip_result = DeathDrops.collect_pickup(
    { kind = "clips", amount = 3 }, { clips = 19 })
check(clip_result ~= nil and clip_result.value == 20,
      "clips clamp at twenty")
local nuyen_result = DeathDrops.collect_pickup(
    { kind = "nuyen", amount = 25 }, { nuyen = 100 })
check(nuyen_result ~= nil and nuyen_result.value == 125,
      "nuyen pickup adds its source amount")

print(string.format("death_drops tests: %d/%d passed", passed, total))
if passed ~= total then os.exit(1) end
