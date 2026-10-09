#include "hotel_menu_runtime.hpp"

#include "shadowrun_ghidra/core/random_conversion.hpp"
#include "shadowrun_ghidra/field/field_hud_conversion.hpp"

#include <QPainter>
#include <QRect>

#include <algorithm>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

constexpr std::array<std::array<int, 2>, 12> kRandomGridCellOrigins{{
    {{40, 200}}, {{72, 200}}, {{184, 200}}, {{216, 200}},
    {{40, 208}}, {{72, 208}}, {{184, 208}}, {{216, 208}},
    {{56, 200}}, {{200, 200}}, {{56, 208}}, {{200, 208}},
}};

HotelOptionsInputFrame hotel_input_from_events(const std::uint16_t events) {
    return HotelOptionsInputFrame{
        (events & 0x0100U) != 0U,
        (events & 0x0200U) != 0U,
        (events & 0x4000U) != 0U,
        (events & 0x1000U) != 0U,
        (events & 0x2000U) != 0U};
}

} // namespace

void HotelMenuRuntime::open(
    std::array<RunnerRecordFields, 3>& records,
    const std::array<std::string, 3>& names,
    const std::size_t active_runner_slot) {
    options_state_ = HotelOptionsMenuState{};
    checkout_state_ = HotelCheckoutPromptState{};
    medical_attention_rows_.fill(false);
    entity_names_ = names;
    medical_attention_count_ = 0U;
    animation_frames_ = 0U;
    reset_page_effect();

    std::array<HotelEntityStatusRecord, kHotelEntityStatusRecordCount>
        status_records{};
    for (std::size_t slot = 0; slot < status_records.size(); ++slot) {
        status_records[slot].status_flags = static_cast<std::uint8_t>(
            records[slot].flags >> 8U);
        status_records[slot].delay_meter = records[slot].delay_meter;
        status_records[slot].display_name = names[slot];
    }

    const auto advance_behavior =
        [&records, active_runner_slot](
            const std::size_t slot, HotelEntityStatusRecord& status,
            const std::uint8_t amount) {
            if (slot >= records.size()) {
                return;
            }
            auto& runner = records[slot];
            FieldHudMentalMeterState meter{};
            meter.slot_word = static_cast<std::uint16_t>(slot);
            meter.flags_byte = status.status_flags;
            meter.meter = static_cast<std::int8_t>(runner.behavior_meter);
            meter.primary_runner = slot == active_runner_slot;
            const auto plan = advance_field_hud_mental_meter(
                meter, static_cast<std::int8_t>(amount));
            runner.behavior_meter = static_cast<std::uint8_t>(meter.meter);
            if (plan.forward_delay_meter) {
                FieldHudDelayMeterState delay{};
                delay.slot_word = static_cast<std::uint16_t>(slot);
                delay.flags_byte = status.status_flags;
                delay.meter = static_cast<std::int8_t>(runner.delay_meter);
                (void)advance_field_hud_delay_meter(delay,
                                                    plan.delay_delta);
                runner.delay_meter = static_cast<std::uint8_t>(delay.meter);
                status.delay_meter = runner.delay_meter;
            }
        };
    const auto advance_delay = [&records](
                                  const std::size_t slot,
                                  HotelEntityStatusRecord& status,
                                  const std::uint8_t amount) {
        if (slot >= records.size()) {
            return;
        }
        auto& runner = records[slot];
        FieldHudDelayMeterState delay{};
        delay.slot_word = static_cast<std::uint16_t>(slot);
        delay.flags_byte = status.status_flags;
        delay.meter = static_cast<std::int8_t>(runner.delay_meter);
        (void)advance_field_hud_delay_meter(
            delay, static_cast<std::int8_t>(amount));
        runner.delay_meter = static_cast<std::uint8_t>(delay.meter);
        status.delay_meter = runner.delay_meter;
    };

    service_hotel_entity_status(
        status_records,
        HotelEntityStatusCallbacks{
            {},
            advance_behavior,
            advance_delay,
            [this](const std::size_t slot, const std::string&) {
                if (slot < medical_attention_rows_.size()) {
                    medical_attention_rows_[slot] = true;
                    ++medical_attention_count_;
                }
            },
            {},
            [] {},
            {}});

    for (std::size_t slot = 0; slot < status_records.size(); ++slot) {
        records[slot].flags = static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(status_records[slot].status_flags)
             << 8U) |
            (records[slot].flags & 0x00FFU));
        records[slot].delay_meter = status_records[slot].delay_meter;
    }
    phase_ = HotelMenuPhase::entity_status;
}

void HotelMenuRuntime::close() noexcept {
    phase_ = HotelMenuPhase::closed;
    options_state_ = HotelOptionsMenuState{};
    checkout_state_ = HotelCheckoutPromptState{};
    medical_attention_rows_.fill(false);
    entity_names_ = {};
    medical_attention_count_ = 0U;
    animation_frames_ = 0U;
    reset_page_effect();
}

void HotelMenuRuntime::reset_page_effect() noexcept {
    random_grid_effect_ = RandomGridEffectState{};
    random_grid_cells_.fill(-1);
    animation_frames_ = 0U;
}

void HotelMenuRuntime::advance_animation(std::uint16_t& random_seed) {
    // ROM 0xEF08 waits through 0xF312, which calls E660 each VBlank. The
    // Hotel Options and checkout wait loops both call E660 (0xCF94 and
    // 0xDABA respectively). The Hotel Karma child pauses the parent loop.
    if (phase_ == HotelMenuPhase::closed) {
        return;
    }
    advance_random_grid_effect(
        random_grid_effect_,
        {[&random_seed](const std::uint16_t bound) {
             const auto value = sample_source_random_bound(random_seed, bound);
             return static_cast<std::uint16_t>(value < 0 ? 0 : value);
         },
         [this](const RandomGridEffectPlan& plan) {
             const bool first_branch =
                 plan.primary_source ==
                 RandomGridPrimarySource::branch_zero_table;
             const auto location = first_branch
                 ? static_cast<std::size_t>(plan.primary_entry_index)
                 : 8U + static_cast<std::size_t>(plan.primary_entry_index);
             const auto pattern = plan.variant_source ==
                     RandomGridVariantSource::fixed_three_variant_resources
                 ? static_cast<std::size_t>(plan.variant_entry_index)
                 : 3U + static_cast<std::size_t>(plan.variant_entry_index);
             if (location < random_grid_cells_.size() && pattern < 7U) {
                 random_grid_cells_[location] =
                     static_cast<std::int8_t>(pattern);
             }
         }});
    ++animation_frames_;
}

HotelMenuEvent HotelMenuRuntime::advance_controller_events(
    const std::uint16_t events) {
    if (!open()) {
        return HotelMenuEvent::waiting;
    }

    if (phase_ == HotelMenuPhase::entity_status) {
        if ((events & 0xFF00U) == 0U) {
            return HotelMenuEvent::waiting;
        }
        phase_ = HotelMenuPhase::options;
        options_state_ = HotelOptionsMenuState{};
        reset_page_effect();
        return HotelMenuEvent::status_acknowledged;
    }

    const auto input = hotel_input_from_events(events);
    if (phase_ == HotelMenuPhase::options) {
        const auto transition =
            apply_hotel_options_input(options_state_, input);
        if (transition.confirmed) {
            if (options_state_.selection == HotelOption::check_out) {
                checkout_state_ = HotelCheckoutPromptState{};
                phase_ = HotelMenuPhase::checkout;
                reset_page_effect();
                return HotelMenuEvent::checkout_opened;
            }
            return HotelMenuEvent::karma_screen_opened;
        }
        if (transition.action) {
            return HotelMenuEvent::mode_action;
        }
        return transition.selection_moved ? HotelMenuEvent::selection_moved
                                          : HotelMenuEvent::waiting;
    }

    const auto previous_selection = checkout_state_.selection;
    const auto result = apply_hotel_checkout_input(checkout_state_, input);
    if (!result.has_value()) {
        return checkout_state_.selection != previous_selection
                   ? HotelMenuEvent::selection_moved
                   : HotelMenuEvent::waiting;
    }
    if (*result == HotelCheckoutResult::stay_at_hotel) {
        phase_ = HotelMenuPhase::options;
        options_state_ = HotelOptionsMenuState{};
        reset_page_effect();
        return HotelMenuEvent::stayed_at_hotel;
    }
    close();
    return HotelMenuEvent::checked_out;
}

void HotelMenuRuntime::paint_random_grid(
    QPainter& painter,
    const std::array<QImage, 7>& random_patterns) const {
    for (std::size_t location = 0; location < random_grid_cells_.size();
         ++location) {
        const auto pattern = random_grid_cells_[location];
        if (pattern < 0 || static_cast<std::size_t>(pattern) >=
                               random_patterns.size()) {
            continue;
        }
        painter.drawImage(kRandomGridCellOrigins[location][0],
                          kRandomGridCellOrigins[location][1],
                          random_patterns[static_cast<std::size_t>(pattern)]);
    }
}

void HotelMenuRuntime::paint(
    QPainter& painter, const QImage& grid,
    const QImage& options_cursor, const QImage& checkout_cursor,
    const std::array<QImage, 7>& random_patterns,
    const HotelMenuContent& content, const TextPainter& draw_text) const {
    // Every Hotel page starts from a complete black composition and the
    // source 28x21 grid. No dialogue panel or previous page controls survive.
    painter.fillRect(QRect(0, 0, 320, 224), Qt::black);
    if (!open()) {
        return;
    }
    if (!grid.isNull()) {
        painter.drawImage(24, 24, grid);
    }
    paint_random_grid(painter, random_patterns);
    if (!draw_text) {
        return;
    }

    if (phase_ == HotelMenuPhase::entity_status) {
        std::size_t attention_row = 0U;
        for (std::size_t slot = 0; slot < medical_attention_rows_.size();
             ++slot) {
            if (!medical_attention_rows_[slot]) {
                continue;
            }
            // ROM 0xEF5E writes the runner name at A2+0x0E; the leading
            // 0x80 in C2643 advances A2 one row before the suffix. Each
            // serviced runner then advances A2 by 0x180 (three text rows).
            const auto y = 40 + static_cast<int>(attention_row) * 24;
            draw_text(painter, 88, y, entity_names_[slot],
                      HotelMenuPaletteLine::status_and_options);
            draw_text(painter, 32, y + 8,
                      content.medical_attention_suffix,
                      HotelMenuPaletteLine::status_and_options);
            ++attention_row;
        }
        // C265E begins with two 0x80 row advances after EF7C's per-runner
        // cursor increment. The visible recovery rows therefore begin at
        // y=40 + 24*serviced-runners + 16.
        const auto recovery_y =
            40 + 24 * static_cast<int>(medical_attention_count_) + 16;
        for (std::size_t line = 2U;
             line < content.recovery_message.size(); ++line) {
            draw_text(painter, 32,
                      recovery_y + static_cast<int>(line - 2U) * 8,
                      content.recovery_message[line],
                      HotelMenuPaletteLine::status_and_options);
        }
        return;
    }

    if (phase_ == HotelMenuPhase::options) {
        draw_text(painter, 72, 48, content.title,
                  HotelMenuPaletteLine::status_and_options);
        draw_text(painter, 72, 56, content.separator,
                  HotelMenuPaletteLine::status_and_options);
        for (std::size_t row = 0; row < content.options.size(); ++row) {
            draw_text(painter, 72, 72 + static_cast<int>(row) * 8,
                      content.options[row].label,
                      HotelMenuPaletteLine::status_and_options);
        }
        const auto row = static_cast<int>(options_state_.selection);
        if (!options_cursor.isNull()) {
            painter.drawImage(72, 72 + row * 8, options_cursor);
        }
        return;
    }

    for (std::size_t row = 0; row < content.checkout_question.size();
         ++row) {
        draw_text(painter, 80, 72 + static_cast<int>(row) * 8,
                  content.checkout_question[row],
                  HotelMenuPaletteLine::checkout);
    }
    draw_text(painter, 128, 96, content.checkout_choices[0].label,
              HotelMenuPaletteLine::checkout);
    draw_text(painter, 128, 112, content.checkout_choices[1].label,
              HotelMenuPaletteLine::checkout);
    if (!checkout_cursor.isNull()) {
        painter.drawImage(
            // C3B2 doubles FFF0DC for this cursor table because FFF0FC is
            // clear. The No row is two text rows below Yes (E387E).
            104, 96 + static_cast<int>(checkout_state_.selection) * 16,
            checkout_cursor);
    }
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
