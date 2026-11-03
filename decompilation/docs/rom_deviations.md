# Approved deviations from the ROM

The port matches the US ROM by default. Each entry here is a user-approved exception, recorded with its ROM address.

| ID | Screen | ROM behavior (address) | Port behavior | Approved |
|---|---|---|---|---|
| #36/#42 | Non-magic user's Magic menu (Magical Defense screen) | The `0xFBA6..0xFBEE` fallback waits at `0x0D94` without drawing posture; dismissal returns through `0x11928` to `0x11AE4`, which draws it | posture bar now always visible on the non-magic user Magic menu (fixes original bug) | User, 2026-11-03 |

See also `menu/route.lua` `E.MenuBPolicy.ALWAYS_CANCEL` (#30b). It is an opt-in setting; the ROM behavior stays the default.
