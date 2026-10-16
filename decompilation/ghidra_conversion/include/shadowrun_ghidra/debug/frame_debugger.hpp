#pragma once

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace shadowrun::ghidra_conversion::debug {

struct KeyboardInputEvent {
    std::uint64_t frame = 0;
    std::int32_t key = 0;
    std::uint32_t modifiers = 0;
    bool pressed = false;
};

struct InputLogConfig {
    enum class Mode { disabled, record, replay } mode = Mode::disabled;
    std::string path;
};

// Records accepted Qt key transitions against the number of completed source
// frames. Replay injects them at the same source-frame boundary. This keeps
// the log independent of Qt's wall-clock timer jitter while preserving the
// existing game input path.
class KeyboardInputLog final {
public:
    KeyboardInputLog() = default;

    explicit KeyboardInputLog(InputLogConfig config)
        : config_(std::move(config)) {
        if (config_.mode == InputLogConfig::Mode::disabled) {
            return;
        }
        if (config_.path.empty()) {
            throw std::invalid_argument("input log path is empty");
        }
        if (config_.mode == InputLogConfig::Mode::replay) {
            load();
        }
    }

    bool recording() const noexcept {
        return config_.mode == InputLogConfig::Mode::record;
    }

    bool replaying() const noexcept {
        return config_.mode == InputLogConfig::Mode::replay;
    }

    std::uint64_t recorded_frame_count() const noexcept {
        return frame_count_;
    }

    void record_event(const std::uint64_t frame, const std::int32_t key,
                      const std::uint32_t modifiers, const bool pressed) {
        if (!recording()) {
            return;
        }
        if (!events_.empty() && frame < events_.back().frame) {
            throw std::logic_error("input log frame order moved backwards");
        }
        events_.push_back({frame, key, modifiers, pressed});
    }

    void record_completed_frame() noexcept {
        if (recording() && frame_count_ != std::numeric_limits<std::uint64_t>::max()) {
            ++frame_count_;
        }
    }

    std::vector<KeyboardInputEvent> take_events_for_frame(
        const std::uint64_t frame) {
        std::vector<KeyboardInputEvent> result;
        while (next_event_ < events_.size() &&
               events_[next_event_].frame == frame) {
            result.push_back(events_[next_event_++]);
        }
        if (next_event_ < events_.size() && events_[next_event_].frame < frame) {
            throw std::logic_error("input log contains an event before the replay cursor");
        }
        return result;
    }

    bool finish(std::string* error = nullptr) {
        if (!recording() || finished_) {
            return true;
        }
        finished_ = true;
        std::ofstream stream(config_.path, std::ios::binary | std::ios::trunc);
        if (!stream) {
            set_error(error, "could not open input log for writing: " + config_.path);
            return false;
        }
        stream.write(kMagic, sizeof(kMagic));
        write_u64(stream, frame_count_);
        write_u64(stream, static_cast<std::uint64_t>(events_.size()));
        for (const auto& event : events_) {
            write_u64(stream, event.frame);
            write_u32(stream, static_cast<std::uint32_t>(event.key));
            write_u32(stream, event.modifiers);
            stream.put(event.pressed ? '\x01' : '\x00');
        }
        stream.flush();
        if (!stream) {
            set_error(error, "failed while writing input log: " + config_.path);
            return false;
        }
        return true;
    }

private:
    static constexpr char kMagic[8] = {'S', 'R', 'X', 'K', 'E', 'Y', '1', '\0'};
    static constexpr std::uint64_t kMaximumEvents = 10'000'000;

    static void set_error(std::string* error, std::string message) {
        if (error != nullptr) {
            *error = std::move(message);
        }
    }

    static void write_u32(std::ostream& stream, const std::uint32_t value) {
        for (unsigned shift = 0; shift < 32; shift += 8) {
            stream.put(static_cast<char>((value >> shift) & 0xFFU));
        }
    }

    static void write_u64(std::ostream& stream, const std::uint64_t value) {
        for (unsigned shift = 0; shift < 64; shift += 8) {
            stream.put(static_cast<char>((value >> shift) & 0xFFU));
        }
    }

    static std::uint32_t read_u32(std::istream& stream) {
        std::uint32_t value = 0;
        for (unsigned shift = 0; shift < 32; shift += 8) {
            const auto byte = stream.get();
            if (byte == std::char_traits<char>::eof()) {
                throw std::runtime_error("truncated input log");
            }
            value |= static_cast<std::uint32_t>(
                         static_cast<unsigned char>(byte))
                     << shift;
        }
        return value;
    }

    static std::uint64_t read_u64(std::istream& stream) {
        std::uint64_t value = 0;
        for (unsigned shift = 0; shift < 64; shift += 8) {
            const auto byte = stream.get();
            if (byte == std::char_traits<char>::eof()) {
                throw std::runtime_error("truncated input log");
            }
            value |= static_cast<std::uint64_t>(
                         static_cast<unsigned char>(byte))
                     << shift;
        }
        return value;
    }

    void load() {
        std::ifstream stream(config_.path, std::ios::binary);
        if (!stream) {
            throw std::runtime_error("could not open input log: " + config_.path);
        }
        char magic[sizeof(kMagic)]{};
        stream.read(magic, sizeof(magic));
        if (!stream || !std::equal(std::begin(kMagic), std::end(kMagic), magic)) {
            throw std::runtime_error("input log has an unsupported header: " + config_.path);
        }
        frame_count_ = read_u64(stream);
        const auto count = read_u64(stream);
        if (count > kMaximumEvents ||
            count > static_cast<std::uint64_t>(events_.max_size())) {
            throw std::runtime_error("input log event count is unreasonable");
        }
        events_.reserve(static_cast<std::size_t>(count));
        for (std::uint64_t index = 0; index < count; ++index) {
            KeyboardInputEvent event;
            event.frame = read_u64(stream);
            event.key = static_cast<std::int32_t>(read_u32(stream));
            event.modifiers = read_u32(stream);
            const auto pressed = stream.get();
            if (pressed != 0 && pressed != 1) {
                throw std::runtime_error("input log contains an invalid key state");
            }
            event.pressed = pressed == 1;
            if (event.frame > frame_count_ ||
                (!events_.empty() && event.frame < events_.back().frame)) {
                throw std::runtime_error("input log events are not frame ordered");
            }
            events_.push_back(event);
        }
        if (stream.peek() != std::char_traits<char>::eof()) {
            throw std::runtime_error("input log has trailing data: " + config_.path);
        }
    }

    InputLogConfig config_;
    std::vector<KeyboardInputEvent> events_;
    std::uint64_t frame_count_ = 0;
    std::size_t next_event_ = 0;
    bool finished_ = false;
};

class FrameStepper final {
public:
    explicit FrameStepper(const bool enabled = false) noexcept : enabled_(enabled) {}

    bool enabled() const noexcept { return enabled_; }
    bool paused() const noexcept { return enabled_ && paused_; }

    void toggle_pause() noexcept {
        if (!enabled_) {
            return;
        }
        paused_ = !paused_;
        if (!paused_) {
            pending_steps_ = 0;
        }
    }

    void queue_step(const std::uint32_t frames) noexcept {
        if (!paused() || frames == 0U) {
            return;
        }
        const auto room = std::numeric_limits<std::uint32_t>::max() - pending_steps_;
        pending_steps_ += std::min(frames, room);
    }

    std::uint32_t frames_to_advance(const std::uint32_t due_frames) noexcept {
        if (!paused()) {
            return due_frames;
        }
        const auto result = std::min(due_frames, pending_steps_);
        pending_steps_ -= result;
        return result;
    }

private:
    bool enabled_ = false;
    bool paused_ = false;
    std::uint32_t pending_steps_ = 0;
};

} // namespace shadowrun::ghidra_conversion::debug
