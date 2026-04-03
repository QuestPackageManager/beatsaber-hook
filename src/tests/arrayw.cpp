#include "arrayw.hpp"

#include "tests.hpp"

static_assert(i2c::type_check::full_type<Array<int>*>);
static_assert(i2c::type_check::ptr_ref_type<Array<int>*>);

static_assert(i2c::type_check::full_type<ArrayW<int>>);
static_assert(i2c::type_check::wrapper_ref_type<ArrayW<int>>);

TEST(arrayw) {
    LOG_OK("Starting ArrayW tests");

    ArrayW<int> a = ArrayW<int>({1, 2, 3});
    LOG_OK("Created ArrayW<int> a size -> {} | first -> {} | last -> {}", a.size(), a.front(), a.back());
    LOG_OK("iterated values -> {}", a);

    ArrayW<int> b(5);
    LOG_OK("Allocated ArrayW<int> b length -> {}", b.size());

    LOG_OK("a.empty() -> {}", a.empty());

    // Test const variants
    ArrayW<int> const const_a = {4, 5, 6};
    LOG_OK("const_a size -> {} | front -> {} | back -> {}", const_a.size(), const_a.front(), const_a.back());

    // Test operator[]
    LOG_OK("a[1] -> {}", a[1]);
    a[1] = 42;
    LOG_OK("After a[1] = 42, a[1] -> {}", a[1]);

    // Test at() and try_get()
    try {
        LOG_OK("a.at(0) -> {}", a.at(0));
        LOG_OK("a.at(2) -> {}", a.at(2));
        auto opt = a.try_get(1);
        if (opt) {
            LOG_OK("a.try_get(1) -> {}", opt->get());
        }
        auto opt_out = a.try_get(10);
        LOG_OK("a.try_get(10) has value -> {}", opt_out.has_value());
    } catch (std::exception const& e) {
        LOG_FAIL("Exception in at/try_get: {}", e.what());
    }

    // Test const at/try_get
    try {
        LOG_OK("const_a.at(0) -> {}", const_a.at(0));
        auto const_opt = const_a.try_get(1);
        if (const_opt) {
            LOG_OK("const_a.try_get(1) -> {}", const_opt->get());
        }
    } catch (std::exception const& e) {
        LOG_FAIL("Exception in const at/try_get: {}", e.what());
    }

    // Test find functions
    ArrayW<int> c = {10, 20, 30, 20, 40};
    auto it = c.find(20);
    LOG_OK("c.find(20) != end() -> {}", it != c.end());
    if (it != c.end()) {
        LOG_OK("Found at index -> {}", std::distance(c.begin(), it));
    }

    auto const_it = const_a.find(5);
    LOG_OK("const_a.find(5) != end() -> {}", const_it != const_a.end());

    // Test find_if
    auto it_if = c.find_if([](int x) { return x > 25; });
    LOG_OK("c.find_if(x > 25) != end() -> {}", it_if != c.end());
    if (it_if != c.end()) {
        LOG_OK("Found value -> {}", *it_if);
    }

    // Test rfind
    auto rit = c.rfind(20);
    LOG_OK("c.rfind(20) != rend() -> {}", rit != c.rend());

    // Test front/back with predicate
    auto front_pred = c.front([](int x) { return x % 10 == 0; });
    LOG_OK("c.front(x % 10 == 0) -> {}", front_pred);

    auto back_pred = c.back([](int x) { return x % 10 == 0; });
    LOG_OK("c.back(x % 10 == 0) -> {}", back_pred);

    // Test front_or_default, back_or_default
    ArrayW<int> d = {1, 3, 5};
    auto front_def = d.front_or_default([](int x) { return x == 2; });
    LOG_OK("d.front_or_default(x == 2) -> {}", front_def);

    auto back_def = d.back_or_default([](int x) { return x == 4; });
    LOG_OK("d.back_or_default(x == 4) -> {}", back_def);

    // Test contains
    LOG_OK("c.contains(30) -> {}", c.contains(30));
    LOG_OK("c.contains(50) -> {}", c.contains(50));

    // Test index_of
    LOG_OK("c.index_of(20) -> {}", c.index_of(20));
    LOG_OK("c.index_of(50) -> {}", c.index_of(50));

    // Test copy_to
    std::vector<int> dest(5);
    c.copy_to(dest, 0);
    LOG_OK("After copy_to, dest -> {}", fmt::join(dest, ", "));

    // Test iterators
    LOG_OK("Iterating c forward:");
    for (auto val : c) {
        LOG_OK("  {}", val);
    }

    LOG_OK("Iterating c backward:");
    for (auto it = c.rbegin(); it != c.rend(); ++it) {
        LOG_OK("  {}", *it);
    }

    // Test ref_to
    auto span = c.ref_to();
    LOG_OK("c.ref_to() size -> {}", span.size());

    auto const_span = const_a.ref_to();
    LOG_OK("const_a.ref_to() size -> {}", const_span.size());

    // Test type conversion
    ArrayW<float> e = {1, 2, 3};
    LOG_OK("ArrayW<float> e from {{1, 2, 3}} -> {}", e);

    // Test assignment operator
    a = const_a;
    LOG_OK("Assigned a = const_a. a size -> {} | a[1] -> {}", a.size(), a[1]);

    // Test move assignment
    ArrayW<int> f = std::move(a);
    LOG_OK("Move assigned f = std::move(a). f size -> {} | f[1] -> {}", f.size(), f[1]);

    // Test bool operator
    LOG_OK("f is non-null -> {}", static_cast<bool>(f));
}
