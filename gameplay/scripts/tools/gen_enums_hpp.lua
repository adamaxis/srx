-- Generates enums.hpp from enums.lua so the two never drift:  lua5.4 gen_enums_hpp.lua > enums.hpp
package.path = "./?.lua;" .. package.path
local E = require("enums")
local out = { "// enums.hpp: GENERATED from enums.lua by gen_enums_hpp.lua; do not edit. ROM sources are cited in enums.lua.",
              "#pragma once", "#include <cstdint>", "namespace srx {" }
local names = {}
for k in pairs(E) do names[#names + 1] = k end
table.sort(names)
for _, k in ipairs(names) do
    local v = E[k]
    if type(v) == "number" then
        out[#out + 1] = string.format("inline constexpr std::int64_t %s = 0x%X;", k, v)
    else
        local keys = {}
        for kk, vv in pairs(v) do if type(kk) == "string" and type(vv) == "number" then keys[#keys + 1] = kk end end
        table.sort(keys, function(a, b) return v[a] < v[b] or (v[a] == v[b] and a < b) end)
        if #keys > 0 then
            out[#out + 1] = string.format("enum class %s : std::int64_t {", k)
            for _, kk in ipairs(keys) do out[#out + 1] = string.format("    %s = 0x%X,", kk, v[kk]) end
            out[#out + 1] = "};"
        else
            out[#out + 1] = string.format("// %s: index-keyed table, see enums.lua", k)
        end
    end
end
out[#out + 1] = "template <class T> constexpr std::int64_t v(T e) { return static_cast<std::int64_t>(e); }  // enum -> value"
out[#out + 1] = "}  // namespace srx"
print(table.concat(out, "\n"))
