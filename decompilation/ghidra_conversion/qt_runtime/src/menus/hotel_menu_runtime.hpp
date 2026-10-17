#pragma once

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
    karma_screen_opened,
    mode_action,
};

struct HotelMenuChoiceText {
    std::string id;
    std::string label;
};

// Host text and stable choice IDs loaded from the ROM-backed Lua text table
// and gameplay/scripts/hotel.lua. Layout and cursor presentation stay native.
struct HotelMenuContent {
    std::string title;
    std::string separator;
    std::array<HotelMenuChoiceText, 2> options{};
    std::array<std::string, 2> checkout_question{};
    std::array<HotelMenuChoiceText, 2> checkout_choices{};
    std::string medical_attention_suffix;
    std::array<std::string, 5> recovery_message{};
    // Lua Hotel.presentation(): frames after open before the status text
    // shows, and frames an Options redraw keeps the previous page.
    std::uint32_t status_text_delay_frames = 0U;
    std::uint32_t options_redraw_frames = 0U;
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
    using SidebarPainter = std::function<void(QPainter&)>;

    void set_presentation_timing(std::uint32_t status_text_delay_frames,
                                 std::uint32_t options_redraw_frames) noexcept;
    void paint(QPainter& painter, const QImage& background,
               const QImage& grid,
               const QImage& options_cursor,
               const QImage& checkout_cursor,
               const std::array<QImage, 7>& random_patterns,
               const HotelMenuContent& content,
               const TextPainter& draw_text,
               const SidebarPainter& paint_sidebar) const;
    HotelMenuPhase presented_phase() const noexcept {
        return presented_phase_;
    }

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
    void present_phase_after(std::uint32_t frames) noexcept;
    HotelMenuPhase presented_phase_ = HotelMenuPhase::closed;
    std::uint32_t open_frames_ = 0U;
    std::uint32_t page_hold_frames_ = 0U;
    std::uint32_t status_text_delay_frames_ = 0U;
    std::uint32_t options_redraw_frames_ = 0U;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
