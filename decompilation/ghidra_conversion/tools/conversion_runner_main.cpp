#include "shadowrun_ghidra/conversion_workbench.hpp"

#include <iostream>

int main() {
    const auto report =
        shadowrun::ghidra_conversion::run_conversion_workbench();
    std::cout << shadowrun::ghidra_conversion::format_conversion_workbench_report(
        report);
    return report.passed ? 0 : 1;
}
