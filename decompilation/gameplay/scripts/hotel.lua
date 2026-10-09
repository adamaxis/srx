-- The native Hotel presenter owns layout, focus, and controller cadence.
-- Lua supplies the stable choices used by its options and checkout pages.
local Hotel = {}

function Hotel.options()
    return { "karma_screen", "check_out" }
end

function Hotel.checkout()
    return { "checkout_yes", "checkout_no" }
end

return Hotel
