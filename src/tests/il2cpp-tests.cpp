#include "../../shared/utils/byref.hpp"
#include "../../shared/utils/il2cpp-utils.hpp"
#include "../../shared/utils/typedefs.h"
#include "scotland2/shared/loader.hpp"
#include "utils/il2cpp-functions.hpp"
#include "utils/il2cpp-type-check.hpp"
#include "utils/il2cpp-utils-classes.hpp"
#include "utils/il2cpp-utils-methods.hpp"
#include "utils/logging.hpp"

#include <span>
#include <string>
#include <string_view>
#include <vector>

static modloader::ModInfo modInfo = { MOD_ID, VERSION, 0 };  // Stores the ID and version of our mod, and is sent to the modloader upon startup

// Logging helpers: prefix good logs with OK and bad logs with FAIL
// Use variadic macros so calls stay concise and avoid template instantiation here.
#define LOG_OK(fmt, ...) il2cpp_utils::Logger.info("OK: " fmt, ##__VA_ARGS__)
#define LOG_FAIL(fmt, ...) il2cpp_utils::Logger.warn("FAIL: " fmt, ##__VA_ARGS__)

void setup(CModInfo* info) {
    info->id = MOD_ID;
    info->version = VERSION;

    LOG_OK("Completed setup!");
}

static void test_basic_creation_and_methods() {
    using namespace il2cpp_utils;
    LOG_OK("[il2cpp-tests] Starting basic creation & method tests");

    // Create a System.Object instance and call ToString()
    if (auto obj = New<Il2CppObject*>("System", "Object"); obj) {
        LOG_OK("[il2cpp-tests] Created System.Object: {}", fmt::ptr(*obj));
        auto* objKlass = GetClassFromName("System", "Object");
        if (!objKlass) {
            LOG_FAIL("[il2cpp-tests] Could not find System.Object class for ToString test");
        } else {
            auto toStringMethod = FindMethod(objKlass, "ToString", std::array<const Il2CppType*, 0>{});
            if (toStringMethod) {
                if (auto str = RunMethodOpt<Il2CppString*>((Il2CppObject*)*obj, toStringMethod)) {
                    LOG_OK("[il2cpp-tests] Object.ToString() returned object ptr: {}", fmt::ptr(*str));
                } else {
                    LOG_FAIL("[il2cpp-tests] Object.ToString() call failed or returned null");
                }
            } else {
                LOG_FAIL("[il2cpp-tests] Could not find ToString method on System.Object");
            }
        }
    } else {
        LOG_FAIL("[il2cpp-tests] Could not construct System.Object (class missing?)");
    }

    // String.Concat test
    auto s1 = newcsstr("Hello ");
    auto s2 = newcsstr("World");

    auto* klass = GetClassFromName("System", "String");
    if (!klass) {
        LOG_FAIL("[il2cpp-tests] Could not find System.String class for String.Concat test");
        return;
    }

    // Find MethodInfo for String.Concat(string, string)

    auto method = FindMethod(klass, "Concat", std::array<const Il2CppType*, 2>{ ExtractIndependentType<Il2CppString*>(), ExtractIndependentType<Il2CppString*>() });
    if (auto concat = RunMethodOpt<Il2CppString*, true>(nullptr, method, s1, s2)) {
        LOG_OK("[il2cpp-tests] String.Concat succeeded, result ptr: {}", fmt::ptr(*concat));
    } else {
        LOG_FAIL("[il2cpp-tests] String.Concat failed");
    }
}

static void test_safeptr_and_countpointer() {
    using namespace il2cpp_utils;
    LOG_OK("[il2cpp-tests] Starting SafePtr / CountPointer tests (reference types only)");

    // Use a reference-type instance (Il2CppObject) instead of primitives
    Il2CppObject inst{};
    LOG_OK("[il2cpp-tests] Created stack Il2CppObject inst at {}", fmt::ptr(&inst));

    // Default constructed SafePtr
    SafePtr<Il2CppObject> a;
    LOG_OK("[il2cpp-tests] Created default SafePtr<Il2CppObject> a (isHandleValid: {})", a.isHandleValid());

    {
        SafePtr<Il2CppObject> b(&inst);
        LOG_OK("[il2cpp-tests] Created SafePtr<Il2CppObject> b(&inst). Counter for &inst -> {}", Counter::get(&inst));

        {
            CountPointer<Il2CppObject> cptr(&inst);
            LOG_OK("[il2cpp-tests] Created CountPointer<Il2CppObject> cptr(&inst). cptr.count() -> {} | Counter.get -> {}", cptr.count(), Counter::get(&inst));
        }
        LOG_OK("[il2cpp-tests] After destroying CountPointer, Counter.get(&inst) -> {}", Counter::get(&inst));

        // Log pointer addresses rather than treating object as a value
        LOG_OK("[il2cpp-tests] b.ptr() -> {}", fmt::ptr(b.ptr()));

        // Create a temporary copy
        {
            SafePtr<Il2CppObject> c(b);
            LOG_OK("[il2cpp-tests] Copied SafePtr c(b). Counter.get(&inst) -> {} | c.ptr() -> {}", Counter::get(&inst), fmt::ptr(c.ptr()));
        }
        LOG_OK("[il2cpp-tests] After destroying copy, Counter.get(&inst) -> {}", Counter::get(&inst));

        // Pass by reference-like usage
        auto testRef = [&](SafePtr<Il2CppObject>& ref) { LOG_OK("[il2cpp-tests] In testRef, received ref.ptr() -> {}", fmt::ptr(ref.ptr())); };
        testRef(b);
        LOG_OK("[il2cpp-tests] After testRef, Counter.get(&inst) -> {}", Counter::get(&inst));

        // Pass by value (copy)
        auto testCopy = [&](SafePtr<Il2CppObject> copy) { LOG_OK("[il2cpp-tests] In testCopy, received copy.ptr() -> {}", fmt::ptr(copy.ptr())); };
        testCopy(b);
        LOG_OK("[il2cpp-tests] After testCopy, Counter.get(&inst) -> {}", Counter::get(&inst));

        // Literal pointer cast (discouraged) — pass raw pointer
        auto testLiteral = [&](Il2CppObject* p) { LOG_OK("[il2cpp-tests] In testLiteral, received raw ptr -> {}", fmt::ptr(p)); };
        testLiteral((Il2CppObject*)b.ptr());
        LOG_OK("[il2cpp-tests] After testLiteral, Counter.get(&inst) -> {}", Counter::get(&inst));

        // Final pointer log
        LOG_OK("[il2cpp-tests] Final b.ptr() -> {} | Counter.get(&inst) -> {}", fmt::ptr(b.ptr()), Counter::get(&inst));
    }

    LOG_OK("[il2cpp-tests] After leaving scope, Counter.get(&inst) -> {}", Counter::get(&inst));
}

static void test_safeptr_casts() {
    using namespace il2cpp_utils;
    auto const& logger = il2cpp_utils::Logger;
    logger.info("[il2cpp-tests] Starting SafePtr cast tests (reference types only)");

    // Reference-type cast example
    {
        Il2CppObject inst{};
        SafePtr<Il2CppObject> a(&inst);
        auto maybeRef = a.try_cast<Il2CppReflectionType>();
        logger.info("[il2cpp-tests] SafePtr<Il2CppObject>.try_cast<Il2CppReflectionType> -> {}", maybeRef.has_value());
        if (maybeRef) {
            logger.info("[il2cpp-tests] Cast succeeded (unexpected)");
        }
    }
}

static void test_stringw() {
    using namespace il2cpp_utils;
    auto const& logger = il2cpp_utils::Logger;
    logger.info("[il2cpp-tests] Starting StringW tests");

    StringW s1("hello");
    StringW s2(" world");
    auto s3 = s1 + s2;
    logger.info("[il2cpp-tests] s1 -> {} | s2 -> {} | s3 -> {}", static_cast<std::string>(s1), static_cast<std::string>(s2), static_cast<std::string>(s3));

    logger.info("[il2cpp-tests] s3.starts_with('he') -> {} | s3.ends_with('ld') -> {}", s3.starts_with("he"), s3.ends_with("ld"));

    // concatenation with std::string
    StringW s4 = s3 + std::string("!");
    logger.info("[il2cpp-tests] s4 -> {}", static_cast<std::string>(s4));
}

static void test_listw() {
    using namespace il2cpp_utils;
    auto const& logger = il2cpp_utils::Logger;
    logger.info("[il2cpp-tests] Starting ListW tests");

    // Create a managed List<int> and wrap it
    ListW<int> arr(*il2cpp_utils::New<List<int>*>(classof(List<int>*)));
    logger.info("[il2cpp-tests] Created ListW<int> arr (ptr {})", fmt::ptr(arr.getPtr()));

    arr.push_back(7);
    arr.push_back(13);
    logger.info("[il2cpp-tests] After push_back: size -> {} | [0] -> {} | [1] -> {}", arr.size(), arr[0], arr[1]);

    arr.insert_at(1, 99);
    logger.info("[il2cpp-tests] After insert_at(1,99): size -> {} | [1] -> {}", arr.size(), arr[1]);

    auto found = arr.find([](int const& v) { return v == 99; });
    logger.info("[il2cpp-tests] find(99) -> {}", found.has_value());

    auto asArray = arr.to_array();
    logger.info("[il2cpp-tests] to_array size -> {} | contents -> {}", asArray->get_Length(), fmt::join(asArray.begin(), asArray.end(), ","));

    arr.clear();
    logger.info("[il2cpp-tests] After clear: size -> {}", arr.size());
}

static void test_arrayw() {
    using namespace il2cpp_utils;
    LOG_OK("[il2cpp-tests] Starting ArrayW tests");

    ArrayW<int> a = ArrayW<int>({ 1, 2, 3 });
    LOG_OK("[il2cpp-tests] Created ArrayW<int> a size -> {} | first -> {} | last -> {}", a->get_Length(), a->First(), a->Last());

    bool contains2 = a->Contains(2);
    LOG_OK("[il2cpp-tests] a.Contains(2) -> {}", contains2);

    auto vec = std::vector<int>(a.begin(), a.end());
    LOG_OK("[il2cpp-tests] iterated values -> {}", fmt::join(vec, ","));

    ArrayW<int> b(5);
    LOG_OK("[il2cpp-tests] Allocated ArrayW<int> b length -> {}", b->get_Length());
}

// ByRef helper tests: exercise ExtractIndependentType and ByRef construction
static void test_byref_helpers() {
    using namespace il2cpp_utils;
    LOG_OK("[il2cpp-tests] Starting ByRef helper tests");

    try {
        // Ensure ExtractIndependentType for ByRef<int> equals that for int&
        auto t_byref = ExtractIndependentType<ByRef<int>>();
        auto t_ref = ExtractIndependentType<int&>();
        if (t_byref && t_ref) {
            bool equal = (t_byref == t_ref);
            LOG_OK("[il2cpp-tests] ExtractIndependentType<ByRef<int>> == ExtractIndependentType<int&> -> {}", equal);
        } else {
            LOG_FAIL("[il2cpp-tests] ExtractIndependentType returned null for ByRef or int&");
        }

        // Construct a ByRef instance from a local int and validate that it can be created
        int local = 123;
        const ByRef<int> br(local);
        // Also test the byref(x) helper if available
        auto br2 = byref(local);
        (void)br;
        (void)br2;
        LOG_OK("[il2cpp-tests] Constructed ByRef<int> and byref(local) successfully");

        // Confirm that ExtractIndependentType<int>() is non-null as a sanity check
        if (auto t_int = ExtractIndependentType<int>()) {
            LOG_OK("[il2cpp-tests] ExtractIndependentType<int> present {}", fmt::ptr(t_int));
        } else {
            LOG_FAIL("[il2cpp-tests] ExtractIndependentType<int> returned null (unexpected)");
        }
    } catch (const std::exception& e) {
        LOG_FAIL("[il2cpp-tests] Exception during ByRef tests: {}", e.what());
    } catch (...) {
        LOG_FAIL("[il2cpp-tests] Unknown exception during ByRef tests");
    }
}

// Test invoking a C# method that will throw and using RunMethodRethrow to observe it
static void test_runmethodrethrow_on_throwing_method() {
    using namespace il2cpp_utils;
    auto const& logger = il2cpp_utils::Logger;
    logger.info("[il2cpp-tests] Starting RunMethodRethrow tests (invoke C# method that throws)");

    try {
        // System.Int32.Parse will throw FormatException for non-numeric input
        auto method = FindMethod(GetClassFromName("System", "Int32"), "Parse", std::array<const Il2CppType*, 1>{ ExtractIndependentType<Il2CppString*>() });
        auto val = RunMethodRethrow<int>(nullptr, method, newcsstr("notanint"));
        logger.warn("[il2cpp-tests] Unexpected success from Int32.Parse, value: {}", val);
    } catch (const il2cpp_utils::RunMethodException& e) {
        logger.info("[il2cpp-tests] Caught RunMethodException from Int32.Parse: {}", e.what());
    } catch (const std::exception& e) {
        logger.warn("[il2cpp-tests] Unexpected std::exception from Int32.Parse test: {}", e.what());
    } catch (...) {
        logger.warn("[il2cpp-tests] Unknown exception from Int32.Parse test");
    }
}

static void test_delegates() {
    using namespace il2cpp_utils;

}

static void test_arrays_and_generics() {
    using namespace il2cpp_utils;
    auto const& logger = il2cpp_utils::Logger;
    logger.info("[il2cpp-tests] Starting array & generics checks");

    // Try creating an array of System.Object
    if (auto objKlass = GetClassFromName("System", "Object")) {
        auto* arr = il2cpp_functions::array_new(objKlass, 3);
        LOG_OK("[il2cpp-tests] Created System.Object[]: {}", fmt::ptr(arr));
    } else {
        LOG_FAIL("[il2cpp-tests] System.Object klass not found, skipping array creation");
    }

    // Check for generic List<T> class presence (without instantiation)
    if (auto listKlass = GetClassFromName("System.Collections.Generic", "List`1")) {
        LOG_OK("[il2cpp-tests] Found generic type List`1: {}", fmt::ptr(listKlass));
    } else {
        LOG_FAIL("[il2cpp-tests] Generic List`1 not found in assemblies");
    }

    // call a generic method - List<int>.Add
    if (auto listKlass = GetClassFromName("System.Collections.Generic", "List`1")) {
        // Make List<int>
        auto* intKlass = GetClassFromName("System", "Int32");
        if (!intKlass) {
            LOG_FAIL("[il2cpp-tests] System.Int32 klass not found, skipping List<int> test");
            return;
        }
        const Il2CppClass* const typeArgs[] = { intKlass };
        auto* genericListKlass = il2cpp_utils::MakeGeneric(listKlass, typeArgs);
        if (!genericListKlass) {
            LOG_FAIL("[il2cpp-tests] Could not make List<int> generic class");
            return;
        }

        // Create instance of List<int>
        auto listInstance = New<Il2CppObject*>(genericListKlass);
        if (!listInstance) {
            LOG_FAIL("[il2cpp-tests] Could not create List<int> instance");
            return;
        }
        // Find Add method
        auto addMethod = FindMethod(genericListKlass, "Add", std::array<const Il2CppType*, 1>{ ExtractIndependentType<int>() });
        if (!addMethod) {
            LOG_FAIL("[il2cpp-tests] Could not find List<int>.Add method");
            return;
        }
        // Call Add(42)
        if (auto result = RunMethodOpt<void, true>(listInstance.value(), addMethod, 42)) {
            LOG_OK("[il2cpp-tests] Successfully called List<int>.Add(42)");
        } else {
            LOG_FAIL("[il2cpp-tests] Failed to call List<int>.Add(42)");
        }
        // Read backing field (int) usually named "_size" on many List<T> implementations
        if (auto sizeOpt = GetFieldValue<int>(listInstance.value(), "_size")) {
            LOG_OK("[il2cpp-tests] List<int> _size before set -> {}", *sizeOpt);
        } else {
            LOG_FAIL("[il2cpp-tests] Could not read List<int>._size (field may be named differently)");
        }

        // Try setting the _size field to 1 and read back
        if (SetFieldValue(listInstance.value(), "_size", 1)) {
            LOG_OK("[il2cpp-tests] Set List<int>._size = 1 (success)");
            if (auto newSize = GetFieldValue<int>(listInstance.value(), "_size")) {
                LOG_OK("[il2cpp-tests] List<int> _size after set -> {}", *newSize);
            }
        } else {
            LOG_FAIL("[il2cpp-tests] Failed to set List<int>._size (field may be readonly or not present)");
        }
    } else {
        LOG_FAIL("[il2cpp-tests] Generic List`1 not found, skipping List<int>.Add test");
    }
}

static void test_interfaces_and_fields() {
    using namespace il2cpp_utils;
    auto const& logger = il2cpp_utils::Logger;
    logger.info("[il2cpp-tests] Starting interface & field tests");

    // Interface check: IComparable
    if (auto ic = GetClassFromName("System", "IComparable")) {
        LOG_OK("[il2cpp-tests] System.IComparable present, IsInterface: {}", IsInterface(ic));
    } else {
        LOG_FAIL("[il2cpp-tests] System.IComparable not found");
    }

    // Try a basic property: get_Length on a string instance
    auto str = newcsstr("test-length");
    {
        auto* stringKlass = GetClassFromName("System", "String");
        if (!stringKlass) {
            LOG_FAIL("[il2cpp-tests] Could not find System.String class for get_Length test");
        } else {
            auto getLenMethod = FindMethod(stringKlass, "get_Length", std::array<const Il2CppType*, 0>{});
            if (getLenMethod) {
                if (auto len = RunMethodOpt<int>(str, getLenMethod)) {
                    LOG_OK("[il2cpp-tests] Il2CppString.get_Length -> {}", *len);
                } else {
                    LOG_FAIL("[il2cpp-tests] Could not call get_Length on string (method failed)");
                }
            } else {
                LOG_FAIL("[il2cpp-tests] Could not find get_Length on System.String");
            }
        }
    }
}

static void test_get_set_field_on_non_generic() {
    using namespace il2cpp_utils;
    auto const& logger = il2cpp_utils::Logger;
    logger.info("[il2cpp-tests] Starting non-generic get/set field test");

    // We'll try System.Text.StringBuilder as a common non-generic runtime type with int fields
    auto* klass = GetClassFromName("System.Text", "StringBuilder");
    if (!klass) {
        LOG_FAIL("[il2cpp-tests] System.Text.StringBuilder not found, skipping non-generic field test");
        return;
    }

    auto instanceOpt = New<Il2CppObject*>(klass);
    if (!instanceOpt) {
        LOG_FAIL("[il2cpp-tests] Could not construct StringBuilder instance");
        return;
    }
    Il2CppObject* instance = instanceOpt.value();

    // Candidate field names (common variants across runtimes)
    const char* fieldCandidates[] = { "m_MaxCapacity", "m_ChunkLength", "m_capacity", "m_ChunkOffset" };

    for (auto const& fname : fieldCandidates) {
        if (auto val = GetFieldValue<int>(instance, fname)) {
            LOG_OK("[il2cpp-tests] Read field '{}' -> {}", fname, *val);
            // Try setting it to val+1 (defensive)
            if (SetFieldValue(instance, fname, *val + 1)) {
                LOG_OK("[il2cpp-tests] Successfully set field '{}' -> {}", fname, *val + 1);
                if (auto newVal = GetFieldValue<int>(instance, fname)) {
                    LOG_OK("[il2cpp-tests] Read back field '{}' -> {}", fname, *newVal);
                }
            } else {
                LOG_FAIL("[il2cpp-tests] Failed to set field '{}' (may be readonly)", fname);
            }
            return;  // done with first successful candidate
        }
    }

    LOG_FAIL("[il2cpp-tests] No suitable int field found on StringBuilder from candidates");
}

// Property getter/setter tests
static void test_property_get_set() {
    using namespace il2cpp_utils;
    auto const& logger = il2cpp_utils::Logger;
    logger.info("[il2cpp-tests] Starting property get/set tests");

    // Test System.Text.StringBuilder.Length getter/setter
    auto* sbClass = GetClassFromName("System.Text", "StringBuilder");
    if (!sbClass) {
        LOG_FAIL("[il2cpp-tests] System.Text.StringBuilder not found, skipping property tests");
    } else {
        if (auto sbOpt = New<Il2CppObject*>(sbClass)) {
            Il2CppObject* sb = sbOpt.value();
            // Use GetPropertyValue / SetPropertyValue helpers
            {
                auto lenRes = GetPropertyValue<int>(sb, "Length");
                if (lenRes.has_result()) {
                    LOG_OK("[il2cpp-tests] StringBuilder.Length initial -> {}", lenRes.get_result());
                } else {
                    LOG_FAIL("[il2cpp-tests] Failed to get StringBuilder.Length");
                }
            }

            {
                auto setRes = SetPropertyValue(sb, "Length", 5);
                if (setRes.has_result()) {
                    LOG_OK("[il2cpp-tests] Called set_Length(5) on StringBuilder");
                    auto newLen = GetPropertyValue<int>(sb, "Length");
                    if (newLen.has_result()) {
                        LOG_OK("[il2cpp-tests] StringBuilder.Length after set -> {}", newLen.get_result());
                    }
                } else {
                    LOG_FAIL("[il2cpp-tests] Failed to set StringBuilder.Length (setter missing or failed)");
                }
            }
        } else {
            LOG_FAIL("[il2cpp-tests] Could not create StringBuilder instance for property tests");
        }
    }

    // Test List<int>.Count getter after Add
    if (auto listKlass = GetClassFromName("System.Collections.Generic", "List`1")) {
        auto* intKlass = GetClassFromName("System", "Int32");
        if (!intKlass) {
            LOG_FAIL("[il2cpp-tests] System.Int32 not found, skipping List<int>.Count test");
        } else {
            const Il2CppClass* const typeArgs[] = { intKlass };
            auto* genericListKlass = il2cpp_utils::MakeGeneric(listKlass, typeArgs);
            if (!genericListKlass) {
                LOG_FAIL("[il2cpp-tests] Could not make List<int> generic class for property test");
            } else if (auto listInstance = New<Il2CppObject*>(genericListKlass)) {
                // Find Add and get_Count
                auto addMethod = FindMethod(genericListKlass, "Add", std::array<const Il2CppType*, 1>{ ExtractIndependentType<int>() });
                if (!addMethod) {
                    LOG_FAIL("[il2cpp-tests] List<int>.Add not found for property test");
                } else {
                    if (auto r = RunMethodOpt<void, true>(listInstance.value(), addMethod, 10)) {
                        LOG_OK("[il2cpp-tests] Called List<int>.Add(10)");
                        auto cnt = GetPropertyValue<int>(listInstance.value(), "Count");
                        if (cnt.has_result()) {
                            LOG_OK("[il2cpp-tests] List<int>.Count -> {}", cnt.get_result());
                        } else {
                            LOG_FAIL("[il2cpp-tests] Failed to get List<int>.Count");
                        }
                    } else {
                        LOG_FAIL("[il2cpp-tests] Failed to call List<int>.Add for property test");
                    }
                }
            } else {
                LOG_FAIL("[il2cpp-tests] Could not create List<int> instance for property test");
            }
        }
    } else {
        LOG_FAIL("[il2cpp-tests] List`1 not found, skipping List<int>.Count test");
    }
}

static void type_check_tests() {
    using namespace il2cpp_utils;
    auto const& logger = il2cpp_utils::Logger;
    logger.info("[il2cpp-tests] Starting type check tests");

    auto str = StringW("foo");
    auto strType = il2cpp_utils::ExtractType<StringW const&>(str);
    il2cpp_type_check::il2cpp_arg_type<StringW&>::get(str);

    // now use literal StringW
    il2cpp_utils::il2cpp_type_check::il2cpp_arg_class<StringW>::get(str);
    il2cpp_utils::il2cpp_type_check::il2cpp_no_arg_class<StringW>::get();

    logger.info("[il2cpp-tests] Extracted type for StringW 'foo' -> {}", fmt::ptr(strType));
    logger.info("[il2cpp-tests] Type name: {:x}", (int)strType->type);
    if (strType->type == IL2CPP_TYPE_STRING) {
        LOG_OK("[il2cpp-tests] Extracted type is IL2CPP_TYPE_STRING as expected");
    } else {
        LOG_FAIL("[il2cpp-tests] Extracted type is not IL2CPP_TYPE_STRING (unexpected)");
    }

    auto ptrType = (Il2CppString*)str;
    auto ptrTypeExtracted = il2cpp_utils::ExtractType<Il2CppString*&>(ptrType);

    il2cpp_type_check::il2cpp_arg_type<Il2CppString*>::get(ptrType);
    il2cpp_utils::il2cpp_type_check::il2cpp_arg_class<Il2CppString*>::get(str);
    il2cpp_utils::il2cpp_type_check::il2cpp_no_arg_class<Il2CppString*>::get();

    logger.info("[il2cpp-tests] Extracted type for Il2CppString* ptr -> {}", fmt::ptr(ptrTypeExtracted));
    logger.info("[il2cpp-tests] Type name: {:x}", (int)ptrTypeExtracted->type);
    if (ptrTypeExtracted->type == IL2CPP_TYPE_STRING) {
        LOG_OK("[il2cpp-tests] Extracted type is IL2CPP_TYPE_STRING as expected for Il2CppString*");
    } else {
        LOG_FAIL("[il2cpp-tests] Extracted type is not IL2CPP_TYPE_STRING (unexpected) for Il2CppString*");
    }

    // Example: check that System.String is assignable from System.Object
    auto* stringKlass = GetClassFromName("System", "String");
    auto* objectKlass = GetClassFromName("System", "Object");
    if (stringKlass && objectKlass) {
        bool assignable = il2cpp_utils::IsConvertibleFrom(&objectKlass->byval_arg, &stringKlass->byval_arg);
        LOG_OK("[il2cpp-tests] System.String is assignable from System.Object -> {}", assignable);
    } else {
        LOG_FAIL("[il2cpp-tests] Could not find System.String or System.Object for type check test");
    }
}

extern "C" void load() {
    LOG_OK("Loaded mod: {} v{}", modInfo.id, modInfo.version);
    LOG_OK("Running il2cpp_init", modInfo.id, modInfo.version);
    il2cpp_functions::Init();
    LOG_OK("il2cpp_init complete!");
    // Run builtin il2cpp tests that exercise common utilities.
    // Execute tests (they are defensive and will log failures rather than crash)

    type_check_tests();
    test_basic_creation_and_methods();
    test_arrays_and_generics();
    test_interfaces_and_fields();
    test_get_set_field_on_non_generic();
    test_safeptr_and_countpointer();
    test_safeptr_casts();
    test_stringw();
    test_listw();
    test_arrayw();
    test_byref_helpers();
    test_property_get_set();
    test_runmethodrethrow_on_throwing_method();
    test_delegates();
}

extern "C" void late_load() {}