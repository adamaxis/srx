#include "shadowrun_ghidra/rendering/resource_refresh_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void refresh_field_resource_streams(
    const ResourceRefreshCallbacks& callbacks) {
    if (callbacks.upload_raw_resource) {
        callbacks.upload_raw_resource({0x2360U, 0xA80U});
    }
    if (callbacks.decode_and_upload_resource) {
        callbacks.decode_and_upload_resource({0x2DE0U});
    }
}

} // namespace shadowrun::ghidra_conversion
