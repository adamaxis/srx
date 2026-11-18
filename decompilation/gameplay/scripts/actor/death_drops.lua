-- ROM 0x16B98 selects one world pickup after an ordinary actor's down list.
-- The host supplies named record facts and the source RNG service; this module
-- owns the gates, table selection, and weighted result.
local DeathDrops = {}

local MODE_RESOURCE_SCENE = "resource_scene"
local MODE_ORDINARY = "ordinary"
local RANDOM_BOUND = 100
local POSITION_TILE_OFFSET = 128
local POSITION_WORD_MODULUS = 65536
local ITEM_IDS = require("data.items").catalog_order
local ITEM_ID_SET = {}
for _, item_id in ipairs(ITEM_IDS) do ITEM_ID_SET[item_id] = true end
local ITEM_SOURCE_CODES = {}
for code, item_id in ipairs(ITEM_IDS) do
    ITEM_SOURCE_CODES[item_id] = code
end

local ITEM_SLOT_COUNT = 8
local GRENADE_FIRST_SOURCE_CODE = 13
local GRENADE_LAST_SOURCE_CODE = 19
local CLIP_COUNT_LIMIT = 20
local NUYEN_WORD_MODULUS = 4294967296

local function result(kind, weight, value)
    return { kind = kind, weight = weight, value = value }
end

local EQUIPPED = "equipped_item"
local NONE = "none"

local MODE_TWO = {
    result(EQUIPPED, 15),
    result("clips", 10, 3),
    result("item", 10, "medkit"),
    result("item", 10, "frag"),
    result(NONE, 55),
}

local REGION_TABLES = {
    [0] = {
        result(EQUIPPED, 15), result("clips", 10, 2),
        result("item", 10, "medkit"), result("nuyen", 10, 20),
        result(NONE, 55),
    },
    [1] = {
        result("nuyen", 15, 25), result("item", 10, "medkit"),
        result("nuyen", 15, 10), result(NONE, 60),
    },
    [2] = {
        result("nuyen", 15, 25), result("item", 10, "medkit"),
        result("nuyen", 15, 10), result(NONE, 60),
    },
    [3] = {
        result(EQUIPPED, 15), result("clips", 10, 2),
        result("nuyen", 10, 25), result("nuyen", 15, 50),
        result(NONE, 50),
    },
    [4] = {
        result(EQUIPPED, 15), result("item", 10, "medkit"),
        result("nuyen", 10, 5), result("item", 15, "fetish"),
        result(NONE, 50),
    },
    [5] = {
        result(EQUIPPED, 15), result("clips", 10, 2),
        result("nuyen", 10, 25), result("nuyen", 15, 50),
        result(NONE, 50),
    },
    [6] = {
        result(EQUIPPED, 15), result("item", 10, "medkit"),
        result("nuyen", 10, 5), result("item", 15, "fetish"),
        result(NONE, 50),
    },
    [7] = {
        result(EQUIPPED, 15), result("item", 10, "medkit"),
        result("nuyen", 10, 5), result("item", 15, "fetish"),
        result(NONE, 50),
    },
    [8] = {
        result(EQUIPPED, 15), result("item", 10, "medkit"),
        result("nuyen", 10, 5), result("item", 15, "fetish"),
        result(NONE, 50),
    },
    [9] = {
        result(EQUIPPED, 15), result("item", 10, "medkit"),
        result("nuyen", 10, 5), result("item", 15, "fetish"),
        result(NONE, 50),
    },
}

local DEFAULT_ACTOR = {
    result("nuyen", 20, 10), result("nuyen", 15, 15),
    result("nuyen", 10, 25), result(NONE, 55),
}

local function weighted_entry(entries, roll)
    if type(entries) ~= "table" or type(roll) ~= "number" or
       roll < 0 or roll >= 100 then
        return nil
    end
    local remaining = roll
    for _, entry in ipairs(entries) do
        if remaining < entry.weight then return entry end
        remaining = remaining - entry.weight
    end
    return nil
end

local function add_scene_tile(position)
    if type(position) ~= "number" then return nil end
    return (position + POSITION_TILE_OFFSET) % POSITION_WORD_MODULUS
end

function DeathDrops.resolve(context)
    if type(context) ~= "table" or type(context.death_drop) ~= "table" or
       context.death_drop.pickup_slot_available ~= true then
        return nil
    end
    local facts = context.death_drop
    local position = context.world_position
    if type(position) ~= "table" or add_scene_tile(position.x) == nil or
       add_scene_tile(position.y) == nil then
        return nil
    end

    local reward
    if facts.actor_kind == "gargoyle" then
        if facts.special_drop_flags and
           facts.special_drop_flags.gargoyle_horn == true then
            reward = result("gargoyle_horn", 100)
        else
            return nil
        end
    elseif facts.actor_kind == "hell_hound" then
        if facts.special_drop_flags and
           facts.special_drop_flags.hell_hound_pelt == true then
            reward = result("hell_hound_pelt", 100)
        else
            return nil
        end
    else
        if facts.actor_kind == "wendigo" or facts.pending_scene_record == true or
           (facts.field_mode ~= MODE_ORDINARY and
            facts.field_mode ~= MODE_RESOURCE_SCENE) then
            return nil
        end
        if facts.actor_kind == "default_actor" and
           type(facts.record_gate_nonzero) ~= "boolean" then
            return nil
        end

        local entries
        if facts.field_mode == MODE_RESOURCE_SCENE then
            entries = MODE_TWO
        elseif facts.actor_kind == "default_actor" and
               facts.record_gate_nonzero ~= true then
            entries = DEFAULT_ACTOR
        else
            entries = REGION_TABLES[facts.region_index]
        end
        if entries == nil or type(context.ai_random_bound) ~= "function" then
            return nil
        end
        local roll = context.ai_random_bound(RANDOM_BOUND)
        reward = weighted_entry(entries, roll)
    end

    if reward == nil or reward.kind == NONE then return nil end
    if reward.kind == EQUIPPED then
        if type(facts.equipped_drop_item_id) ~= "string" or
           facts.equipped_drop_item_id == "" then
            return nil
        end
        reward = result("item", 100, facts.equipped_drop_item_id)
    end

    local request = {
        kind = reward.kind,
        world_x = add_scene_tile(position.x),
        world_y = add_scene_tile(position.y),
    }
    if reward.kind == "item" then
        if type(reward.value) ~= "string" or
           ITEM_ID_SET[reward.value] ~= true then
            return nil
        end
        request.item_id = reward.value
    elseif reward.kind == "clips" or reward.kind == "nuyen" then
        request.amount = reward.value
    elseif reward.kind == "hell_hound_pelt" then
        request.linked_actor_id = context.actor_id
    end
    return request
end

function DeathDrops.needs_pickup_phase_roll(pickup, inventory_item_codes)
    if type(pickup) ~= "table" or pickup.kind ~= "item" or
       type(pickup.item_id) ~= "string" or
       type(inventory_item_codes) ~= "table" then
        return false
    end
    local has_empty_slot = false
    for slot = 1, ITEM_SLOT_COUNT do
        if inventory_item_codes[slot] == 0 then
            has_empty_slot = true
            break
        end
    end
    local source_code = ITEM_SOURCE_CODES[pickup.item_id]
    return has_empty_slot and source_code ~= nil and
           source_code >= GRENADE_FIRST_SOURCE_CODE and
           source_code <= GRENADE_LAST_SOURCE_CODE
end

function DeathDrops.collect_pickup(pickup, runner, phase_roll)
    if type(pickup) ~= "table" or type(runner) ~= "table" then
        return nil
    end
    if pickup.kind == "item" then
        local source_code = ITEM_SOURCE_CODES[pickup.item_id]
        if source_code == nil or type(runner.inventory_item_codes) ~= "table" then
            return nil
        end
        for slot = 1, ITEM_SLOT_COUNT do
            if runner.inventory_item_codes[slot] == 0 then
                local phase = 0
                if source_code >= GRENADE_FIRST_SOURCE_CODE and
                   source_code <= GRENADE_LAST_SOURCE_CODE then
                    if type(phase_roll) ~= "number" or phase_roll < 0 or
                       phase_roll >= 5 then
                        return nil
                    end
                    phase = phase_roll + 2
                end
                return {
                    collected = true,
                    kind = "item",
                    inventory_slot = slot,
                    item_id = pickup.item_id,
                    phase = phase,
                }
            end
        end
        return { collected = false, kind = "item" }
    elseif pickup.kind == "clips" then
        local current = runner.clips
        local amount = pickup.amount
        if type(current) ~= "number" or type(amount) ~= "number" or
           current < 0 or current > 255 or amount < 0 or amount > 255 then
            return nil
        end
        local updated = (current + amount) % 256
        local compared_value = updated >= 128 and updated - 256 or updated
        if compared_value >= CLIP_COUNT_LIMIT + 1 then
            updated = CLIP_COUNT_LIMIT
        end
        return { collected = true, kind = "clips", value = updated }
    elseif pickup.kind == "nuyen" then
        local current = runner.nuyen
        local amount = pickup.amount
        if type(current) ~= "number" or type(amount) ~= "number" or
           current < 0 or current >= NUYEN_WORD_MODULUS or
           amount < 0 or amount > 4095 then
            return nil
        end
        return {
            collected = true,
            kind = "nuyen",
            value = (current + amount) % NUYEN_WORD_MODULUS,
        }
    elseif pickup.kind == "gargoyle_horn" or
           pickup.kind == "hell_hound_pelt" then
        return {
            collected = true,
            kind = pickup.kind,
            linked_actor_id = pickup.linked_actor_id,
        }
    end
    return nil
end

return DeathDrops
