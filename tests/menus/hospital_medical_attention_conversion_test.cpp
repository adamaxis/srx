#include "shadowrun_ghidra/menus/hospital_medical_attention_conversion.hpp"

using namespace shadowrun::ghidra_conversion;

int main() {
    // Base tier 4 with Body/Willpower 6 reduces D4 to the source minimum 2,
    // selecting the 100 percent factor and returning Body as the charge.
    if (calculate_hospital_medical_attention_cost(
            {2U, 6U, 6U, 4U}) != 6U) {
        return 1;
    }

    // The source's class-under-two gate adds two to D4. Zero attributes each
    // become -1 after the source ceil(value / 3) - 1 operation.
    if (calculate_hospital_medical_attention_cost(
            {0U, 0U, 0U, 4U}) != 1U) {
        return 2;
    }

    // Body 11, Willpower 4, and base tier 8 select factor 60. 660 / 100 has
    // remainder 60, so the source half-up step returns seven.
    if (calculate_hospital_medical_attention_cost(
            {2U, 11U, 4U, 8U}) != 7U) {
        return 3;
    }

    // Base tier 10 reaches the fixed factor-ten branch; the minimum charge
    // still applies after integer division.
    if (calculate_hospital_medical_attention_cost(
            {2U, 11U, 1U, 10U}) != 1U) {
        return 4;
    }

    return 0;
}
