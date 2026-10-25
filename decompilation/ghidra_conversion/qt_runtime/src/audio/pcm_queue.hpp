#pragma once

// Host-side PCM FIFO between the frame-clocked mixer and a pull/limited
// output device (playtest #35 crackle). The old sink dropped a whole
// 735-sample frame whenever the device had less free space than one frame
// and ignored short writes, which produced audible gaps. This queue keeps
// unwritten bytes in order and only discards the OLDEST whole frames when
// the backlog exceeds its cap, so the game clock never blocks and samples are
// never duplicated or reordered. Not a ROM structure.

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <deque>
#include <functional>

namespace shadowrun::ghidra_conversion::qt_runtime {

class PcmQueue {
public:
    explicit PcmQueue(std::size_t max_bytes, std::size_t frame_bytes = 4U)
        : max_bytes_(max_bytes), frame_bytes_(frame_bytes) {}

    void push(const char* data, std::size_t bytes) {
        bytes_.insert(bytes_.end(), data, data + bytes);
        if (bytes_.size() > max_bytes_) {
            auto excess = bytes_.size() - max_bytes_;
            excess += (frame_bytes_ - excess % frame_bytes_) % frame_bytes_;
            excess = std::min(excess, bytes_.size());
            bytes_.erase(bytes_.begin(),
                         bytes_.begin() + static_cast<std::ptrdiff_t>(excess));
            dropped_bytes_ += excess;
        }
    }

    // `write(ptr, n)` returns how many bytes the device accepted (<= n, or
    // negative on error). `available` limits one flush.
    template <typename Write>
    std::size_t flush(std::size_t available, Write&& write) {
        std::size_t written = 0U;
        char chunk[4096];
        while (!bytes_.empty() && available > 0U) {
            auto n = std::min({available, bytes_.size(), sizeof(chunk)});
            std::copy_n(bytes_.begin(), n, chunk);
            const auto accepted = write(chunk, n);
            if (accepted <= 0) {
                break;
            }
            const auto taken = static_cast<std::size_t>(accepted);
            bytes_.erase(bytes_.begin(),
                         bytes_.begin() + static_cast<std::ptrdiff_t>(taken));
            written += taken;
            available -= taken;
            if (taken < n) {
                break;
            }
        }
        return written;
    }

    std::size_t pending_bytes() const { return bytes_.size(); }
    std::uint64_t dropped_bytes() const { return dropped_bytes_; }

private:
    std::deque<char> bytes_;
    std::size_t max_bytes_;
    std::size_t frame_bytes_;
    std::uint64_t dropped_bytes_ = 0U;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
