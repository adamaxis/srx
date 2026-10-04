#pragma once

#include "field_menu_text_catalog.hpp"
#include "shadowrun_ghidra/menus/hotel_entity_status_conversion.hpp"
#include "shadowrun_ghidra/menus/hotel_options_menu_conversion.hpp"
#include "shadowrun_ghidra/rendering/display_service_batch_conversion.hpp"
#include "shadowrun_ghidra/session/runner_record_access_conversion.hpp"

#include <QImage>

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>

class QPainter;

namespace shadowrun::ghidra_conversion::qt_runtime {

enum class HotelMenuPhase {
    closed,
    entity_status,
    options,
    checkout,
};

enum class HotelMenuPaletteLine : std::uint8_t {
    checkout = 2U,
    status_and_options = 3U,
};

enum class HotelMenuEvent {
    waiting,
    status_acknowledged,
    selection_moved,
    checkout_opened,
    stayed_at_hotel,
    checked_out,
    karma_screen_boundary,
    mode_action,
};

// Typed controller and presenter for the Hotel route entered after a paid
// room prompt. State transitions reuse the ROM-derived blocking menu
// conversions, while this Qt owner presents each full-screen page directly.
class HotelMenuRuntime final {
public:
    using TextPainter = std::function<void(QPainter&, int, int,
                                           std::string_view,
                                           HotelMenuPaletteLine)>;

    void open(std::array<RunnerRecordFields, 3>& records,
              const std::array<std::string, 3>& names,
              std::size_t active_runner_slot);
    void close() noexcept;
    void advance_animation(std::uint16_t& random_seed);
    HotelMenuEvent advance_controller_events(std::uint16_t events);
    void paint(QPainter& painter, const QImage& grid,
               const QImage& options_cursor,
               const QImage& checkout_cursor,
               const std::array<QImage, 7>& random_patterns,
               const FieldMenuTextCatalog& text,
               const TextPainter& draw_text) const;

    bool open() const noexcept { return phase_ != HotelMenuPhase::closed; }
    HotelMenuPhase phase() const noexcept { return phase_; }
    HotelOption option_selection() const noexcept {
        return options_state_.selection;
    }
    HotelCheckoutSelection checkout_selection() const noexcept {
        return checkout_state_.selection;
    }
    const std::array<bool, kHotelEntityStatusRecordCount>&
    medical_attention_rows() const noexcept {
        return medical_attention_rows_;
    }
    std::uint32_t animation_frames() const noexcept { return animation_frames_; }

private:
    void reset_page_effect() noexcept;
    void advance_status_acknowledgement();
    void paint_random_grid(QPainter& painter,
                           const std::array<QImage, 7>& patterns) const;

    HotelMenuPhase phase_ = HotelMenuPhase::closed;
    HotelOptionsMenuState options_state_{};
    HotelCheckoutPromptState checkout_state_{};
    std::array<bool, kHotelEntityStatusRecordCount>
        medical_attention_rows_{};
    std::array<std::string, kHotelEntityStatusRecordCount> entity_names_{};
    std::size_t medical_attention_count_ = 0U;
    RandomGridEffectState random_grid_effect_{};
    std::array<std::int8_t, 12> random_grid_cells_{};
    std::uint32_t animation_frames_ = 0U;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
