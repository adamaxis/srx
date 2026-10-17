#pragma once

// Live output: pushes each mixed game frame into a QAudioSink. The game
// frame clock stays authoritative; when the device buffer is full the frame
// is dropped rather than blocking the game thread.

#include "audio_mixer.hpp"

#include <memory>

class QAudioSink;
class QIODevice;

namespace shadowrun::ghidra_conversion::qt_runtime {

class QtAudioSink final : public AudioSink {
public:
    QtAudioSink();
    ~QtAudioSink() override;
    bool ok() const { return device_ != nullptr; }
    void submit(const std::int16_t* stereo, std::size_t frames) override;
    std::uint64_t dropped_frames() const { return dropped_; }

private:
    std::unique_ptr<QAudioSink> sink_;
    QIODevice* device_ = nullptr;
    std::uint64_t dropped_ = 0U;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
