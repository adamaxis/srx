# Parked (not built, not loaded)
Moved here from `decompilation/gameplay/scripts/menu/` by the grok-fixes branch. The "last commit" (d2b21f8) dropped them in unwired.
- `save.lua`: the ROM SRAM format 0x596C4/0x59360. The project uses JSON host snapshots instead (owner decision), so nothing loads it. Keep it as a reference for `.srm` import/export.
- `menu_binding.hpp`, `*.sketch.cpp`: the original MenuGame/MatrixGame interface sketch. The live boundary is `ScriptMenuRouteBindings` in `include/shadowrun_ghidra/scripting/game_script_runtime.hpp`.
