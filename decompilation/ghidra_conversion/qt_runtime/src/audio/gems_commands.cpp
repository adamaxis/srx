#include "gems_commands.hpp"

#include <utility>

namespace shadowrun::ghidra_conversion::qt_runtime {

GemsCommandLayer::GemsCommandLayer(AudioMixer& mixer) : mixer_(mixer) {}

void GemsCommandLayer::run(std::function<void()> operation) {
    if (blocked()) {
        deferred_.push_back(std::move(operation));
        return;
    }
    operation();
}

void GemsCommandLayer::play_sfx(const std::uint16_t id) {
    run([this, id] {
        if (sfx_enabled_) {
            mixer_.start(id);
        }
    });
}

void GemsCommandLayer::play_music(const std::uint16_t id) {
    run([this, id] {
        if (music_enabled_) {
            mixer_.stop(id);   // 0x530F4 bsr 0x53112: same id only [C]
            mixer_.start(id);
        }
    });
}

void GemsCommandLayer::stop_sfx(const std::uint16_t id) {
    run([this, id] {
        if (sfx_enabled_) {
            mixer_.stop(id);
        }
    });
}

void GemsCommandLayer::stop_all() {
    run([this] { mixer_.stop_all(); });
}

void GemsCommandLayer::attenuate(const std::uint8_t level) {
    run([this, level] { mixer_.attenuate(level); });
}

void GemsCommandLayer::fade_out() {
    run([this] {
        fade_frame_ = 0U;
        step_fade();
    });
}

void GemsCommandLayer::step_fade() {
    if (!fade_frame_) {
        return;
    }
    const auto elapsed = *fade_frame_;
    constexpr std::uint32_t kEnd = kGemsFadeSteps * kGemsFadeStepFrames;
    if (elapsed < kEnd) {
        if (elapsed % kGemsFadeStepFrames == 0U) {
            const auto step = elapsed / kGemsFadeStepFrames + 1U;
            mixer_.attenuate(static_cast<std::uint8_t>(step * kGemsFadeStep));
        }
        return;
    }
    // 0x53146 jsr 0x52FA0 (op 0x16), then 0x5314C bsr 0x53156 (op 0x20, 0).
    mixer_.stop_all();
    mixer_.attenuate(0U);
    fade_frame_.reset();
    drain_deferred();
}

void GemsCommandLayer::wait_frames(const std::uint16_t frames) {
    run([this, frames] { wait_frames_ = frames; });
}

void GemsCommandLayer::drain_deferred() {
    while (!deferred_.empty() && !blocked()) {
        auto operation = std::move(deferred_.front());
        deferred_.pop_front();
        operation();
    }
}

void GemsCommandLayer::advance_frame(AudioSink* sink) {
    mixer_.advance_frame(sink);
    if (fade_frame_) {
        ++*fade_frame_;
        step_fade();
    } else if (wait_frames_ != 0U && --wait_frames_ == 0U) {
        drain_deferred();
    }
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
