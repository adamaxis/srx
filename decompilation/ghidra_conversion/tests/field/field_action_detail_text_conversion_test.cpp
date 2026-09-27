#include "shadowrun_ghidra/field/field_action_detail_text_conversion.hpp"

using namespace shadowrun::ghidra_conversion;

int main() {
    if (!select_field_action_detail_text(0x00U) ||
        select_field_action_detail_text(0x00U)->table_index != 0U) {
        return 1;
    }
    if (!select_field_action_detail_text(0x18U) ||
        select_field_action_detail_text(0x18U)->table_index != 0x18U) {
        return 2;
    }
    // 0x19..0x24 share source table entry 0x18.
    if (!select_field_action_detail_text(0x24U) ||
        select_field_action_detail_text(0x24U)->table_index != 0x18U) {
        return 3;
    }
    if (!select_field_action_detail_text(0x25U) ||
        select_field_action_detail_text(0x25U)->table_index != 0x18U) {
        return 4;
    }
    if (!select_field_action_detail_text(0x27U) ||
        select_field_action_detail_text(0x27U)->table_index != 0x1AU) {
        return 5;
    }
    // The source's 0x28..0x2A range returns without a text transfer.
    if (select_field_action_detail_text(0x28U) ||
        select_field_action_detail_text(0x2AU)) {
        return 6;
    }
    if (!select_field_action_detail_text(0x2BU) ||
        select_field_action_detail_text(0x2BU)->table_index != 0x1BU ||
        !select_field_action_detail_text(0x2FU) ||
        select_field_action_detail_text(0x2FU)->table_index != 0x1FU) {
        return 7;
    }
    if (select_field_action_detail_text(0x30U) ||
        select_field_action_detail_text(0xFFU)) {
        return 8;
    }
    return 0;
}
