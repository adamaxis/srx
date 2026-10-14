#pragma once

namespace shadowrun::ghidra_conversion {

// ROM 0x1F32E: consume the optional mode-resource pending marker.
void clear_optional_mode_resource(bool& pending);

} // namespace shadowrun::ghidra_conversion
