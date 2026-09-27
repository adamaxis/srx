#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

// ROM 0xCDF6 reads these fields from the active runner while the Medical
// Attn. handler builds the hospital option costs. The names are now tied to
// the Attributes/Skills record: +0x77 is Body and +0x7C is Willpower.
struct HospitalMedicalAttentionCostInput {
    std::uint8_t class_code = 0;          // source +0x9B
    std::uint8_t body_attribute = 0;      // source +0x77
    std::uint8_t willpower_attribute = 0; // source +0x7C
    // The caller enters 0xCDF6 with D4 = 8, 6, or 4 for the three medical
    // attention rows. It is a decoded service tier, not a ROM address.
    std::uint16_t base_service_level = 0;
};

// ROM 0xCDF6: calculate one Medical Attn. price from the active runner's
// class, Body, Willpower, and caller-selected service tier. The source
// returns the computed D6 word; the surrounding routine compares it with the
// available nuyen before applying the selected treatment.
std::uint16_t calculate_hospital_medical_attention_cost(
    const HospitalMedicalAttentionCostInput& input);

} // namespace shadowrun::ghidra_conversion
