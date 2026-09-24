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

    const auto has_sprite_group = [this](std::string_view role,
                                         std::size_t table_index) {
        return std::any_of(
            assets_.sprites.begin(), assets_.sprites.end(),
            [role, table_index](const NativeIntroSprite& sprite) {
                return sprite.role == role &&
                       sprite.table_index == table_index;
            });
    };
    const auto draw_sprite_group = [&screen](
                                        const std::vector<NativeIntroSprite>&
                                            sprites,
                                        std::string_view role,
                                        std::size_t table_index,
                                        std::int32_t x, std::int32_t y) {
        for (const auto& sprite : sprites) {
            if (sprite.role != role || sprite.table_index != table_index ||
                sprite.image.pixels.empty()) {
                continue;
            }
            screen.blit_rgba(
                x + sprite.draw_offset_x, y + sprite.draw_offset_y,
                sprite.image.width, sprite.image.height, sprite.image.pixels);
        }
    };
    const auto draw_patch = [&screen](const NativeImage& patch) {
        if (patch.pixels.empty()) {
            return;
        }
        screen.blit_rgba(patch.origin_x, patch.origin_y, patch.width,
                         patch.height, patch.pixels);
    };

    const auto render_ambush = [&](std::size_t relative_frame) {
        constexpr std::array<std::uint32_t, 4> kMapDelays{{12, 24, 60, 12}};
        constexpr std::array<std::uint32_t, 4> kActorDelays{{60, 36, 60, 24}};
        constexpr std::array<std::uint32_t, 5> kMuzzleDelays{{60, 6, 12, 18,
                                                                60}};
        constexpr std::array<std::int32_t, 5> kMuzzleX{{1, 0x11A, 0, 0, 1}};
        constexpr std::array<std::int32_t, 5> kMuzzleVelocity{{0, 4, 6, 8,
                                                                 0}};
        constexpr std::array<std::int32_t, 7> kProjectileBY{{0xBB, 0xA3,
                                                              0x113, 0xE3,
                                                              0xF8, 0x105,
                                                              0xBC}};
        constexpr std::array<std::int32_t, 7> kProjectileBThreshold{{0x40,
                                                                       0x40,
                                                                       0x145,
                                                                       0x183,
                                                                       0x40,
                                                                       0x104,
                                                                       0x40}};
        constexpr std::array<std::uint32_t, 7> kProjectileBDelays{{0x2A, 0x30,
                                                                     0x4E, 0x30,
                                                                     0x6A, 0x2A,
                                                                     0x3C}};
        constexpr std::array<std::int32_t, 8> kProjectileCY{{0xAC, 0xE6, 0xBB,
                                                              0x116, 0xB1, 0x2A,
                                                              0xAC, 0x16}};
        constexpr std::array<std::uint32_t, 8> kProjectileCDelays{{0x30, 0x3C,
                                                                    0x30, 0x3C,
                                                                    0x30, 0x3C,
                                                                    0x30, 0x3C}};
        constexpr std::array<std::int32_t, 8> kProjectileDY{{0x20, 0xD7, 0x94,
                                                              0xA6, 0x0C, 0xE2,
                                                              0xC4, 0xF8}};
        constexpr std::array<std::uint32_t, 8> kProjectileDDelays{{0x36, 0x48,
                                                                    0x36, 0x48,
                                                                    0x36, 0x48,
                                                                    0x36, 0x48}};
        constexpr std::array<std::uint32_t, 3> kMichaelDelays{{3, 6, 9}};

        std::size_t map_phase = 0;
        std::array<std::size_t, 4> loaded_patches{};
        std::size_t loaded_patch_count = 0;
        std::uint32_t map_next = 0;
        std::size_t actor_phase = 0;
        std::uint32_t actor_next = 0;
        bool actor_loaded = false;
        std::size_t muzzle_phase = 0;
        std::size_t muzzle_index = 0;
        std::int32_t muzzle_x = 0;
        std::int32_t muzzle_velocity = 0;
        std::uint32_t muzzle_next = 0;
        std::size_t flash_phase = 0;
        std::size_t flash_first = 0;
        std::size_t flash_second = 1;
        std::uint32_t flash_next = 0;
        bool flash_suppressed = false;
        std::size_t body_phase = 6;
        std::size_t body_index = 0;
        std::int32_t body_x = 0;
        std::int32_t body_y = 0;
        std::uint32_t body_next = 0;
        bool body_loaded = false;
        bool draw_body_this_pass = false;
        std::size_t projectile_b_phase = 0;
        std::size_t projectile_b_index = 0;
        std::int32_t projectile_b_x = 0;
        std::int32_t projectile_b_y = 0;
        std::int32_t projectile_b_threshold = 0;
        std::uint32_t projectile_b_next = 0;
        std::size_t projectile_c_phase = 0;
        std::size_t projectile_c_index = 0;
        std::int32_t projectile_c_x = 0;
        std::int32_t projectile_c_y = 0;
        std::uint32_t projectile_c_next = 0;
        std::size_t projectile_d_phase = 0;
        std::size_t projectile_d_index = 0;
        std::int32_t projectile_d_x = 0;
        std::int32_t projectile_d_y = 0;
        std::uint32_t projectile_d_next = 0;
        std::size_t michael_phase = 0;
        std::size_t michael_index = 0;
        std::int32_t michael_x = 0x193;
        std::uint32_t michael_next = 0;
        bool michael_active = false;
        bool central_body_removed = false;
        bool indian_body_removed = false;
        std::int32_t scroll_fixed = 0;

        const auto start_body = [&](std::int32_t x, std::int32_t y) {
            body_phase = 0;
            body_next = 0;
            body_x = x;
            body_y = y;
            body_loaded = false;
            if (x == 0x13D) {
                central_body_removed = true;
            } else if (x == 0x182) {
                indian_body_removed = true;
            }
        };

        for (std::size_t now = 0; now <= relative_frame; ++now) {
            if (now >= 0x120U && now <= 0x168U) {
                scroll_fixed += 0xAA;
            }
            const auto scroll_x = scroll_fixed >> 8;

            if (map_phase < 0x0CU && now >= map_next) {
                if (map_phase < 4U) {
                    loaded_patches[loaded_patch_count++] = map_phase;
                    map_next = now + kMapDelays[map_phase];
                    ++map_phase;
                } else if (map_phase == 4U) {
                    ++map_phase;
                    map_next = now + 9U;
                } else {
                    map_next = now + 9U;
                    ++map_phase;
                    if (map_phase == 9U) {
                        start_body(0x13D, 0x139);
                    }
                }
            }

            if (actor_phase < 0x0CU && now >= actor_next) {
                if (actor_phase < 4U) {
                    actor_next = now + kActorDelays[actor_phase];
                    ++actor_phase;
                    actor_loaded = true;
                } else {
                    if (actor_phase == 4U) {
                        flash_suppressed = true;
                    }
                    actor_next = now + 9U;
                    ++actor_phase;
                    if (actor_phase == 9U) {
                        start_body(0x182, 0x0E2);
                    }
                }
            }

            if (muzzle_phase < 5U && now >= muzzle_next) {
                muzzle_index = muzzle_phase;
                muzzle_x = kMuzzleX[muzzle_phase];
                muzzle_velocity = kMuzzleVelocity[muzzle_phase];
                muzzle_next = now + kMuzzleDelays[muzzle_phase];
                ++muzzle_phase;
                if (muzzle_phase >= 5U) {
                    muzzle_phase = now < 0x168U ? 1U : 5U;
                }
            }

            if (!flash_suppressed && now >= flash_next) {
                flash_first = flash_phase;
                flash_second = (flash_phase + 1U) % 4U;
                flash_next = now + 6U;
                flash_phase = (flash_phase + 1U) % 4U;
            }

            draw_body_this_pass = false;
            if (body_phase < 6U && now >= body_next) {
                body_index = body_phase;
                body_next = now + 8U;
                ++body_phase;
                body_loaded = true;
                draw_body_this_pass = true;
            }
            if (body_loaded && body_phase < 6U) {
                draw_body_this_pass = true;
            }

            if (projectile_b_phase < 7U && now >= projectile_b_next) {
                projectile_b_index = projectile_b_phase;
                projectile_b_y = kProjectileBY[projectile_b_phase];
                projectile_b_threshold =
                    kProjectileBThreshold[projectile_b_phase];
                projectile_b_next = now + kProjectileBDelays[projectile_b_phase];
                projectile_b_x = 0x200;
                ++projectile_b_phase;
            }
            if (projectile_c_phase < 8U && now >= projectile_c_next) {
                projectile_c_index = projectile_c_phase;
                projectile_c_y = kProjectileCY[projectile_c_phase];
                projectile_c_next = now + kProjectileCDelays[projectile_c_phase];
                projectile_c_x = 0x200;
                ++projectile_c_phase;
            }
            if (projectile_d_phase < 8U && now >= projectile_d_next) {
                projectile_d_index = projectile_d_phase;
                projectile_d_y = kProjectileDY[projectile_d_phase];
                projectile_d_next = now + kProjectileDDelays[projectile_d_phase];
                projectile_d_x = 0x200;
                ++projectile_d_phase;
            }

            if (now < michael_next) {
                if (michael_active) {
                    michael_x += 4;
                }
            } else if (now >= 6U && michael_phase < 3U) {
                michael_index = michael_phase;
                michael_next = now + kMichaelDelays[michael_phase];
                ++michael_phase;
                michael_active = true;
                michael_x += 4;
            } else {
                michael_active = false;
            }

            if (now != relative_frame) {
                if (projectile_b_phase != 0U && projectile_b_x >=
                    projectile_b_threshold) {
                    projectile_b_x -= 9;
                }
                if (projectile_c_phase != 0U && projectile_c_x >= 0x40) {
                    projectile_c_x -= 8;
                }
                if (projectile_d_phase != 0U && projectile_d_x >= 0x40) {
                    projectile_d_x -= 0x0B;
                }
                continue;
            }

            if (!assets_.stages[1].pixels.empty()) {
                screen.blit_rgba(0, 0, assets_.stages[1].width,
                                 assets_.stages[1].height,
                                 assets_.stages[1].pixels);
            }
            for (std::size_t index = 0; index < loaded_patch_count; ++index) {
                draw_patch(assets_.background_patches[loaded_patches[index]]);
            }
            if (central_body_removed) {
                screen.fill_rect(112, 128, 160, 96,
                                 {0x00, 0x00, 0x00, 0xFF});
            }
            if (indian_body_removed) {
                screen.fill_rect(224, 56, 80, 72,
                                 {0x00, 0x00, 0x00, 0xFF});
            }
            if (actor_loaded && actor_phase < 4U) {
                draw_sprite_group(assets_.sprites, "ambush-actor",
                                  actor_phase - 1U, 0x178, 0x0D1);
            }
            if (!flash_suppressed && muzzle_phase < 5U) {
                muzzle_x += muzzle_velocity;
                draw_sprite_group(assets_.sprites, "ambush-muzzle", muzzle_index,
                                  muzzle_x + scroll_x, 0x133);
            }
            if (!flash_suppressed) {
                draw_sprite_group(assets_.sprites, "ambush-flash", flash_first,
                                  0x183, 0x0BE);
                draw_sprite_group(assets_.sprites, "ambush-flash", flash_second,
                                  0x1AB, 0x0D7);
            }
            if (draw_body_this_pass) {
                draw_sprite_group(assets_.sprites, "ambush-body", body_index,
                                  body_x, body_y);
            }
            if (projectile_b_phase != 0U && projectile_b_x >=
                projectile_b_threshold) {
                projectile_b_x -= 9;
                draw_sprite_group(assets_.sprites, "ambush-projectile-b",
                                  projectile_b_index, projectile_b_x,
                                  projectile_b_y + 0x18);
            }
            if (projectile_c_phase != 0U && projectile_c_x >= 0x40) {
                projectile_c_x -= 8;
                draw_sprite_group(assets_.sprites, "ambush-projectile-c",
                                  projectile_c_index, projectile_c_x,
                                  projectile_c_y + 0x18);
            }
            if (projectile_d_phase != 0U && projectile_d_x >= 0x40) {
                projectile_d_x -= 0x0B;
                draw_sprite_group(assets_.sprites, "ambush-projectile-d",
                                  projectile_d_index, projectile_d_x,
                                  projectile_d_y + 0x18);
            }
            if (michael_active) {
                draw_sprite_group(assets_.sprites, "ambush-michael",
                                  michael_index, michael_x, 0x13C);
            }
        }

        if (relative_frame >= kAmbushDeathStart &&
            has_sprite_group("trivid-left", 0U)) {
            screen.fill_rect(0, 112, 152, 112,
                             {0x00, 0x00, 0x00, 0xFF});
            draw_sprite_group(assets_.sprites, "trivid-left", 0U, 0xC2, 0x13C);
        }
    };

    if (frame_ < kSeattleEnd && !assets_.stages[0].pixels.empty()) {
        blit_faded_stage(assets_.stages[0], fade_weight(0, kSeattleEnd));
    } else if (frame_ < kWildernessEnd &&
               !assets_.stages[1].pixels.empty()) {
        if (frame_ < kWildernessPan) {
            const auto pan_frame = static_cast<std::size_t>(
                frame_ - kSeattleEnd);
            const auto plane_a_xscroll =
                static_cast<std::int32_t>(-0x40 + pan_frame);
            const auto plane_b_vscroll = static_cast<std::int32_t>(
                -0x20 + std::min<std::size_t>(pan_frame / 2U, 0x20U));
            screen.blit_rgba(plane_a_xscroll, -plane_b_vscroll,
                             assets_.stages[1].width,
                             assets_.stages[1].height,
                             assets_.stages[1].pixels);
            draw_sprite_group(
                assets_.sprites, "ambush-actor", 0,
                0x138 + static_cast<std::int32_t>(
                            std::min<std::size_t>(pan_frame, 0x40U)),
                0x0F1 - static_cast<std::int32_t>(
                            std::min<std::size_t>(pan_frame / 2U, 0x20U)));
        } else {
            render_ambush(static_cast<std::size_t>(frame_ - kWildernessPan));
        }
    } else if (frame_ < kTrividEnd &&
               (!assets_.trivid_background.pixels.empty() ||
                !assets_.stages[2].pixels.empty())) {
        if (frame_ < kTrividMapStart) {
            draw_sprite_group(assets_.sprites, "trivid-left", 0U, 0xC2, 0x13C);
        } else if (!assets_.trivid_background.pixels.empty()) {
            const auto pass = std::min<std::uint64_t>(
                frame_ - kTrividMapStart, 0x114U);
            const auto moving_x = static_cast<std::int32_t>(
                (pass * 0x40U) >> 8U);
            const auto moving_y = static_cast<std::int32_t>(
                (pass * 0x18U) >> 8U);
            draw_sprite_group(assets_.sprites, "trivid-right", 0U,
                              0x60 + moving_x, 0x123 + moving_y);
            screen.blit_rgba(0, 0, assets_.trivid_background.width,
                             assets_.trivid_background.height,
                             assets_.trivid_background.pixels);
            draw_sprite_group(assets_.sprites, "trivid-left", 0U, 0xC2, 0x13C);
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

            if (elapsed <= 0xDCU &&
                has_sprite_group("helicopter", 0U)) {
                const auto phase = static_cast<std::size_t>(
                    (elapsed / 6U) % kNativeIntroHeliFrameCount);
                draw_sprite_group(assets_.sprites, "helicopter", phase,
                                  0x1FC - plane_a_scroll, 0xEF);
            }

            if (elapsed >= 0x226U &&
                has_sprite_group("aircraft-movement", 0U)) {
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
                draw_sprite_group(assets_.sprites, "aircraft-movement",
                                  movement_index, x, y);
            }
        } else if (frame_ < kAircraftEnd &&
                   has_sprite_group("helicopter", 0U)) {
            const auto elapsed = frame_ - kTrividEnd;
            const auto phase = std::min<std::size_t>(
                (elapsed * kNativeIntroHeliFrameCount) /
                    (kAircraftEnd - kTrividEnd),
                kNativeIntroHeliFrameCount - 1U);
            screen.clear({0x00, 0x00, 0x00, 0xFF});
            draw_sprite_group(assets_.sprites, "helicopter", phase, 0x1FC,
                              0xEF);
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
