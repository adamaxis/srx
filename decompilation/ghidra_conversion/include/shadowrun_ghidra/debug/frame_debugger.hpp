#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace shadowrun::ghidra_conversion::debug {

struct KeyboardInputEvent {
    std::uint64_t frame = 0;
    std::int32_t key = 0;
    std::uint32_t modifiers = 0;
    bool pressed = false;
};

struct SourceBoundaryEvent {
    std::uint64_t frame = 0;
    std::string name;
};

struct InputLogConfig {
    enum class Mode { disabled, record, replay } mode = Mode::disabled;
    std::string path;
};

// Records accepted Qt key transitions against source-frame boundaries. Version
// 5 appends input, frame, and event chunks while the game runs; each completed
// frame flushes its seed and inspector fingerprint. A missing final marker
// still leaves earlier complete chunks available for replay.
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
        } else {
            output_.open(config_.path, std::ios::binary | std::ios::trunc);
            if (!output_) {
                throw std::runtime_error("could not open input log for writing: " +
                                         config_.path);
            }
            output_.write(kMagicV5, sizeof(kMagicV5));
            output_.flush();
            if (!output_) {
                throw std::runtime_error("failed while starting input log: " +
                                         config_.path);
            }
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

    bool has_seed_history() const noexcept { return has_seed_history_; }
    bool has_fingerprint_history() const noexcept {
        return has_fingerprint_history_;
    }
    bool has_source_event_history() const noexcept {
        return has_source_event_history_;
    }
    bool has_complete_recording() const noexcept { return complete_recording_; }
    const std::vector<SourceBoundaryEvent>& source_boundary_events() const noexcept {
        return source_boundary_events_;
    }

    void record_event(const std::uint64_t frame, const std::int32_t key,
                      const std::uint32_t modifiers, const bool pressed) {
        if (!recording() || finished_) {
            return;
        }
        if (frame > frame_count_ || frame < last_input_event_frame_) {
            throw std::logic_error(
                "input log event is outside the current frame boundary");
        }
        if (event_count_ >= kMaximumEvents) {
            throw std::length_error("input log contains too many events");
        }
        last_input_event_frame_ = frame;
        ++event_count_;
        std::array<char, 9> payload{};
        encode_u32(payload.data(), static_cast<std::uint32_t>(key));
        encode_u32(payload.data() + 4, modifiers);
        payload[8] = pressed ? '\x01' : '\x00';
        append_chunk(kInputChunk, frame, payload.data(), payload.size());
    }

    void record_source_event(const std::uint64_t frame,
                             const std::string_view name) {
        if (!recording() || finished_ || name.empty()) {
            return;
        }
        if (name.size() > kMaximumSourceEventBytes ||
            source_event_count_ >= kMaximumEvents) {
            throw std::length_error("source event is too large for the input log");
        }
        if (frame > frame_count_ || frame < last_source_event_frame_) {
            throw std::logic_error(
                "source event is outside the current frame boundary");
        }
        last_source_event_frame_ = frame;
        ++source_event_count_;
        append_chunk(kSourceEventChunk, frame, name.data(), name.size());
    }

    void record_completed_frame(const std::uint16_t source_seed,
                                const std::uint64_t inspection_fingerprint = 0U) {
        if (recording() && !finished_) {
            if (frame_count_ >= kMaximumFrames) {
                write_failed_ = true;
                return;
            }
            std::array<char, 10> payload{};
            encode_u16(payload.data(), source_seed);
            encode_u64(payload.data() + 2, inspection_fingerprint);
            append_chunk(kFrameChunk, frame_count_, payload.data(), payload.size());
            ++frame_count_;
            if (!write_failed_) {
                output_.flush();
                write_failed_ = !output_;
            }
        }
    }

    std::optional<std::uint16_t> expected_seed_after_frame(
        const std::uint64_t frame) const noexcept {
        if (!has_seed_history_ || !replaying() || frame >= frame_seeds_.size()) {
            return std::nullopt;
        }
        return frame_seeds_[static_cast<std::size_t>(frame)];
    }

    std::optional<std::uint64_t> expected_fingerprint_after_frame(
        const std::uint64_t frame) const noexcept {
        if (!has_fingerprint_history_ || !replaying() ||
            frame >= frame_fingerprints_.size()) {
            return std::nullopt;
        }
        return frame_fingerprints_[static_cast<std::size_t>(frame)];
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
            if (write_failed_) {
                set_error(error, "failed while writing input log: " + config_.path);
                return false;
            }
            return true;
        }
        finished_ = true;
        append_chunk(kEndChunk, frame_count_, nullptr, 0U);
        output_.flush();
        if (!output_ || write_failed_) {
            write_failed_ = true;
            output_.close();
            set_error(error, "failed while writing input log: " + config_.path);
            return false;
        }
        complete_recording_ = true;
        output_.close();
        return true;
    }

private:
    static constexpr char kMagicV2[8] = {'S', 'R', 'X', 'K', 'E', 'Y', '2', '\0'};
    static constexpr char kMagicV3[8] = {'S', 'R', 'X', 'K', 'E', 'Y', '3', '\0'};
    static constexpr char kMagicV4[8] = {'S', 'R', 'X', 'K', 'E', 'Y', '4', '\0'};
    static constexpr char kMagicV5[8] = {'S', 'R', 'X', 'K', 'E', 'Y', '5', '\0'};
    static constexpr char kInputChunk = 'I';
    static constexpr char kFrameChunk = 'F';
    static constexpr char kSourceEventChunk = 'E';
    static constexpr char kEndChunk = 'X';
    static constexpr std::uint64_t kMaximumEvents = 10'000'000;
    static constexpr std::uint64_t kMaximumFrames = 10'000'000;
    static constexpr std::uint32_t kMaximumSourceEventBytes = 65536U;

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

    static void encode_u16(char* const output, const std::uint16_t value) {
        output[0] = static_cast<char>(value & 0xFFU);
        output[1] = static_cast<char>((value >> 8U) & 0xFFU);
    }

    static void encode_u32(char* const output, const std::uint32_t value) {
        for (unsigned shift = 0; shift < 32; shift += 8) {
            output[shift / 8U] = static_cast<char>((value >> shift) & 0xFFU);
        }
    }

    static void encode_u64(char* const output, const std::uint64_t value) {
        for (unsigned shift = 0; shift < 64; shift += 8) {
            output[shift / 8U] = static_cast<char>((value >> shift) & 0xFFU);
        }
    }

    static std::uint16_t decode_u16(const char* const input) {
        return static_cast<std::uint16_t>(
            static_cast<unsigned char>(input[0]) |
            (static_cast<std::uint16_t>(
                 static_cast<unsigned char>(input[1]))
             << 8U));
    }

    static std::uint32_t decode_u32(const char* const input) {
        std::uint32_t value = 0U;
        for (unsigned shift = 0; shift < 32; shift += 8) {
            value |= static_cast<std::uint32_t>(
                         static_cast<unsigned char>(input[shift / 8U]))
                     << shift;
        }
        return value;
    }

    static std::uint64_t decode_u64(const char* const input) {
        std::uint64_t value = 0U;
        for (unsigned shift = 0; shift < 64; shift += 8) {
            value |= static_cast<std::uint64_t>(
                         static_cast<unsigned char>(input[shift / 8U]))
                     << shift;
        }
        return value;
    }

    void append_chunk(const char type, const std::uint64_t frame,
                      const char* const payload, const std::size_t size) {
        if (write_failed_ || !output_.is_open()) {
            return;
        }
        if (size > std::numeric_limits<std::uint32_t>::max()) {
            write_failed_ = true;
            return;
        }
        output_.put(type);
        write_u64(output_, frame);
        write_u32(output_, static_cast<std::uint32_t>(size));
        if (size != 0U) {
            output_.write(payload, static_cast<std::streamsize>(size));
        }
        write_failed_ = !output_;
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

    void load_chunked(std::istream& stream) {
        has_seed_history_ = true;
        has_fingerprint_history_ = true;
        has_source_event_history_ = true;
        while (true) {
            const auto type_value = stream.get();
            if (type_value == std::char_traits<char>::eof()) {
                break;
            }
            std::array<char, 12> header{};
            stream.read(header.data(),
                        static_cast<std::streamsize>(header.size()));
            if (stream.gcount() != static_cast<std::streamsize>(header.size())) {
                stream.clear();
                break;
            }
            const auto frame = decode_u64(header.data());
            const auto length = decode_u32(header.data() + 8);
            const auto type = static_cast<char>(type_value);
            if (length > kMaximumSourceEventBytes) {
                throw std::runtime_error("input log chunk is too large");
            }
            std::string payload(length, '\0');
            if (length != 0U) {
                stream.read(payload.data(), static_cast<std::streamsize>(length));
                if (stream.gcount() != static_cast<std::streamsize>(length)) {
                    stream.clear();
                    break;
                }
            }

            if (type == kInputChunk) {
                if (length != 9U || frame > frame_count_ ||
                    event_count_ >= kMaximumEvents ||
                    (!events_.empty() && frame < events_.back().frame) ||
                    (payload[8] != '\x00' && payload[8] != '\x01')) {
                    throw std::runtime_error(
                        "input log has an invalid input-event chunk");
                }
                KeyboardInputEvent event;
                event.frame = frame;
                event.key = static_cast<std::int32_t>(
                    decode_u32(payload.data()));
                event.modifiers = decode_u32(payload.data() + 4);
                event.pressed = payload[8] == '\x01';
                events_.push_back(event);
                ++event_count_;
            } else if (type == kFrameChunk) {
                if (length != 10U || frame != frame_count_ ||
                    frame_count_ >= kMaximumFrames) {
                    throw std::runtime_error(
                        "input log has an invalid completed-frame chunk");
                }
                frame_seeds_.push_back(decode_u16(payload.data()));
                frame_fingerprints_.push_back(decode_u64(payload.data() + 2));
                ++frame_count_;
            } else if (type == kSourceEventChunk) {
                if (length == 0U || frame > frame_count_ ||
                    source_event_count_ >= kMaximumEvents ||
                    (!source_boundary_events_.empty() &&
                     frame < source_boundary_events_.back().frame)) {
                    throw std::runtime_error(
                        "input log has an invalid source-event chunk");
                }
                source_boundary_events_.push_back({frame, std::move(payload)});
                ++source_event_count_;
            } else if (type == kEndChunk) {
                if (length != 0U || frame != frame_count_) {
                    throw std::runtime_error(
                        "input log has an invalid completion marker");
                }
                complete_recording_ = true;
                if (stream.peek() != std::char_traits<char>::eof()) {
                    throw std::runtime_error(
                        "input log has data after its completion marker");
                }
                break;
            } else {
                throw std::runtime_error("input log has an unknown chunk type");
            }
        }
    }

    void load() {
        std::ifstream stream(config_.path, std::ios::binary);
        if (!stream) {
            throw std::runtime_error("could not open input log: " + config_.path);
        }
        char magic[sizeof(kMagicV4)]{};
        stream.read(magic, sizeof(magic));
        const bool version_four = std::equal(
            std::begin(kMagicV4), std::end(kMagicV4), magic);
        const bool version_five = std::equal(
            std::begin(kMagicV5), std::end(kMagicV5), magic);
        const bool version_three = std::equal(
            std::begin(kMagicV3), std::end(kMagicV3), magic);
        const bool version_two = std::equal(
            std::begin(kMagicV2), std::end(kMagicV2), magic);
        const bool version_one =
            magic[0] == 'S' && magic[1] == 'R' && magic[2] == 'X' &&
            magic[3] == 'K' && magic[4] == 'E' && magic[5] == 'Y' &&
            magic[6] == '1' && magic[7] == '\0';
        if (!stream || (!version_one && !version_two && !version_three &&
                        !version_four && !version_five)) {
            throw std::runtime_error("input log has an unsupported header: " + config_.path);
        }
        if (version_five) {
            load_chunked(stream);
            return;
        }
        has_seed_history_ = version_two || version_three || version_four;
        has_fingerprint_history_ = version_three || version_four;
        has_source_event_history_ = version_four;
        frame_count_ = read_u64(stream);
        const auto count = read_u64(stream);
        const auto source_event_count = version_four ? read_u64(stream) : 0U;
        if (frame_count_ > kMaximumFrames || count > kMaximumEvents ||
            source_event_count > kMaximumEvents ||
            count > static_cast<std::uint64_t>(events_.max_size())) {
            throw std::runtime_error("input log size is unreasonable");
        }
        if (has_seed_history_) {
            frame_seeds_.reserve(static_cast<std::size_t>(frame_count_));
            for (std::uint64_t frame = 0; frame < frame_count_; ++frame) {
                const auto seed = read_u32(stream);
                if (seed > 0xFFFFU) {
                    throw std::runtime_error("input log contains an invalid random seed");
                }
                frame_seeds_.push_back(static_cast<std::uint16_t>(seed));
            }
        }
        if (has_fingerprint_history_) {
            frame_fingerprints_.reserve(static_cast<std::size_t>(frame_count_));
            for (std::uint64_t frame = 0; frame < frame_count_; ++frame) {
                frame_fingerprints_.push_back(read_u64(stream));
            }
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
        source_boundary_events_.reserve(
            static_cast<std::size_t>(source_event_count));
        for (std::uint64_t index = 0; index < source_event_count; ++index) {
            SourceBoundaryEvent event;
            event.frame = read_u64(stream);
            const auto length = read_u32(stream);
            if (length == 0U || length > kMaximumSourceEventBytes) {
                throw std::runtime_error(
                    "input log contains an invalid source event name length");
            }
            event.name.resize(length);
            stream.read(event.name.data(), static_cast<std::streamsize>(length));
            if (!stream || event.frame > frame_count_ ||
                (!source_boundary_events_.empty() &&
                 event.frame < source_boundary_events_.back().frame)) {
                throw std::runtime_error(
                    "input log source events are truncated or out of frame order");
            }
            source_boundary_events_.push_back(std::move(event));
        }
        if (stream.peek() != std::char_traits<char>::eof()) {
            throw std::runtime_error("input log has trailing data: " + config_.path);
        }
        complete_recording_ = true;
    }

    InputLogConfig config_;
    std::ofstream output_;
    std::vector<KeyboardInputEvent> events_;
    std::vector<std::uint16_t> frame_seeds_;
    std::vector<std::uint64_t> frame_fingerprints_;
    std::vector<SourceBoundaryEvent> source_boundary_events_;
    std::uint64_t frame_count_ = 0;
    std::uint64_t event_count_ = 0;
    std::uint64_t source_event_count_ = 0;
    std::uint64_t last_input_event_frame_ = 0;
    std::uint64_t last_source_event_frame_ = 0;
    std::size_t next_event_ = 0;
    bool finished_ = false;
    bool has_seed_history_ = false;
    bool has_fingerprint_history_ = false;
    bool has_source_event_history_ = false;
    bool complete_recording_ = false;
    bool write_failed_ = false;
};

class FrameStepper final {
public:
    explicit FrameStepper(const bool enabled = false) noexcept : enabled_(enabled) {}

    bool enabled() const noexcept { return enabled_; }
    bool paused() const noexcept { return enabled_ && paused_; }
    std::uint32_t pending_steps() const noexcept { return pending_steps_; }

    void pause() noexcept {
        if (enabled_) {
            paused_ = true;
            pending_steps_ = 0;
        }
    }

    void resume() noexcept {
        if (enabled_) {
            paused_ = false;
            pending_steps_ = 0;
        }
    }

    void toggle_pause() noexcept {
        if (!enabled_) {
            return;
        }
        if (paused_) {
            resume();
        } else {
            pause();
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
