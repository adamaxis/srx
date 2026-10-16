-- cursor.lua: shared row cursor rule used by ROM 0xC376.
-- The ROM adds the requested step as a signed byte, wraps a negative sum to
-- the caller's maximum row, then wraps a signed (maximum - candidate) result
-- to row zero. The paired cursor-cell erase/draw remains a presenter concern.
local M = {}
local BYTE_MODULUS = 256
local BYTE_SIGN = 128

local function signed_byte(value)
    value = value & (BYTE_MODULUS - 1)
    if value >= BYTE_SIGN then return value - BYTE_MODULUS end
    return value
end

function M.step(current, delta, maximum)
    local candidate = (current + delta) & (BYTE_MODULUS - 1)
    if signed_byte(current) + signed_byte(delta) < 0 then
        return maximum & (BYTE_MODULUS - 1)
    end
    if signed_byte((maximum - candidate) & (BYTE_MODULUS - 1)) < 0 then
        return 0
    end
    return candidate
end

return M
