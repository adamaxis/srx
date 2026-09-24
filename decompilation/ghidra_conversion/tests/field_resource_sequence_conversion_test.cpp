#include "shadowrun_ghidra/field_resource_sequence_conversion.hpp"

#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<int> order;
    prepare_field_resources({[&order] { order.push_back(1); },
                             [&order] { order.push_back(2); },
                             [&order] { order.push_back(3); },
                             [&order] { order.push_back(4); }});
    if (order != std::vector<int>{1, 2, 3, 4}) {
        return 1;
    }
    return 0;
}
