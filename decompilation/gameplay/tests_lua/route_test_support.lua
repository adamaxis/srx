-- Test fixture adapter for source-derived scene routing inputs. Production
-- Lua receives the host's resolved transition object; only these legacy test
-- fixtures retain the source table/index shape while cases are migrated.
local M = {}

local function transition_from_fixture(context)
    local index = context.event_index
    if type(index) ~= "number" and type(context.event_code) == "string" then
        index = tonumber(context.event_code:match("^event_(%d+)$"))
    end
    if context.event_table_available == true and
       type(context.event_table) == "table" and type(index) == "number" then
        local destination = context.event_table[index + 1]
        if type(destination) == "number" then
            return { kind = "enter_scene", destination_scene_id = destination }
        end
        return { kind = "no_destination" }
    end
    if context.event_table_available == false and index == 0 then
        return { kind = "return_to_field" }
    end
    return { kind = "no_destination" }
end

function M.wrap(building)
    return setmetatable({
        resolve_field_location_route = function(context)
            local semantic_context = {}
            for key, value in pairs(context) do
                if key ~= "event_index" and key ~= "event_table_available" and
                   key ~= "event_table" then
                    semantic_context[key] = value
                end
            end
            semantic_context.transition = transition_from_fixture(context)
            return building.resolve_field_location_route(semantic_context)
        end,
    }, { __index = building })
end

return M
