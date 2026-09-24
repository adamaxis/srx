#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct RawResourceTransferRequest {
    std::uint16_t destination = 0;
    std::size_t byte_count = 0;
};

struct CompressedResourceTransferRequest {
    std::size_t compressed_byte_count = 0;
};

struct ResourceRefreshCallbacks {
    std::function<void(const RawResourceTransferRequest&)> upload_raw_resource;
    std::function<void(const CompressedResourceTransferRequest&)>
        decode_and_upload_resource;
};

// ROM 0x176BA: refresh the shared field resource streams in source order.
// The host receives typed transfer requests; it does not inherit the source
// ROM pointers or VDP state machine.
void refresh_field_resource_streams(
    const ResourceRefreshCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
