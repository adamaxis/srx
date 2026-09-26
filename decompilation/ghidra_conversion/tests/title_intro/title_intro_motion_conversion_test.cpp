#include "shadowrun_ghidra/title_intro/title_intro_motion_conversion.hpp"

#include <array>
#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    TitleIntroMotionState motion{0U, 0U, 0U, 0U, 0U, 0U, 0U};
    reset_title_intro_motion(motion);
    if (motion.next_event_frame != 0U || motion.phase != 0U) {
        return 1;
    }
    const std::array<TitleIntroMotionPhase, kTitleIntroMotionPhaseCount>
        phases{{{0x3000U, 2U, 0x0100U, 3U},
                {0x3001U, 2U, 0U, 4U},
                {0x3002U, 2U, 0U, 5U},
                {0x3003U, 2U, 0U, 6U},
                {0x3004U, 2U, 0U, 7U}}};
    std::vector<TitleIntroPositionedSpriteRequest> motion_requests;
    const TitleIntroMotionCallbacks motion_callbacks{
        [&motion_requests](const TitleIntroPositionedSpriteRequest& request) {
            motion_requests.push_back(request);
        }};

    motion.current_frame = 0U;
    if (advance_title_intro_motion(motion, phases, 0x0200U,
                                   motion_callbacks) !=
            TitleIntroMotionAdvanceResult::sprite_rendered ||
        motion.sprite_descriptor != 0x3000U || motion.position != 0x0103U ||
        motion_requests.front().x != 0x0105U || motion_requests.front().y !=
            0x0133U) {
        return 2;
    }
    motion.current_frame = 0x0168U;
    motion.next_event_frame = 0x0168U;
    motion.phase = 4U;
    if (advance_title_intro_motion(motion, phases, 0U, motion_callbacks) !=
            TitleIntroMotionAdvanceResult::sequence_finished ||
        motion.phase != 5U) {
        return 3;
    }

    TitleIntroDualSpriteState dual{0U, 0U, 0U, 0U, 0U};
    reset_title_intro_dual_sprites(dual);
    const std::array<std::uint32_t, kTitleIntroDualSpriteCount> sprites{{
        0x4000U, 0x4001U, 0x4002U, 0x4003U,
        0x4010U, 0x4011U, 0x4012U, 0x4013U}};
    std::vector<TitleIntroPositionedSpriteRequest> left;
    std::vector<TitleIntroPositionedSpriteRequest> right;
    const TitleIntroDualSpriteCallbacks dual_callbacks{
        [&left](const TitleIntroPositionedSpriteRequest& request) {
            left.push_back(request);
        },
        [&right](const TitleIntroPositionedSpriteRequest& request) {
            right.push_back(request);
        }};
    advance_title_intro_dual_sprites(dual, sprites, dual_callbacks);
    if (dual.left_sprite_descriptor != 0x4000U ||
        dual.right_sprite_descriptor != 0x4010U || dual.phase != 1U ||
        dual.next_event_frame != 6U || left.size() != 1U || right.size() != 1U ||
        left.front().x != 0x0183U || right.front().x != 0x01ABU) {
        return 4;
    }
    dual.current_frame = 6U;
    advance_title_intro_dual_sprites(dual, sprites, dual_callbacks);
    if (dual.left_sprite_descriptor != 0x4001U ||
        dual.right_sprite_descriptor != 0x4011U || left.size() != 2U ||
        right.size() != 2U) {
        return 5;
    }
    return 0;
}
