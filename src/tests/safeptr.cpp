#include "safeptr.hpp"

#include "members.hpp"
#include "stringw.hpp"
#include "tests.hpp"

TEST(safeptr_and_countpointer) {
    LOG_OK("Starting safe_ptr / CountPointer tests (reference types only)");

    // Use a reference-type instance (Il2CppObject) instead of primitives
    Il2CppObject inst{};
    LOG_OK("Created stack Il2CppObject inst at {}", fmt::ptr(&inst));

    // Default constructed safe_ptr
    safe_ptr<Il2CppObject*> a;
    LOG_OK("Created default safe_ptr<Il2CppObject> a (bool: {})", static_cast<bool>(a));

    safe_ptr<Il2CppObject*> b(&inst);
    LOG_OK("Created safe_ptr<Il2CppObject> b(&inst). Counter for &inst -> {}", i2c::detail::get_count(&inst));

    {
        i2c::detail::count_ptr<Il2CppObject> c_ptr(&inst);
        LOG_OK(
            "Created CountPointer<Il2CppObject> c_ptr(&inst). c_ptr.count() -> {} | Counter for &inst -> {}",
            c_ptr.count(),
            i2c::detail::get_count(&inst)
        );
    }
    LOG_OK("After destroying CountPointer, Counter for &inst -> {}", i2c::detail::get_count(&inst));

    // Log pointer addresses rather than treating object as a value
    LOG_OK("b.ptr() -> {}", fmt::ptr(b.ptr()));

    // Test assignment
    a = b.ptr();
    LOG_OK("a.ptr() -> {}", fmt::ptr(a.ptr()));

    // Create a temporary copy
    {
        safe_ptr<Il2CppObject*> c(b);
        LOG_OK("Copied safe_ptr c(b). Counter for &inst -> {} | c.ptr() -> {}", i2c::detail::get_count(&inst), fmt::ptr(c.ptr()));
    }
    LOG_OK("After destroying copy, Counter for &inst -> {}", i2c::detail::get_count(&inst));

    // Pass by reference-like usage
    auto test_ref = [&](safe_ptr<Il2CppObject*>& ref) {
        LOG_OK("In test_ref, received ref.ptr() -> {}", fmt::ptr(ref.ptr()));
    };
    test_ref(b);
    LOG_OK("After test_ref, Counter for &inst -> {}", i2c::detail::get_count(&inst));

    // Pass by value (copy)
    auto test_copy = [&](safe_ptr<Il2CppObject*> copy) {
        LOG_OK("In test_copy, received copy.ptr() -> {}", fmt::ptr(copy.ptr()));
    };
    test_copy(b);
    LOG_OK("After test_copy, Counter for &inst -> {}", i2c::detail::get_count(&inst));

    // Literal pointer cast (discouraged) — pass raw pointer
    auto test_literal = [&](Il2CppObject* p) {
        LOG_OK("In test_literal, received raw ptr -> {}", fmt::ptr(p));
    };
    test_literal((Il2CppObject*) b.ptr());
    LOG_OK("After test_literal, Counter for &inst -> {}", i2c::detail::get_count(&inst));

    // Final pointer log
    LOG_OK("Final b.ptr() -> {} | Counter for &inst -> {}", fmt::ptr(b.ptr()), i2c::detail::get_count(&inst));

    LOG_OK("After leaving scope, Counter for &inst -> {}", i2c::detail::get_count(&inst));
}

TEST(countptr_assignment) {
    LOG_OK("Starting count_ptr copy/move assignment tests");

    Il2CppObject inst1{};
    Il2CppObject inst2{};

    i2c::detail::count_ptr<Il2CppObject> a(&inst1);
    i2c::detail::count_ptr<Il2CppObject> b(&inst2);

    // Copy assignment: a should now point at inst2, inst1's count should drop, inst2's count should rise.
    a = b;
    if (a.get() != &inst2) {
        LOG_FAIL("count_ptr copy assignment did not update held pointer");
    }
    if (i2c::detail::get_count(&inst1) != 0) {
        LOG_FAIL("count_ptr copy assignment did not release previous pointer's count (inst1 count -> {})", i2c::detail::get_count(&inst1));
    }
    if (i2c::detail::get_count(&inst2) != 2) {
        LOG_FAIL("count_ptr copy assignment did not add to new pointer's count (inst2 count -> {})", i2c::detail::get_count(&inst2));
    }

    // Move assignment: c should take over inst1, a should be reset to null, inst2's count should stay the same overall.
    i2c::detail::count_ptr<Il2CppObject> c(&inst1);
    c = std::move(a);
    if (c.get() != &inst2) {
        LOG_FAIL("count_ptr move assignment did not transfer held pointer");
    }
    if (a.get() != nullptr) {
        LOG_FAIL("count_ptr move assignment did not null out the moved-from instance");
    }
    if (i2c::detail::get_count(&inst1) != 0) {
        LOG_FAIL("count_ptr move assignment did not release the overwritten pointer's count (inst1 count -> {})", i2c::detail::get_count(&inst1));
    }
    if (i2c::detail::get_count(&inst2) != 2) {
        LOG_FAIL("count_ptr move assignment changed count unexpectedly (inst2 count -> {})", i2c::detail::get_count(&inst2));
    }

    LOG_OK("count_ptr copy/move assignment tests complete");
}

TEST(safeptr_copy_assignment) {
    LOG_OK("Starting safe_ptr copy assignment tests");

    Il2CppObject inst{};
    safe_ptr<Il2CppObject*> a(&inst);
    safe_ptr<Il2CppObject*> b;

    // This exercises safe_ptr::operator=(safe_ptr const&), which relies on count_ptr's copy assignment operator.
    b = a;
    if (b.ptr() != a.ptr()) {
        LOG_FAIL("safe_ptr copy assignment did not share the same held pointer");
    }
    LOG_OK("safe_ptr copy assignment tests complete");
}

TEST(safeptr_casts) {
    LOG_OK("Starting safe_ptr cast tests (reference types only)");

    // Reference-type cast example
    safe_ptr<Il2CppObject*> a(i2c::new_ctor<Il2CppObject*>());
    auto maybe_ref = a.try_cast<Il2CppReflectionType*>();
    LOG_OK("safe_ptr<Il2CppObject>.try_cast<Il2CppReflectionType> -> {}", static_cast<bool>(maybe_ref));
    if (maybe_ref) {
        LOG_FAIL("Cast succeeded (unexpected)");
    }
}

namespace UnityEngine {
    // wrapper_type concept requires a complete type
    class RectTransform {};
}

DEFINE_IL2CPP_CLASS(UnityEngine::RectTransform*, "UnityEngine", "RectTransform");
MARK_REF_T(UnityEngine::RectTransform*);

TEST(safeptr_unity_gameobject) {
    LOG_OK("Starting safe_ptr Unity tests");

    // Invoke Create to get a GameObject instance
    Il2CppObject* game_obj = nullptr;
    try {
        game_obj = i2c::new_ctor({"UnityEngine", "GameObject"}, StringW("TestObject"));
    } catch (std::exception& e) {
        LOG_FAIL("Exception thrown while creating GameObject: {}", e.what());
        return;
    }
    LOG_OK("Created GameObject -> {}", fmt::ptr(game_obj));

    // Test safe_ptr with GameObject reference type
    safe_ptr<Il2CppObject*, true> go_ptr(game_obj);
    LOG_OK("GameObject safe_ptr -> {}", static_cast<bool>(go_ptr));

    try {
        auto add_rect_transform_method = i2c::find_method(game_obj, {"AddComponent", {i2c::class_of<UnityEngine::RectTransform*>()}, {}});
        add_rect_transform_method = i2c::make_generic(add_rect_transform_method, {i2c::class_of<UnityEngine::RectTransform*>()});
        i2c::run_method(game_obj, add_rect_transform_method);
        LOG_OK("Added RectTransform component to GameObject");
    } catch (std::exception& e) {
        LOG_FAIL("Exception thrown while adding RectTransform: {}", e.what());
    }

    Il2CppObject* transform = nullptr;
    try {
        transform = i2c::run_method<Il2CppObject*>(go_ptr.ptr(), "get_transform");
    } catch (std::exception& e) {
        LOG_FAIL("Exception thrown in get_transform: {}", e.what());
        return;
    }
    LOG_OK("GameObject.get_transform -> {}", fmt::ptr(transform));

    safe_ptr<Il2CppObject*, true> transform_ptr(transform);
    LOG_OK("Transform safe_ptr -> {}", static_cast<bool>(transform_ptr));

    auto rect_cast = transform_ptr.try_cast<UnityEngine::RectTransform*>();
    LOG_OK("Transform safe_ptr.try_cast<RectTransform> -> {}", static_cast<bool>(rect_cast));

    try {
        i2c::run_method({"UnityEngine", "Object"}, "DestroyImmediate", game_obj);
        LOG_OK("Destroyed GameObject");
    } catch (std::exception& e) {
        LOG_FAIL("Exception thrown while destroying GameObject: {}", e.what());
    }

    LOG_OK(
        "safe_ptr alive checks: go_ptr -> {} | transform_ptr -> {} | rect_cast -> {}",
        static_cast<bool>(go_ptr),
        static_cast<bool>(transform_ptr),
        static_cast<bool>(rect_cast)
    );

    LOG_OK("safe_ptr Unity tests complete");
}
