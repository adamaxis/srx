-- Run from decompilation/: lua ghidra_conversion/tools/gen_gameplay_enums.lua
-- Gameplay enums stay in gameplay/scripts; native selector enums live beside this generator.
local script_dir = (arg[0]:match("^(.*[/\\])") or "")
package.path = script_dir .. "?.lua;gameplay/scripts/?.lua;" .. package.path
local E = require("enums")
-- Source selector values used at the native adapter edge are deliberately
-- kept outside the gameplay script package.
for name, values in pairs(require("native_boundary_enums")) do
    assert(E[name] == nil, "native boundary enum conflicts with gameplay enum: " .. name)
    E[name] = values
end
local out = { "// enums.hpp: GENERATED from gameplay enums and tools/native_boundary_enums.lua; do not edit.",
              "#pragma once", "#include <cstdint>", "namespace srx {" }
local function format_value(value)
    if value < 0 then return string.format("%d", value) end
    return string.format("0x%X", value)
end
local names = {}
for k in pairs(E) do names[#names + 1] = k end
table.sort(names)
for _, k in ipairs(names) do
    local v = E[k]
    if type(v) == "number" then
        out[#out + 1] = string.format("inline constexpr std::int64_t %s = %s;", k, format_value(v))
    else
        local keys = {}
        local has_string_values = false
        for kk, vv in pairs(v) do
            if type(kk) == "string" and type(vv) == "number" then
                keys[#keys + 1] = kk
            elseif type(vv) == "string" then
                has_string_values = true
            end
        end
        table.sort(keys, function(a, b) return v[a] < v[b] or (v[a] == v[b] and a < b) end)
        if #keys > 0 then
            out[#out + 1] = string.format("enum class %s : std::int64_t {", k)
            for _, kk in ipairs(keys) do out[#out + 1] = string.format("    %s = %s,", kk, format_value(v[kk])) end
            out[#out + 1] = "};"
        else
            local kind = has_string_values and "string-valued gameplay data" or "index-keyed table"
            out[#out + 1] = string.format("// %s: %s, see enums.lua", k, kind)
        end
    end
end
out[#out + 1] = "template <class T> constexpr std::int64_t v(T e) { return static_cast<std::int64_t>(e); }  // enum -> value"
out[#out + 1] = "}  // namespace srx"
print(table.concat(out, "\n"))
