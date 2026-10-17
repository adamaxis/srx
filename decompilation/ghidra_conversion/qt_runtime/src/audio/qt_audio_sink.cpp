#include "qt_audio_sink.hpp"

#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSink>
#include <QIODevice>
#include <QMediaDevices>

namespace shadowrun::ghidra_conversion::qt_runtime {

QtAudioSink::QtAudioSink() {
    const auto device = QMediaDevices::defaultAudioOutput();
    if (device.isNull()) {
        return;
    }
    QAudioFormat format;
    format.setSampleRate(static_cast<int>(kAudioSampleRate));
    format.setChannelCount(2);
    format.setSampleFormat(QAudioFormat::Int16);
    if (!device.isFormatSupported(format)) {
        return;
    }
    sink_ = std::make_unique<QAudioSink>(device, format);
    // About four game frames of device buffer (~67 ms).
    sink_->setBufferSize(static_cast<qsizetype>(kAudioSamplesPerFrame * 4U * 4U));
    device_ = sink_->start();
}

QtAudioSink::~QtAudioSink() {
    if (sink_) {
        sink_->stop();
    }
}

void QtAudioSink::submit(const std::int16_t* stereo, const std::size_t frames) {
    if (device_ == nullptr) {
        return;
    }
    const auto bytes = static_cast<qint64>(frames * 4U);
    if (sink_->bytesFree() < bytes) {
        dropped_ += frames;
        return;
    }
    device_->write(reinterpret_cast<const char*>(stereo), bytes);
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
