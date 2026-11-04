-- Package entry point for the field obstruction grid. The grid implementation
-- stays under field/ with other field behavior; this module lets native scene
-- loading hand Lua the decoded source rectangles through the package API.
local Grid = require("field.obstruction_grid")

return {
    set_scene = Grid.set_scene,
}
