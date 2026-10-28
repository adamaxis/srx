-- Qt owns pixel placement and VBlank collection. Lua owns the selected Hotel
-- option and the controller transition that consumes each typed input sample.
local Hotel = {}
local options_selection = 0
local checkout_selection = 1

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

function Hotel.reset_options()
    options_selection = 0
    checkout_selection = 1
    return { options_selection }
end

-- ROM 0xCF98 checks Up before Down, then 0xCFD2 dispatches A using the new
-- selection. C is checked afterward at 0xCFEA; B, Left, Right, and Start do
-- not change this menu. Event bits are the host's latched 16-bit input word.
-- Results: event code (0 waiting, 1 moved, 2 checkout, 3 Karma, 4 action),
-- selected row (0 or 1), and whether a direction moved the cursor. Direction
-- and confirm can occur in one source sample, so retain both facts.
function Hotel.advance_options(events)
    if type(events) ~= "number" or events % 1 ~= 0 or
       events < 0 or events > 0xFFFF then
        error("Hotel Options needs a 16-bit controller event word")
    end

    local moved = false
    if (events & 0x0100) ~= 0 then
        options_selection = (options_selection + 1) & 1
        moved = true
    elseif (events & 0x0200) ~= 0 then
        options_selection = (options_selection + 1) & 1
        moved = true
    end

    local result = moved and 1 or 0
    if (events & 0x4000) ~= 0 then
        if options_selection == 0 then
            result = 3
        else
            checkout_selection = 1
            result = 2
        end
    elseif (events & 0x2000) ~= 0 then
        result = 4
    end
    return { result, options_selection, moved and 1 or 0 }
end

-- ROM 0xDAA8..0xDAEE consumes an Up/Down sample by moving the two-row
-- cursor and returning to the wait. A/B/C confirmation at 0xDAF0..0xDB10 is
-- reached only on a direction-free sample. Yes is row 0; No is row 1 and the
-- initial choice. Left, Right, and Start have no branch here.
-- Results: event code (0 waiting, 1 moved, 2 stay, 3 checked out), selected
-- row (0 Yes, 1 No), and whether the cursor moved.
function Hotel.advance_checkout(events)
    if type(events) ~= "number" or events % 1 ~= 0 or
       events < 0 or events > 0xFFFF then
        error("Hotel Check Out needs a 16-bit controller event word")
    end

    local moved = false
    if (events & 0x0100) ~= 0 then
        checkout_selection = (checkout_selection + 1) & 1
        moved = true
    elseif (events & 0x0200) ~= 0 then
        checkout_selection = (checkout_selection + 1) & 1
        moved = true
    end

    local result = moved and 1 or 0
    if not moved and
       ((events & 0x4000) ~= 0 or (events & 0x1000) ~= 0 or
        (events & 0x2000) ~= 0) then
        result = checkout_selection == 0 and 3 or 2
    end
    return { result, checkout_selection, moved and 1 or 0 }
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
-- Karma Screen loads with the display black: A at +8560, black 8562-8574,
-- partly drawn at 8576, complete at 8578 (emulator run3) [C].
function Hotel.presentation()
    return {
        status_text_delay_frames = "4",
        options_redraw_frames = "4",
        karma_load_black_frames = "15",
        -- Check Out -> field (the door event returns d6=-1; 0x2F32 puts
        -- the party back at the 0xBC36 return points, 0x131C fades out,
        -- 0x2BD2 reloads the scene). Frames after the confirm press:
        -- dimming from +4, black from +16, fade-in from +99, walking
        -- resumes at +132 (emulator run5: A at 8440) [C].
        checkout_fade_out_start_frame = "2",
        checkout_fade_in_start_frame = "99",
        checkout_input_resume_frame = "132",
        checkout_camera_settle_start_frame = "83",
    }
end

return Hotel
