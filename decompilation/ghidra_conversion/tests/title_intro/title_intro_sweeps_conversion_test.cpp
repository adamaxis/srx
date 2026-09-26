#include "shadowrun_ghidra/title_intro/title_intro_sweeps_conversion.hpp"

#include <array>
#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    const std::array<TitleIntroTargetedSweepPhase,
                     kTitleIntroTargetedSweepPhaseCount>
        targeted_phases{{{0x6000U, 0x0100U, 2U, 0x01E0U},
                         {0x6001U, 0x0101U, 2U, 0x01D0U},
                         {0x6002U, 0x0102U, 2U, 0x01C0U},
                         {0x6003U, 0x0103U, 2U, 0x01B0U},
                         {0x6004U, 0x0104U, 2U, 0x01A0U},
                         {0x6005U, 0x0105U, 2U, 0x0190U},
                         {0x6006U, 0x0106U, 2U, 0x0180U}}};
    TitleIntroTargetedSweepState targeted{};
    reset_title_intro_targeted_sweep(targeted);
    std::vector<TitleIntroSweepSpriteRequest> targeted_draws;
    const auto capture_targeted =
        [&targeted_draws](const TitleIntroSweepSpriteRequest& request) {
            targeted_draws.push_back(request);
        };
    targeted.current_frame = 0U;
    advance_title_intro_targeted_sweep(targeted, targeted_phases,
                                       capture_targeted);
    if (targeted.sprite_descriptor != 0x6000U || targeted.current_x != 0x01F7U ||
        targeted.next_event_frame != 2U || targeted.phase != 1U ||
        targeted_draws.front().y != 0x0118U) {
        return 1;
    }
    targeted.current_frame = 1U;
    targeted.target_x = 0x01F6U;
    advance_title_intro_targeted_sweep(targeted, targeted_phases,
                                       capture_targeted);
    if (targeted_draws.size() != 2U) {
        return 2;
    }
    targeted.current_frame = 2U;
    advance_title_intro_targeted_sweep(targeted, targeted_phases,
                                       capture_targeted);
    if (targeted.sprite_descriptor != 0x6001U || targeted.phase != 2U) {
        return 3;
    }

    const std::array<TitleIntroLinearSweepPhase,
                     kTitleIntroLinearSweepPhaseCount>
        linear_phases{{{0x7000U, 0x0200U, 2U}, {0x7001U, 0x0201U, 2U},
                       {0x7002U, 0x0202U, 2U}, {0x7003U, 0x0203U, 2U},
                       {0x7004U, 0x0204U, 2U}, {0x7005U, 0x0205U, 2U},
                       {0x7006U, 0x0206U, 2U}, {0x7007U, 0x0207U, 2U}}};
    TitleIntroLinearSweepState linear{};
    reset_title_intro_linear_sweep_a(linear);
    std::vector<TitleIntroSweepSpriteRequest> linear_draws;
    const auto capture_linear =
        [&linear_draws](const TitleIntroSweepSpriteRequest& request) {
            linear_draws.push_back(request);
        };
    linear.current_frame = 0U;
    advance_title_intro_linear_sweep_a(linear, linear_phases, capture_linear);
    if (linear.current_x != 0x01F8U || linear.sprite_descriptor != 0x7000U ||
        linear_draws.front().y != 0x0218U) {
        return 4;
    }
    linear.current_frame = 1U;
    linear.next_event_frame = 10U;
    linear.current_x = 0x0038U;
    advance_title_intro_linear_sweep_a(linear, linear_phases, capture_linear);
    if (linear_draws.size() != 1U) {
        return 5;
    }
    reset_title_intro_linear_sweep_b(linear);
    linear.current_frame = 0U;
    advance_title_intro_linear_sweep_b(linear, linear_phases, capture_linear);
    if (linear.current_x != 0x01F5U) {
        return 6;
    }
    return 0;
}
