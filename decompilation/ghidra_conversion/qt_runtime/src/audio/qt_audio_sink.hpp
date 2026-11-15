#pragma once

// Live output: pushes each mixed game frame into a QAudioSink through a
// bounded PcmQueue. The game frame clock stays authoritative; short device
// writes are carried over in order, and only an over-full backlog (about
// eight game frames) discards its oldest samples. The sink follows the host's
// default output: when the OS default device changes (or the current device
// errors out, e.g. a headset is unplugged) it reopens on the new default.

#include "audio_mixer.hpp"
#include "pcm_queue.hpp"

#include <QByteArray>

#include <atomic>
#include <memory>

class QAudioSink;
class QMediaDevices;
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
    // Opens the current default output; returns false when none is usable.
    bool open_default_device();
    void close_device();
    // Reopens when the default output changed or the device reported an error.
    void follow_default_device();

    std::unique_ptr<QMediaDevices> devices_;
    std::atomic<bool> outputs_changed_{false};
    QByteArray device_id_;
    std::unique_ptr<QAudioSink> sink_;
    QIODevice* device_ = nullptr;
    std::uint64_t dropped_ = 0U;
    PcmQueue queue_{kAudioSamplesPerFrame * 4U * 8U};
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
