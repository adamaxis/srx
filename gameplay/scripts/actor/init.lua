-- actor/init.lua: loads the split modules in dependency order and returns the ActorAI table (same API as before).
require("actor.tables")
require("actor.targeting")
require("actor.anim")
require("actor.actions")
require("actor.movement")
return require("actor.heartbeat")
