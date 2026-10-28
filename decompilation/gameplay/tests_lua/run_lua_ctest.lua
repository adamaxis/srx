local root = assert(arg[1], "repository root is required")
local suite = assert(arg[2], "Lua suite path is required")
root = root:gsub("\\", "/")

package.path = table.concat({
    root .. "/decompilation/gameplay/scripts/?.lua",
    root .. "/decompilation/gameplay/scripts/?/init.lua",
    root .. "/decompilation/gameplay/scripts/tools/?.lua",
    root .. "/decompilation/gameplay/tests_lua/?.lua",
    package.path,
}, ";")

dofile(suite)
