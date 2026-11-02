package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local E = require("enums")
local Json = require("json")
local WorldLinks = require("world_links")

local total, failed = 0, 0
local routes = {}
for _, path in ipairs(arg) do
    local file = assert(io.open(path, "rb"), "cannot read " .. path)
    local document = Json.decode(file:read("*a"))
    file:close()
    for _, interaction in ipairs(document.interactions or {}) do
        local route = interaction.route or {}
        local tasks = interaction.tasks or {}
        local prompt_route = route.kind == "dialogue" or
            type(interaction.entryDialogue) == "table" or
            type(tasks.fieldExitPrompt) == "table"
        if prompt_route then
            routes[#routes + 1] = {
                entity_id = document.entityId,
                interaction_id = interaction.id,
                source_scene = interaction.source.sceneId,
                route_kind = route.kind,
                same_scene_return_policy = route.sameSceneReturnPolicy,
            }
        end
    end
end

table.sort(routes, function(left, right)
    if left.entity_id ~= right.entity_id then
        return left.entity_id < right.entity_id
    end
    return left.interaction_id < right.interaction_id
end)

local preserved, source_door_returns = 0, 0
for _, route in ipairs(routes) do
    total = total + 1
    -- ContinueInPlace is the typed outcome returned only when the prompt did
    -- not change scenes. That outcome must discard any doorway probe saved by
    -- an earlier route and preserve the live field position and facing.
    local scene_outcome = route.same_scene_return_policy == "source_door_return"
        and E.FieldLocationSceneOutcome.SOURCE_DOOR_RETURN
        or E.FieldLocationSceneOutcome.SAME_SCENE
    local policy = WorldLinks.building_service_exit_policy{
        exit_id = route.entity_id .. "." .. route.interaction_id,
        scene_outcome = scene_outcome,
    }
    local is_preserved = policy.return_anchor == E.FieldLocationReturnAnchor.NONE and
        policy.return_facing == E.FieldLocationReturnFacing.PRESERVE_ENTRY and
        policy.return_music_policy == E.FieldLocationMusicReturnPolicy.NONE
    local source_door_return =
        policy.return_anchor == E.FieldLocationReturnAnchor.COLLISION_PROBE_LEAD and
        policy.return_facing == E.FieldLocationReturnFacing.FACE_AWAY_FROM_ENTRY and
        policy.return_music_policy ==
            E.FieldLocationMusicReturnPolicy.RESUME_SCENE_PROFILE
    if route.same_scene_return_policy == "source_door_return" then
        if source_door_return then
            source_door_returns = source_door_returns + 1
        else
            failed = failed + 1
            print("FAIL " .. route.entity_id .. "." .. route.interaction_id ..
                " lost its ROM source-door return policy")
        end
    elseif is_preserved then
        preserved = preserved + 1
    else
        failed = failed + 1
        print(string.format(
            "FAIL %s.%s scene=%d kind=%s returned=%s/%s/%s",
            route.entity_id, route.interaction_id, route.source_scene,
            tostring(route.route_kind), tostring(policy.return_anchor),
            tostring(policy.return_facing),
            tostring(policy.return_music_policy)))
    end
end

print(string.format(
    "same-map prompt returns: %d/%d preserve live position; %d source-door return",
    preserved, total - source_door_returns, source_door_returns))
assert(total > 0, "no same-map prompt/dialogue routes found in scene data")
assert(source_door_returns == 1,
    "expected the single ROM-proven Stoker source-door-return route")
assert(failed == 0, string.format(
    "%d same-map prompt routes failed their ROM-backed return policy",
    failed, total))
