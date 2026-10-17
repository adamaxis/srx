-- menu/init.lua: `require("menu")` entry point. The live registration list is
-- menu/field_init.lua (the one GameScriptRuntime loads); keep a single list.
return require("menu.field_init")
