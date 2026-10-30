-- Semantic group-item ownership. The host translates these stable IDs to and
-- from the save format; gameplay scripts do not use the source bit positions.
local E = require("enums")

local GroupItems = {}
local known = {}

for _, item_id in pairs(E.GroupItemId) do
    known[item_id] = true
end

function GroupItems.is_known(item_id)
    return type(item_id) == "string" and known[item_id] == true
end

function GroupItems.fact_id(item_id)
    assert(GroupItems.is_known(item_id),
        "unknown group item: " .. tostring(item_id))
    return "group_item." .. item_id
end

-- Returns true/false, or nil while the host has not restored this item yet.
function GroupItems.has(item_id)
    return game.state[GroupItems.fact_id(item_id)]
end

function GroupItems.set(item_id, owned)
    assert(type(owned) == "boolean",
        "group item ownership must be boolean")
    game.state[GroupItems.fact_id(item_id)] = owned
    return true
end

return GroupItems
