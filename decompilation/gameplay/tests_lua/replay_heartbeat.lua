-- Replays recorded heartbeat traffic (refactor/tools/heartbeat_recorder.lua) through the original actor_ai.lua
-- and the refactored actor.init, and checks both against the recording.
-- Run from the repository root; set SHADOWRUN_ROOT if the working directory differs.
-- Usage: LUA_PATH="decompilation/gameplay/scripts/?.lua;decompilation/gameplay/scripts/?/init.lua;decompilation/gameplay/scripts/tools/?.lua;;" lua5.4 decompilation/gameplay/tests_lua/replay_heartbeat.lua file.rec
local Rec = require("heartbeat_recorder")
local ser, deepcopy = Rec.serialize, Rec.deepcopy
local ROOT = os.getenv("SHADOWRUN_ROOT") or "."
-- The original monolith is the pre-split reference (bfa3908); the live actor_ai.lua is a shim to actor.init.
local ORIG = os.getenv("SHADOWRUN_ORIG_ACTOR_AI") or (ROOT .. "/decompilation/gameplay/tests_lua/reference/actor_ai_bfa3908.lua")
local recs = Rec.load(assert(arg[1], "record file"))

local function canon(v)   -- order-independent serialization for comparison
  if type(v) ~= "table" then return ser(v) end
  local keys = {}
  for k in pairs(v) do keys[#keys + 1] = k end
  table.sort(keys, function(a, b) return ser(a) < ser(b) end)
  local o = {}
  for _, k in ipairs(keys) do o[#o + 1] = ser(k) .. "=" .. canon(v[k]) end
  return "{" .. table.concat(o, ",") .. "}"
end

local function replay_game(rec, divergences)
  local i = 0
  local function svc(prefix)
    return setmetatable({}, { __index = function(_, name)
      return function(...)
        i = i + 1
        local e = rec.services[i]
        local full = prefix .. "." .. name
        if not e or e.name ~= full or canon(e.args) ~= canon(deepcopy({ n = select("#", ...), ... })) then
          divergences[#divergences + 1] = ("service #%d: got %s, recorded %s"):format(i, full, e and e.name or "none")
          return nil
        end
        return table.unpack(e.returns, 1, e.returns.n)
      end
    end })
  end
  return { actors = {}, movement = svc("movement"),
           modules = { combat = svc("combat"), entity_motion = svc("entity_motion"), items = svc("items"), magic = svc("magic") } },
         function() return i end
end

local function context_for(rec)
  local ctx = deepcopy(rec.ctx)
  local j = 0
  for k, v in pairs(ctx) do
    if v == "<fn>" then
      ctx[k] = function(...)
        repeat j = j + 1 until not rec.rng[j] or rec.rng[j].name == k
        local e = rec.rng[j]
        if not e then return nil end
        return table.unpack(e.returns, 1, e.returns.n)
      end
    end
  end
  return ctx
end

local function run(impl_loader, rec)
  local div = {}
  local g, used = replay_game(rec, div)
  for k, v in pairs(deepcopy(rec.before or {})) do g.actors[k] = v end
  _G.game = g
  local impl = impl_loader(g)
  local ctx = context_for(rec)
  local res = table.pack(pcall(impl.heartbeat, ctx))
  local out = {
    ok = res[1],
    result = res[1] and canon({ n = res.n - 1, table.unpack(res, 2, res.n) }) or nil,
    after = canon(g.actors), ctx_after = canon(ctx), services_used = used(),
  }
  return out, div
end

-- each implementation keeps one instance whose `states` table we swap in per record
local function loader_orig()
  local inst, g0
  return function(g)
    if not inst then g0 = g; inst = dofile(ORIG) end
    for k in pairs(g0.actors) do g0.actors[k] = nil end
    for k, v in pairs(g.actors) do g0.actors[k] = v end
    g.actors = g0.actors
    return inst
  end
end
local function loader_split()
  local inst, g0
  return function(g)
    if not inst then g0 = g; inst = require("actor.init") end
    for k in pairs(g0.actors) do g0.actors[k] = nil end
    for k, v in pairs(g.actors) do g0.actors[k] = v end
    g.actors = g0.actors
    return inst
  end
end
local LA, LB = loader_orig(), loader_split()
local bad_orig, bad_split, diverged = 0, 0, 0
for n, rec in ipairs(recs) do
  local want = { ok = rec.ok, result = rec.ok and canon(rec.result) or nil, after = canon(rec.after), ctx_after = canon(rec.ctx_after) }
  local a, da = run(LA, rec)
  local b, db = run(LB, rec)
  local function same(x) return x.ok == want.ok and x.result == want.result and x.after == want.after and x.ctx_after == want.ctx_after end
  if not same(a) then bad_orig = bad_orig + 1; if bad_orig <= 2 then io.stderr:write(("record %d: original differs from recording %s\n"):format(n, da[1] or "")) end end
  if not same(b) then bad_split = bad_split + 1; if bad_split <= 2 then io.stderr:write(("record %d: split differs from recording %s\n"):format(n, db[1] or "")) end end
  if #da + #db > 0 then diverged = diverged + 1 end
end
print(("records=%d original_mismatch=%d split_mismatch=%d service_divergence=%d"):format(#recs, bad_orig, bad_split, diverged))
os.exit((bad_orig == 0 and bad_split == 0) and 0 or 1)
