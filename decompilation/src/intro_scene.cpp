#include "shadowrun/intro_scene.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

namespace shadowrun::native {

namespace {

struct StoryGroup {
    std::uint64_t start_frame;
    std::size_t first_line;
    std::size_t line_count;
    std::int32_t top_y;
};

// These are the source presentation beats recovered from the three opening
// routines. They are scene timing, not gameplay timing and do not belong in
// NativePortShell.
constexpr std::uint64_t kSeattleHold = 0x3CU;
constexpr std::uint64_t kSeattlePlace = kSeattleHold + 0x78U;
constexpr std::uint64_t kSeattleTeam = kSeattlePlace + 0xB4U;
constexpr std::uint64_t kSeattleEnd = kSeattleTeam + 0xF0U + 0x0FU;
constexpr std::uint64_t kWildernessPan = kSeattleEnd + 0xA1U;
constexpr std::uint64_t kWildernessMassacre =
    kWildernessPan + (0x1F0U - 0x164U);
constexpr std::uint64_t kWildernessEnd = kWildernessPan + 0x1F1U;
constexpr std::size_t kAmbushDeathStart = 0x1E0U;
constexpr std::uint64_t kTrividCockpitStart = kWildernessEnd;
constexpr std::uint64_t kTrividMapStart =
    kTrividCockpitStart + 1U + (0x11U * 2U);
constexpr std::uint64_t kTrividEnd = kTrividMapStart + 0x115U;
constexpr std::uint64_t kAirportText = kTrividEnd + (0x528U - 0x1E0U);
constexpr std::uint64_t kAircraftEnd = kTrividEnd + 0x529U;
constexpr std::uint64_t kDuration = kAircraftEnd;

constexpr std::int32_t kAircraftPlaneOriginX = 37 * 8;
constexpr std::int32_t kAircraftPlaneOriginY = 14 * 8;
constexpr std::array<std::int16_t, 7> kAircraftMovementDx{{
    0x00F1, 0x006E, 0x0071, 0x002A, 0x0000, 0x0063, 0x0000}};
constexpr std::array<std::int16_t, 7> kAircraftMovementDy{{
    0x0071, 0x0000, 0x0071, 0x0071, 0x0038, 0x0038, 0x0000}};
constexpr std::array<std::uint16_t, 7> kAircraftMovementDelay{{
    0x0024, 0x005A, 0x0048, 0x0012, 0x0024, 0x0012, 0x0000}};

constexpr std::array<StoryGroup, 9> kStoryGroups{{
    {0x000U, 0U, 0U, 16},
    {kSeattleHold, 0U, 1U, 16},
    {kSeattlePlace, 0U, 2U, 16},
    {kSeattleTeam, 0U, 5U, 16},
    {kSeattleEnd, 5U, 2U, 16},
    {kWildernessMassacre, 7U, 3U, 16},
    {kTrividMapStart, 10U, 3U, 16},
    {kTrividEnd, 13U, 3U, 16},
    {kAirportText, 16U, 6U, 16},
}};

const StoryGroup& story_group(std::uint64_t frame) {
    const auto group = std::find_if(
        kStoryGroups.rbegin(), kStoryGroups.rend(),
        [frame](const StoryGroup& candidate) {
            return frame >= candidate.start_frame;
        });
    return group == kStoryGroups.rend() ? kStoryGroups.front() : *group;
}

std::vector<Rgba8> scale_rgba(const std::vector<Rgba8>& source,
                              std::uint16_t remaining,
                              std::uint16_t total) {
    auto faded = source;
    if (total == 0U || remaining >= total) {
        return faded;
    }
    for (auto& pixel : faded) {
        pixel.red = static_cast<std::uint8_t>(
            (static_cast<std::uint16_t>(pixel.red) * remaining) / total);
        pixel.green = static_cast<std::uint8_t>(
            (static_cast<std::uint16_t>(pixel.green) * remaining) / total);
        pixel.blue = static_cast<std::uint8_t>(
            (static_cast<std::uint16_t>(pixel.blue) * remaining) / total);
    }
    return faded;
}

} // namespace

IntroScene::IntroScene(IntroSceneAssets assets) : assets_(std::move(assets)) {}

void IntroScene::reset() {
    line_start_ = 0;
    frame_ = 0;
    host_tick_ = 0;
}

IntroSceneResult IntroScene::update(const InputState& input) {
    if (input.was_pressed(Button::A) || input.was_pressed(Button::B) ||
        input.was_pressed(Button::Start)) {
        return IntroSceneResult::Skipped;
    }

    // The host presents at 60 Hz. The opening's source presentation loop is
    // advanced at 30 Hz, so two host ticks represent one source pass.
    if (host_tick_ == 0U) {
        host_tick_ = 1U;
    } else {
        host_tick_ = 0U;
        if (frame_ < kDuration) {
            ++frame_;
        }
    }
    line_start_ = story_group(frame_).first_line;
    return frame_ >= kDuration ? IntroSceneResult::Finished
                               : IntroSceneResult::Running;
}

void IntroScene::write_story_text(ScreenWriter& screen, std::int32_t x,
                                  std::int32_t y,
                                  std::string_view text) const {
    if (assets_.font.pixels.size() !=
            static_cast<std::size_t>(assets_.font.width) *
                assets_.font.height ||
        assets_.font.width < 8U || assets_.font.height < 8U) {
        screen.write_text(x, y, text, 1, {0x00, 0xE8, 0x00, 0xFF});
        return;
    }

    auto cursor = x;
    for (const auto character : text) {
        const auto code = static_cast<unsigned char>(character);
        if (code < 0x20U || code > 0x7FU) {
            cursor += 8;
            continue;
        }
        const auto glyph_index = static_cast<std::size_t>(code - 0x20U);
        const auto glyph_column = glyph_index % 16U;
        const auto glyph_row = glyph_index / 16U;
        if ((glyph_row + 1U) * 8U > assets_.font.height) {
            cursor += 8;
            continue;
        }

        std::vector<Rgba8> glyph(64U);
        auto glyph_has_ink = false;
        for (std::size_t row = 0; row < 8U; ++row) {
            for (std::size_t column = 0; column < 8U; ++column) {
                glyph[row * 8U + column] = assets_.font.pixels[
                    (glyph_row * 8U + row) * assets_.font.width +
                    glyph_column * 8U + column];
                glyph_has_ink = glyph_has_ink ||
                    glyph[row * 8U + column].alpha != 0U;
            }
        }
        if (glyph_has_ink) {
            screen.blit_rgba(cursor, y, 8, 8, glyph);
        } else if (character != ' ') {
            screen.write_text(cursor, y, std::string(1, character), 1,
                              {0x00, 0xE8, 0x00, 0xFF});
        }
        cursor += 8;
    }
}

void IntroScene::render(ScreenWriter& screen) const {
    screen.clear({0x00, 0x00, 0x00, 0xFF});
    const auto& group = story_group(frame_);
    constexpr std::uint16_t kFade = 16U;
    const auto fade_weight = [this](std::uint64_t appear,
                                    std::uint64_t disappear) {
        if (frame_ < appear || (disappear != 0U && frame_ >= disappear)) {
            return static_cast<std::uint16_t>(0U);
        }
        const auto into = frame_ - appear;
        if (into < kFade) {
            return static_cast<std::uint16_t>(into + 1U);
        }
        if (disappear != 0U && frame_ + kFade >= disappear) {
            return static_cast<std::uint16_t>(disappear - frame_);
        }
        return kFade;
    };
    const auto blit_faded_stage = [&screen](const NativeImage& stage,
                                             std::uint16_t weight) {
        if (stage.pixels.empty() || weight == 0U) {
            return;
        }
        screen.blit_rgba(0, 0, stage.width, stage.height,
                         scale_rgba(stage.pixels, weight, kFade));
    };

    if (frame_ < kSeattleEnd && !assets_.stages[0].pixels.empty()) {
        blit_faded_stage(assets_.stages[0], fade_weight(0, kSeattleEnd));
    } else if (frame_ < kWildernessEnd &&
               !assets_.stages[1].pixels.empty()) {
        const NativeImage* plate = &assets_.stages[1];
        if (frame_ < kWildernessPan &&
            !assets_.wilderness_pan_frames.empty() &&
            !assets_.wilderness_pan_frames.front().pixels.empty()) {
            const auto pan_frame = std::min<std::size_t>(
                ((frame_ - kSeattleEnd) * assets_.wilderness_pan_frames.size()) /
                    (kWildernessPan - kSeattleEnd),
                assets_.wilderness_pan_frames.size() - 1U);
            plate = &assets_.wilderness_pan_frames[pan_frame];
        } else if (frame_ >= kWildernessPan && !assets_.ambush_frames.empty()) {
            const auto ambush_frame = std::min<std::size_t>(
                frame_ - kWildernessPan, assets_.ambush_frames.size() - 1U);
            if (!assets_.ambush_frames[ambush_frame].pixels.empty()) {
                plate = &assets_.ambush_frames[ambush_frame];
            }
        }
        screen.blit_rgba(0, 0, plate->width, plate->height, plate->pixels);
        if (frame_ >= kWildernessPan + kAmbushDeathStart &&
            frame_ < kWildernessEnd && !assets_.trivid_slain.pixels.empty()) {
            screen.fill_rect(0, 112, 152, 112,
                             {0x00, 0x00, 0x00, 0xFF});
            screen.blit_rgba(assets_.trivid_slain.origin_x,
                             assets_.trivid_slain.origin_y,
                             assets_.trivid_slain.width,
                             assets_.trivid_slain.height,
                             assets_.trivid_slain.pixels);
        }
    } else if (frame_ < kTrividEnd &&
               (!assets_.trivid_background.pixels.empty() ||
                !assets_.stages[2].pixels.empty())) {
        if (frame_ < kTrividMapStart) {
            if (!assets_.trivid_slain.pixels.empty()) {
                screen.blit_rgba(
                    assets_.trivid_slain.origin_x,
                    assets_.trivid_slain.origin_y,
                    assets_.trivid_slain.width,
                    assets_.trivid_slain.height,
                    scale_rgba(assets_.trivid_slain.pixels, kFade, kFade));
            }
        } else if (!assets_.trivid_background.pixels.empty()) {
            const auto pass = std::min<std::uint64_t>(
                frame_ - kTrividMapStart, 0x114U);
            if (!assets_.trivid_moving.pixels.empty()) {
                const auto moving_x = static_cast<std::int32_t>(
                    (pass * 0x40U) >> 8U);
                const auto moving_y = static_cast<std::int32_t>(
                    (pass * 0x18U) >> 8U);
                screen.blit_rgba(
                    moving_x + assets_.trivid_moving.origin_x,
                    moving_y + assets_.trivid_moving.origin_y,
                    assets_.trivid_moving.width,
                    assets_.trivid_moving.height,
                    assets_.trivid_moving.pixels);
            }
            screen.blit_rgba(0, 0, assets_.trivid_background.width,
                             assets_.trivid_background.height,
                             assets_.trivid_background.pixels);
            if (!assets_.trivid_slain.pixels.empty()) {
                screen.blit_rgba(
                    assets_.trivid_slain.origin_x,
                    assets_.trivid_slain.origin_y,
                    assets_.trivid_slain.width,
                    assets_.trivid_slain.height,
                    assets_.trivid_slain.pixels);
            }
        } else {
            blit_faded_stage(assets_.stages[2], kFade);
        }
    } else if (!assets_.stages[3].pixels.empty()) {
        if (frame_ < kAircraftEnd && !assets_.aircraft_plane_a.pixels.empty()) {
            const auto elapsed = frame_ - kTrividEnd;
            const auto plane_a_scroll = static_cast<std::int32_t>(
                std::min<std::uint64_t>(elapsed * 2U, 0x200U));
            const auto plane_b_scroll = elapsed <= 0xD8U
                ? 0
                : static_cast<std::int32_t>(
                      std::min<std::uint64_t>(elapsed - 0xD8U, 0x78U));
            screen.clear({0x00, 0x00, 0x00, 0xFF});
            screen.blit_rgba(
                0, static_cast<std::int32_t>(0x78U - plane_b_scroll),
                assets_.stages[3].width, assets_.stages[3].height,
                assets_.stages[3].pixels);
            screen.blit_rgba(
                kAircraftPlaneOriginX - plane_a_scroll,
                kAircraftPlaneOriginY, assets_.aircraft_plane_a.width,
                assets_.aircraft_plane_a.height,
                assets_.aircraft_plane_a.pixels);

            if (elapsed <= 0xDCU) {
                const auto phase = static_cast<std::size_t>(
                    (elapsed / 6U) % assets_.heli_frames.size());
                const auto& overlay = assets_.heli_frames[phase];
                if (!overlay.pixels.empty()) {
                    screen.blit_rgba(
                        -plane_a_scroll + overlay.origin_x, overlay.origin_y,
                        overlay.width, overlay.height, overlay.pixels);
                }
            }

            if (elapsed >= 0x226U &&
                !assets_.aircraft_movement_frames[0].pixels.empty()) {
                std::uint32_t next = 0;
                std::size_t phase = 0;
                std::int32_t dx = 0;
                std::int32_t dy = 0;
                std::uint16_t x_fixed = 0x7C00;
                std::uint16_t y_fixed = 0xB700;
                for (std::uint64_t pass = 0; pass <= elapsed; ++pass) {
                    if (pass < 0x226U) {
                        continue;
                    }
                    if (phase < kAircraftMovementDx.size() && pass >= next) {
                        dx = kAircraftMovementDx[phase];
                        dy = kAircraftMovementDy[phase];
                        next = static_cast<std::uint32_t>(pass) +
                               kAircraftMovementDelay[phase];
                        ++phase;
                    }
                    x_fixed = static_cast<std::uint16_t>(
                        x_fixed + static_cast<std::uint16_t>(dx));
                    y_fixed = static_cast<std::uint16_t>(
                        y_fixed + static_cast<std::uint16_t>(dy));
                }
                const auto x = static_cast<std::int32_t>(x_fixed >> 8U);
                const auto y = static_cast<std::int32_t>(y_fixed >> 8U) + 0x28;
                const auto movement_index = y >= 0x110 && y < 0x11F ? 1U : 0U;
                const auto& movement = assets_.aircraft_movement_frames[
                    movement_index];
                screen.blit_rgba(
                    x - 0x90 + movement.origin_x,
                    y - 0x90 + movement.origin_y,
                    movement.width, movement.height, movement.pixels);
            }
        } else if (frame_ < kAircraftEnd && !assets_.heli_frames[0].pixels.empty()) {
            const auto elapsed = frame_ - kTrividEnd;
            const auto phase = std::min<std::size_t>(
                (elapsed * assets_.heli_frames.size()) /
                    (kAircraftEnd - kTrividEnd),
                assets_.heli_frames.size() - 1U);
            const auto& heli = assets_.heli_frames[phase];
            screen.clear({0x00, 0x00, 0x00, 0xFF});
            screen.blit_rgba(0, 0, heli.width, heli.height, heli.pixels);
        } else {
            blit_faded_stage(assets_.stages[3],
                             fade_weight(kTrividEnd, kDuration));
        }
    }

    const auto line_end = std::min(
        group.first_line + group.line_count, assets_.story_lines.size());
    for (auto line = group.first_line; line < line_end; ++line) {
        const auto& text = assets_.story_lines[line];
        const auto text_width = static_cast<std::int32_t>(text.size() * 8U);
        const auto x = std::max<std::int32_t>(
            0, (static_cast<std::int32_t>(kScreenWidth) - text_width) / 2);
        const auto y = group.top_y + static_cast<std::int32_t>(
            line - group.first_line) * 16;
        write_story_text(screen, x, y, text);
    }
}

} // namespace shadowrun::native
