#include "shadowrun_ghidra/title_intro/title_reveal_transfer_conversion.hpp"

#include <cstdint>
#include <optional>
#include <vector>

using namespace shadowrun::ghidra_conversion;

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    TitleRevealTransferState state{};
    state.countdown = 4;
    state.profile_selector = 1U;
    state.destination_base = 0x0070U;
    state.random_bound = 0x0100U;
    state.source_buffer_token = 0xDA038U;

    std::vector<TitleRevealWordWrite> writes;
    std::uint16_t random_value = 0x0037U;
    std::uint32_t transferred = 0;
    std::uint32_t mask_samples = 0;
    WorldSourceBlockTransferRequest last_transfer{};
    TitleRevealTransferCallbacks callbacks{};
    callbacks.transfer_source_block =
        [&](const WorldSourceBlockTransferRequest& request) {
            ++transferred;
            last_transfer = request;
        };
    callbacks.sample_random = [&](const std::uint16_t bound) {
        return static_cast<std::uint16_t>(random_value % (bound == 0U ? 1U : bound));
    };
    const auto& mask_banks = source_random_nibble_mask_banks();
    callbacks.expand_mask = [&](const std::uint16_t selector) {
        return expand_random_nibble_mask(
            mask_banks, selector, [&](const std::uint16_t bound) {
                ++mask_samples;
                return static_cast<std::uint16_t>((mask_samples - 1U) % bound);
            });
    };
    callbacks.read_source_longword =
        [](const std::uint32_t token, const std::uint16_t offset)
        -> std::optional<std::uint32_t> {
        return token ^ static_cast<std::uint32_t>(offset);
    };
    callbacks.write_reveal_word =
        [&](const TitleRevealWordWrite& write) { writes.push_back(write); };

    if (!expect(advance_title_reveal_transfer_service(state, callbacks) ==
                    TitleRevealTransferAction::rendered &&
                state.countdown == 3 && transferred == 0U &&
                mask_samples == 0x47U &&
                writes.size() == 0x47U)) {
        return 1;
    }
    if (!expect(writes.front().destination_address ==
                    static_cast<std::uint16_t>(0x70U + random_value) &&
                writes.front().source_byte_offset ==
                    static_cast<std::uint16_t>(random_value & 0xFFFCU) &&
                writes.front().mask == 0x000FFFFFU &&
                writes.front().value ==
                    ((0xDA038U ^ (random_value & 0xFFFCU)) &
                     0x000FFFFFU) &&
                writes[1].mask == 0x00F0FFFFU &&
                writes[1].mask != writes.front().mask)) {
        return 2;
    }

    state.countdown = 0;
    state.behavior_flags = 0xFFU;
    state.world_sequence_requested = true;
    if (!expect(advance_title_reveal_transfer_service(state, callbacks) ==
                    TitleRevealTransferAction::started_world_sequence &&
                state.behavior_flags == 0xEFU &&
                state.world_sequence.active &&
                state.world_sequence.sequence_table_token == 0x1D428U &&
                state.world_sequence.source_buffer_token == 0xDA038U &&
                last_transfer.source_address == 0xDA038U &&
                last_transfer.destination_address == 0x0070U &&
                last_transfer.byte_count == 0x0100U)) {
        return 3;
    }

    state.countdown = 0;
    state.world_sequence_requested = false;
    std::uint32_t menu_handoff = 0;
    callbacks.publish_menu_selection_then_text = [&] { ++menu_handoff; };
    if (!expect(advance_title_reveal_transfer_service(state, callbacks) ==
                    TitleRevealTransferAction::handed_to_menu_text &&
                menu_handoff == 1U && transferred == 2U)) {
        return 4;
    }

    state.countdown = 2;
    callbacks.read_source_longword =
        [](const std::uint32_t, const std::uint16_t) {
        return std::optional<std::uint32_t>{};
    };
    if (!expect(advance_title_reveal_transfer_service(state, callbacks) ==
                TitleRevealTransferAction::source_read_unresolved)) {
        return 5;
    }
    return 0;
}
