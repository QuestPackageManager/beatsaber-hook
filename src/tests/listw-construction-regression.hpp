#pragma once

#include "listw.hpp"
#include "members.hpp"
#include "stringw.hpp"

#include <array>
#include <ranges>

namespace listw_construction_regression {
template <class Check>
void run(Check check) {
    std::array<int, 3> const expected{7, 13, 29};
    auto check_values = [&](ListW<int> list, char const* name) {
        check(list.size() == expected.size() &&
              i2c::run_method<int>(list, "get_Count") == static_cast<int>(expected.size()) &&
              std::ranges::equal(list, expected), name);
    };

    check_values(ListW<int>({7, 13, 29}), "initializer-list count and contents");
    std::vector<int> const source(expected.begin(), expected.end());
    auto from_vector = ListW<int>(source);
    check_values(from_vector, "vector count and contents");
    check_values(ListW<int>(std::span<int const>(expected)), "const span count and contents");
    check_values(ListW<int>(i2c::view<int>(source)), "view count and contents");

    // A managed consumer must see the copied items, and appending must not
    // overwrite item zero because the constructor left Count at zero.
    check(i2c::run_method<int>(from_vector, "get_Item", 1) == 13, "managed indexed access");
    i2c::run_method<void>(from_vector, "Add", 41);
    std::array<int, 4> const appended{7, 13, 29, 41};
    check(from_vector.size() == 4 && std::ranges::equal(from_vector, appended), "managed append preserves contents");

    auto empty_vector = ListW<int>(std::vector<int>{});
    auto empty_span = ListW<int>(std::span<int const>{});
    auto empty_initializer = ListW<int>(std::initializer_list<int>{});
    for (auto empty : {empty_vector, empty_span, empty_initializer}) {
        check(empty.empty() && empty.begin() == empty.end() &&
              i2c::run_method<int>(empty, "get_Count") == 0, "empty input remains empty");
    }

    auto capacity_only = ListW<int>::New(8);
    check(capacity_only.empty() && i2c::run_method<int>(capacity_only, "get_Count") == 0,
          "capacity constructor remains empty");
    capacity_only.push_back(53);
    check(capacity_only.size() == 1 && capacity_only.front() == 53, "append to capacity-only list");

    StringW text("ListW constructor regression");
    std::vector<StringW::ptr> const raw_strings{text};
    auto wrapped_strings = ListW<StringW>(raw_strings);
    check(wrapped_strings.size() == 1 && i2c::run_method<int>(wrapped_strings, "get_Count") == 1 &&
          wrapped_strings.front().convert() == text.convert(), "converting view count and reference identity");
}
} // namespace listw_construction_regression
