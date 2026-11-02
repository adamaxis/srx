local E = require("enums")
local DialogueFramework = require("dialogue_framework")

local function equal(actual, expected, message)
    assert(actual == expected,
        (message or "value mismatch") .. ": expected " .. tostring(expected) ..
        ", got " .. tostring(actual))
end

local defaults = DialogueFramework.source_defaults()
local source_count = 0
for _, source in pairs(E.DialogueSource) do
    source_count = source_count + 1
    assert(defaults[source], "dialogue source is missing a default: " .. source)
end
local default_count = 0
for _ in pairs(defaults) do default_count = default_count + 1 end
equal(default_count, source_count, "every dialogue source has one default")

local contact = DialogueFramework.describe(
    E.DialogueSource.FIELD_NPC_CONTACT, "field_generic_contacts.present")
equal(contact.scene_outcome, E.FieldLocationSceneOutcome.SAME_SCENE,
    "NPC contact stays in its scene")
equal(contact.exit_behavior, E.DialogueExitBehavior.STAY,
    "NPC contact has a stay-in-place close")
equal(contact.input_lock, E.DialogueInputLock.WORLD_ACTIVITY_FFE11C,
    "generic contact declares the shared input lock")

local exit = DialogueFramework.describe(
    E.DialogueSource.BUILDING_EXIT, "michael_package.present_field_exit_prompt")
equal(exit.scene_outcome, E.FieldLocationSceneOutcome.SOURCE_DOOR_RETURN,
    "building exit returns through its source door")
equal(exit.return_anchor, E.FieldLocationReturnAnchor.COLLISION_PROBE_LEAD,
    "building exit uses the source door anchor")
equal(exit.return_facing,
    E.FieldLocationReturnFacing.FACE_AWAY_FROM_ENTRY,
    "building exit faces away from its entry")
equal(exit.music_return_policy,
    E.FieldLocationMusicReturnPolicy.RESUME_SCENE_PROFILE,
    "building exit resumes the scene music profile")

local routed = DialogueFramework.describe(
    E.DialogueSource.FIELD_EVENT, "fixture.stays",
    { scene_outcome = E.FieldLocationSceneOutcome.SAME_SCENE,
      exit_behavior = E.DialogueExitBehavior.STAY,
      transition = E.DialogueTransition.NONE })
equal(routed.scene_outcome, E.FieldLocationSceneOutcome.SAME_SCENE,
    "route data can select the same-scene outcome")
local unlocked = DialogueFramework.describe(
    E.DialogueSource.SCRIPTED_ENCOUNTER, "fixture.unlocked")
equal(unlocked.input_lock, E.DialogueInputLock.NONE,
    "a dialogue declares an explicit no-lock policy")
equal(unlocked.return_anchor, E.FieldLocationReturnAnchor.NONE,
    "a dialogue declares no source-door return by default")
equal(unlocked.music_return_policy, E.FieldLocationMusicReturnPolicy.NONE,
    "a dialogue declares no music resume by default")

local prior_game = rawget(_G, "game")
_G.game = {}
package.preload.dialogue_framework_fixture = function()
    return {
        run = function()
            equal(game.dialogue.active.source,
                E.DialogueSource.RANDOM_ENCOUNTER,
                "opening source is active in the task")
            equal(game.dialogue.active.scene_outcome,
                E.FieldLocationSceneOutcome.SAME_SCENE,
                "opening applies its route outcome")
            local resumed, answer = coroutine.yield("fixture-page")
            assert(resumed)
            equal(answer, "continue", "task resumes through its open call")
            equal(game.dialogue.active.route_id,
                "dialogue_framework_fixture.run",
                "route stays active for every yielded page")
            return "fixture-complete"
        end,
    }
end

local task = coroutine.create(function()
    return DialogueFramework.open(
        E.DialogueSource.RANDOM_ENCOUNTER,
        "dialogue_framework_fixture", "run",
        E.FieldLocationSceneOutcome.SAME_SCENE,
        E.DialogueExitBehavior.STAY,
        E.DialogueTransition.NONE)
end)
local started, page = coroutine.resume(task)
assert(started, page)
equal(page, "fixture-page", "task yields its first page")
equal(game.dialogue.open_count, 1, "framework records the opening")
equal(game.dialogue.active.route_id,
    "dialogue_framework_fixture.run", "framework retains active route")
local resumed, result = coroutine.resume(task, true, "continue")
assert(resumed, result)
equal(result, "fixture-complete", "framework returns the task result")
equal(game.dialogue.active, nil, "framework clears active route on return")
equal(game.dialogue.last_opening.source,
    E.DialogueSource.RANDOM_ENCOUNTER,
    "framework records the completed opening")

package.preload.dialogue_framework_fixture = nil
_G.game = prior_game

-- Runtime task starts in the Qt host must use the one typed dialogue entry.
-- The remaining direct generic task start is the non-dialogue field event.
local function read_file(path)
    local file = assert(io.open(path, "rb"), "could not read " .. path)
    local contents = file:read("*a")
    file:close()
    return contents
end

local opener_path =
    "decompilation/ghidra_conversion/qt_runtime/src/dialogue/field_dialogue_runtime.cpp"
local opener_source = read_file(opener_path)
assert(opener_source:find("script_runtime_->start_dialogue_task(", 1, true),
    "FieldDialogueRuntime must invoke the Lua dialogue framework")
assert(not opener_source:find("script_runtime_->start_task(", 1, true),
    "FieldDialogueRuntime must not start raw Lua tasks")

local direct_starts = assert(io.popen(
    "rg -n '\\.start_task[[:space:]]*\\(' " ..
    "decompilation/ghidra_conversion/qt_runtime/src", "r"))
local allowed_non_dialogue_start = 0
for line in direct_starts:lines() do
    if line:find(
        "field_scene_runtime_transitions.cpp:", 1, true) and
       line:find('game_scripts_.start_task("field_events", activation.handler)',
                 1, true) then
        allowed_non_dialogue_start = allowed_non_dialogue_start + 1
    else
        error("raw C++ task start bypasses dialogue framework: " .. line)
    end
end
local _, _, command_status = direct_starts:close()
assert(command_status == 0 or command_status == true,
    "could not scan Qt runtime task starts")
equal(allowed_non_dialogue_start, 1,
    "only the known non-dialogue field event uses a raw task start")

print("dialogue framework opening contract passed")
