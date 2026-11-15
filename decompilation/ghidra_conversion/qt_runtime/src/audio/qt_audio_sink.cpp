#include "qt_audio_sink.hpp"

#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSink>
#include <QIODevice>
#include <QMediaDevices>
#include <QObject>

namespace shadowrun::ghidra_conversion::qt_runtime {

QtAudioSink::QtAudioSink() : devices_(std::make_unique<QMediaDevices>()) {
    QObject::connect(devices_.get(), &QMediaDevices::audioOutputsChanged,
                     [this] { outputs_changed_.store(true); });
    open_default_device();
}

QtAudioSink::~QtAudioSink() {
    close_device();
}

bool QtAudioSink::open_default_device() {
    const auto device = QMediaDevices::defaultAudioOutput();
    if (device.isNull()) {
        return false;
    }
    QAudioFormat format;
    format.setSampleRate(static_cast<int>(kAudioSampleRate));
    format.setChannelCount(2);
    format.setSampleFormat(QAudioFormat::Int16);
    if (!device.isFormatSupported(format)) {
        return false;
    }
    sink_ = std::make_unique<QAudioSink>(device, format);
    // About four game frames of device buffer (~67 ms).
    sink_->setBufferSize(static_cast<qsizetype>(kAudioSamplesPerFrame * 4U * 4U));
    device_ = sink_->start();
    device_id_ = device.id();
    return device_ != nullptr;
}

void QtAudioSink::close_device() {
    if (sink_) {
        sink_->stop();
    }
    device_ = nullptr;
    sink_.reset();
    device_id_.clear();
}

void QtAudioSink::follow_default_device() {
    const bool changed = outputs_changed_.exchange(false);
    const bool failed = sink_ != nullptr &&
                        (sink_->error() == QAudio::OpenError ||
                         sink_->error() == QAudio::IOError ||
                         sink_->error() == QAudio::FatalError);
    if (!changed && !failed) {
        return;
    }
    if (!failed && QMediaDevices::defaultAudioOutput().id() == device_id_ &&
        device_ != nullptr) {
        return; // device list changed, default output did not
    }
    close_device();
    open_default_device();
}

void QtAudioSink::submit(const std::int16_t* stereo, const std::size_t frames) {
    follow_default_device();
    if (device_ == nullptr) {
        return;
    }
    // Queue first, then hand the device as much as it will take; keep any
    // short-write remainder in order for the next game frame (playtest #35).
    const auto before = queue_.dropped_bytes();
    queue_.push(reinterpret_cast<const char*>(stereo), frames * 4U);
    dropped_ += (queue_.dropped_bytes() - before) / 4U;
    const auto free_bytes = sink_->bytesFree();
    if (free_bytes <= 0) {
        return;
    }
    queue_.flush(static_cast<std::size_t>(free_bytes),
                 [this](const char* data, std::size_t n) {
                     return device_->write(data, static_cast<qint64>(n));
                 });
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
