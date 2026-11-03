# Posture bar reference crops

These 64x8 RGB crops are independent screenshots from the headless GPGX
emulator running the US ROM. They are test references, not runtime assets.

- `gpgx_us_magic_posture_selected.png`: crop `(160,136)-(224,144)` from the
  selected Magic posture page after the Force/Posture loop takes Down. ROM
  path `0x10722 -> 0x107CE`, row table `C8600`.
- `gpgx_us_command_posture_selected.png`: crop `(176,168)-(240,176)` from the
  selected Command posture page at Neutral. ROM path `0x119CE ->
  0x11ABE/0x11AE4`, row table `CF334`.

The test allows 20 levels per RGB channel because GPGX's screenshot palette
conversion differs slightly from the port's direct CRAM-to-RGBA asset import.
The outline pixels still have to occupy the same positions and colors within
that measured palette tolerance.
