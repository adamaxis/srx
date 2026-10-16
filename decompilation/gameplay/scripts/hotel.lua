-- The native Hotel presenter owns layout, focus, and controller cadence.
-- Lua supplies the stable choices used by its options and checkout pages.
local Hotel = {}

function Hotel.options()
    return { "karma_screen", "check_out" }, "hotel.options.title", {
        "hotel.option.karma_screen",
        "hotel.option.check_out",
    }
end

function Hotel.checkout()
    return { "checkout_yes", "checkout_no" }, "hotel.checkout.title", {
        "hotel.option.checkout_yes",
        "hotel.option.checkout_no",
    }
end

function Hotel.text_keys()
    return {
        options_separator = "hotel.options.separator",
        medical_attention_suffix = "hotel.status.medical_attention_suffix",
        recovery_2 = "hotel.status.recovery.2",
        recovery_3 = "hotel.status.recovery.3",
        recovery_4 = "hotel.status.recovery.4",
    }
end

return Hotel
