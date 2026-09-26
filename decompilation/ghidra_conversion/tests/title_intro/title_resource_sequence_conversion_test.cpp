#include "shadowrun_ghidra/title_intro/title_resource_sequence_conversion.hpp"

#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<int> order;
    load_title_resources({[&order] { order.push_back(1); },
                          [&order] { order.push_back(2); },
                          [&order] { order.push_back(3); }});
    if (order != std::vector<int>{1, 2, 1, 1, 3}) {
        return 1;
    }

    order.clear();
    load_title_resources({[&order] { order.push_back(1); }, {}, {}});
    if (order != std::vector<int>{1, 1, 1}) {
        return 2;
    }

    prepare_title_sprite_list([&order] { order.push_back(4); });
    if (order != std::vector<int>{1, 1, 1, 4}) {
        return 3;
    }
    return 0;
}
