#include "shadowrun_ghidra/scene_profile_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    if (!expect(scene_uses_alternate_profile(6) &&
                scene_uses_alternate_profile(9) &&
                scene_uses_alternate_profile(0x2C) &&
                scene_uses_alternate_profile(0x31) &&
                !scene_uses_alternate_profile(5) &&
                !scene_uses_alternate_profile(10) &&
                !scene_uses_alternate_profile(0x2B) &&
                !scene_uses_alternate_profile(0x32) &&
                !scene_uses_alternate_profile(-1))) {
        return 1;
    }

    return 0;
}
