#include "controller_key_mapping.hpp"

using namespace shadowrun::ghidra_conversion;
using namespace shadowrun::ghidra_conversion::qt_runtime;

int main() {
    if (sampled_menu_byte(Qt::Key_A) != kStartupMenuA ||
        sampled_field_button_byte(Qt::Key_A) != kStartupMenuA ||
        sampled_field_button_byte(Qt::Key_Z) != kStartupMenuA ||
        sampled_field_button_byte(Qt::Key_X) != kStartupMenuB ||
        sampled_field_button_byte(Qt::Key_C) != kStartupMenuC ||
        sampled_field_button_byte(Qt::Key_Space) != 0x80U ||
        sampled_field_button_byte(Qt::Key_Return) != 0x80U ||
        sampled_title_byte(Qt::Key_A) != kStartupMenuA ||
        sampled_title_byte(Qt::Key_Space) != kTitleStartButton) {
        return 1;
    }
    return 0;
}
