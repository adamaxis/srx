#pragma once

#include "shadowrun/native_port.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace shadowrun::native {

// The opening owns its presentation resources as one scene boundary. The
// startup menus and gameplay do not need to know how these layers are timed
// or composited.
struct IntroSceneAssets {
    std::vector<std::string> story_lines;
    std::array<NativeImage, kNativeIntroStageCount> stages{};
    NativeImage aircraft_plane_a;
    NativeImage trivid_background;
    std::vector<NativeIntroSprite> sprites;
    std::array<NativeImage, 4> background_patches{};

    // The scene owns the copy it uses for story text so moving the Qt game
    // object cannot leave a self-referential pointer behind.
    NativeImage font;
};

enum class IntroSceneResult {
    Running,
    Skipped,
    Finished,
};

class IntroScene final {
public:
    explicit IntroScene(IntroSceneAssets assets);

    void reset();
    IntroSceneResult update(const InputState& input);
    void render(ScreenWriter& screen) const;

    bool has_story() const { return !assets_.story_lines.empty(); }
    std::size_t line_start() const { return line_start_; }
    std::uint64_t frame() const { return frame_; }
    std::size_t story_line_count() const { return assets_.story_lines.size(); }

private:
    void write_story_text(ScreenWriter& screen, std::int32_t x,
                          std::int32_t y, std::string_view text) const;

    IntroSceneAssets assets_;
    std::size_t line_start_ = 0;
    std::uint64_t frame_ = 0;
    std::uint8_t host_tick_ = 0;
};

} // namespace shadowrun::native
