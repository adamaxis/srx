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

-- Page staging, in VBlanks. 0xCF38 -> 0xCFFE redraws the frame and decodes
-- the 0xC1064 font (0xC0C2) before any text: the status text shows 4 frames
-- after the frame and sidebar (emulator +8122 frame, +8126 text), and each
-- return to Options (0xCF4E after 0xEF08 or the Check Out No path) keeps
-- the previous page for 4 frames (emulator press +8200 -> Options +8204,
-- No +8400 -> +8404). Options -> Check Out (0xDA6A) has no redraw. [C]
function Hotel.presentation()
    return {
        status_text_delay_frames = "4",
        options_redraw_frames = "4",
    }
end

return Hotel
