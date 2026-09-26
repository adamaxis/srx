#include "shadowrun_ghidra/field/mode_optional_resource_conversion.hpp"

int main() {
    bool pending = true;
    shadowrun::ghidra_conversion::clear_optional_mode_resource(pending);
    return pending ? 1 : 0;
}
