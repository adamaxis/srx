#include "shadowrun_ghidra/rendering/resource_refresh_conversion.hpp"

#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<std::uint32_t> order;
    RawResourceTransferRequest raw{};
    CompressedResourceTransferRequest compressed{};
    refresh_field_resource_streams(
        {[&order, &raw](const RawResourceTransferRequest& request) {
             order.push_back(1U);
             raw = request;
         },
         [&order, &compressed](
             const CompressedResourceTransferRequest& request) {
             order.push_back(2U);
             compressed = request;
         }});
    if (order != std::vector<std::uint32_t>{1U, 2U} ||
        raw.destination != 0x2360U || raw.byte_count != 0xA80U ||
        compressed.compressed_byte_count != 0x2DE0U) {
        return 1;
    }

    order.clear();
    refresh_field_resource_streams({{}, [&order](const auto&) {
                                        order.push_back(2U);
                                    }});
    if (order != std::vector<std::uint32_t>{2U}) {
        return 2;
    }
    return 0;
}
