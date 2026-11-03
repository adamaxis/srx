# Magical Defense reference captures

`ui_ref_magic_defense_decker_ammo04_source.png` is the user supplied real-game
screen `1_magic_defense_screen.png`, including its 1.0x emulator overlay.
`gpgx_us_magic_defense_decker_ammo04.png` is a headless GPGX capture from the
US ROM in the same Decker / Magic fallback state, with the source current-ammo
counter at 04. The screen was reached through the ROM menu inputs; the live
ammo byte is `FF014E=04` and its already-rendered counter uses the ROM table at
`0x6AC46`.

`ui_ref_magic_defense_decker_ammo04.png` is the supplied screenshot registered
to 320x224 using its visible game-area bounds (x origin 129.86, x scale 3.0594,
y origin -4.356, y scale 3.34225). The capture is filtered and clipped at the
top edge. For the 448 ambiguous samples outside the posture row, the registered
image uses the corresponding pixels from the independent same-state GPGX
capture to recover exact Genesis CRAM indices. All tests compare every pixel
outside the posture row against both 320x224 captures after mapping RGB values
to their Genesis 3-bit channel index.

The port's posture bar is an approved deviation. Only the screen rectangle
`x=96..263, y=160..183` is omitted from the two external pixel comparisons; a
separate assertion checks that the posture bar itself is visible.
