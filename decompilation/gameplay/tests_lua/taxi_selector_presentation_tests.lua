-- The cab-map selector caption uses the ROM's white-on-black presentation.
package.path = "decompilation/gameplay/scripts/?.lua;decompilation/gameplay/scripts/?/init.lua;" .. package.path

local E = require("enums")
local Taxi = require("data.taxi_destinations")
local Selector = require("menu.screens.taxi_selector")

local state = Selector.new({ current_region = Taxi.Region.REDMOND_BARRENS })
assert(E.TaxiCaptionPresentation.WHITE_ON_BLACK == "white_on_black",
       "taxi caption treatment enum must be stable for the host presenter")
assert(state.caption_presentation == E.TaxiCaptionPresentation.WHITE_ON_BLACK,
       "intro and gameplay cab-map captions require white text on black")

print("taxi_selector_presentation_tests: 2/2 passed")
