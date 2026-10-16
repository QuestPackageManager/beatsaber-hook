#include "members.hpp"
#include "safeptr.hpp"
#include "valuew.hpp"
#include "tests.hpp"

#ifdef UNITY_6
TEST(task_completion_source_vector_result) {
    using Vector3 = ValueW<3 * sizeof(float), "UnityEngine", "Vector3">;
    auto* definition = i2c::find_class({"System.Threading.Tasks", "TaskCompletionSource`1"});
    auto* source_class = i2c::make_generic(definition, {i2c::class_of<Vector3>()});
    safe_ptr<Il2CppObject*, false> source(i2c::new_ctor(source_class));
    safe_ptr<Il2CppObject*, false> task(i2c::run_method<Il2CppObject*>(source.ptr(), "get_Task"));
    if (i2c::run_method<bool>(task.ptr(), "get_IsCompleted")) {
        LOG_FAIL("A new TaskCompletionSource<Vector3> task is already completed");
        return;
    }

    // A native position calculation publishes its result to managed callers.
    // Unity 6 can supply this value-type generic through a shared invoker only.
    auto position = i2c::new_ctor<Vector3>(1.25f, -2.5f, 3.75f);
    if (!i2c::run_method<bool>(source.ptr(), "TrySetResult", position) ||
        !i2c::run_method<bool>(task.ptr(), "get_IsCompleted")) {
        LOG_FAIL("TaskCompletionSource<Vector3> did not complete its task");
        return;
    }
    auto returned = i2c::run_method<Vector3>(task.ptr(), "get_Result");
    if (i2c::get_field<float>(returned, "x") != 1.25f || i2c::get_field<float>(returned, "y") != -2.5f ||
        i2c::get_field<float>(returned, "z") != 3.75f) {
        LOG_FAIL("Task<Vector3> did not preserve the calculated position");
        return;
    }

    // Real managed failure paths: TrySetResult returns false after completion,
    // whereas SetResult throws. Neither is allowed to overwrite the first value.
    auto replacement = i2c::new_ctor<Vector3>(4.0f, 5.0f, 6.0f);
    auto repeated = i2c::run_method<i2c::result<bool>>(source.ptr(), "TrySetResult", replacement);
    if (!repeated || repeated.value()) {
        LOG_FAIL("Completing TaskCompletionSource<Vector3> twice must return false");
        return;
    }
    auto rejected = i2c::run_method<i2c::result<>>(source.ptr(), "SetResult", replacement);
    if (rejected || rejected.error().find("InvalidOperationException") == std::string::npos) {
        LOG_FAIL("Repeated SetResult did not report the managed InvalidOperationException");
        return;
    }

    auto unchanged = i2c::run_method<Vector3>(task.ptr(), "get_Result");
    if (unchanged.instance != position.instance) {
        LOG_FAIL("Repeated completion overwrote the original Vector3 result");
        return;
    }
    LOG_OK("TaskCompletionSource<Vector3> publishes a position and rejects duplicate completion without changing the result");
}
#endif
