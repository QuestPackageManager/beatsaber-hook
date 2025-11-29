#include <unistd.h>

#include "../../shared/utils/capstone-utils.hpp"
#include "../../shared/utils/hooking.hpp"
#include "../../shared/utils/il2cpp-functions.hpp"
#include "../../shared/utils/logging.hpp"
#include "capstone/shared/capstone/capstone.h"

#define API_INIT_MONO(rt, name, ...) rt(*il2cpp_functions::mono_##name) __VA_ARGS__
#define API_INIT(rt, name, ...) rt(*il2cpp_functions::il2cpp_##name) __VA_ARGS__
// All the fields...
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
API_INIT(int, init, (const char* domain_name));
API_INIT(int, init_utf16, (const Il2CppChar* domain_name));
#else
API_INIT(void, init, (const char* domain_name));
API_INIT(void, init_utf16, (const Il2CppChar* domain_name));
#endif
API_INIT(void, shutdown, ());
API_INIT(void, set_config_dir, (const char* config_path));
API_INIT(void, set_data_dir, (const char* data_path));
API_INIT(void, set_temp_dir, (const char* temp_path));
API_INIT(void, set_commandline_arguments, (int argc, const char* const argv[], const char* basedir));
API_INIT(void, set_commandline_arguments_utf16, (int argc, const Il2CppChar* const argv[], const char* basedir));
API_INIT(void, set_config_utf16, (const Il2CppChar* executablePath));
API_INIT(void, set_config, (const char* executablePath));
API_INIT(void, set_memory_callbacks, (Il2CppMemoryCallbacks * callbacks));
API_INIT(const Il2CppImage*, get_corlib, ());
API_INIT(void, add_internal_call, (const char* name, Il2CppMethodPointer method));
API_INIT(Il2CppMethodPointer, resolve_icall, (const char* name));
API_INIT(void*, alloc, (size_t size));
API_INIT(void, free, (void* ptr));
API_INIT(Il2CppClass*, array_class_get, (Il2CppClass * element_class, uint32_t rank));
API_INIT(uint32_t, array_length, (Il2CppArray * array));
API_INIT(uint32_t, array_get_byte_length, (Il2CppArray * array));
API_INIT(Il2CppArray*, array_new, (Il2CppClass * elementTypeInfo, il2cpp_array_size_t length));
API_INIT(Il2CppArray*, array_new_specific, (Il2CppClass * arrayTypeInfo, il2cpp_array_size_t length));
API_INIT(Il2CppArray*, array_new_full, (Il2CppClass * array_class, il2cpp_array_size_t* lengths, il2cpp_array_size_t* lower_bounds));
API_INIT(Il2CppClass*, bounded_array_class_get, (Il2CppClass * element_class, uint32_t rank, bool bounded));
API_INIT(int, array_element_size, (const Il2CppClass* array_class));
API_INIT(const Il2CppImage*, assembly_get_image, (const Il2CppAssembly* assembly));
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
API_INIT(void, class_for_each, (void (*klassReportFunc)(Il2CppClass* klass, void* userData), void* userData));
#endif
API_INIT(const Il2CppType*, class_enum_basetype, (Il2CppClass * klass));
API_INIT(bool, class_is_generic, (const Il2CppClass* klass));
API_INIT(bool, class_is_inflated, (const Il2CppClass* klass));
API_INIT(bool, class_is_assignable_from, (Il2CppClass * klass, Il2CppClass* oklass));
API_INIT(bool, class_is_subclass_of, (Il2CppClass * klass, Il2CppClass* klassc, bool check_interfaces));
API_INIT(bool, class_has_parent, (Il2CppClass * klass, Il2CppClass* klassc));
API_INIT(Il2CppClass*, class_from_il2cpp_type, (const Il2CppType* type));
API_INIT(Il2CppClass*, class_from_name, (const Il2CppImage* image, const char* namespaze, const char* name));
API_INIT(Il2CppClass*, class_from_system_type, (Il2CppReflectionType * type));
API_INIT(Il2CppClass*, class_get_element_class, (Il2CppClass * klass));
API_INIT(const EventInfo*, class_get_events, (Il2CppClass * klass, void** iter));
API_INIT(FieldInfo*, class_get_fields, (Il2CppClass * klass, void** iter));
API_INIT(Il2CppClass*, class_get_nested_types, (Il2CppClass * klass, void** iter));
API_INIT(Il2CppClass*, class_get_interfaces, (Il2CppClass * klass, void** iter));
API_INIT(const PropertyInfo*, class_get_properties, (Il2CppClass * klass, void** iter));
API_INIT(const PropertyInfo*, class_get_property_from_name, (Il2CppClass * klass, const char* name));
API_INIT(FieldInfo*, class_get_field_from_name, (Il2CppClass * klass, const char* name));
API_INIT(const MethodInfo*, class_get_methods, (Il2CppClass * klass, void** iter));
API_INIT(const MethodInfo*, class_get_method_from_name, (const Il2CppClass* klass, const char* name, int argsCount));
API_INIT(const char*, class_get_name, (const Il2CppClass* klass));
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
API_INIT(void, type_get_name_chunked, (const Il2CppType* type, void (*chunkReportFunc)(void* data, void* userData), void* userData));
#endif
API_INIT(const char*, class_get_namespace, (const Il2CppClass* klass));
API_INIT(Il2CppClass*, class_get_parent, (Il2CppClass * klass));
API_INIT(Il2CppClass*, class_get_declaring_type, (const Il2CppClass* klass));
API_INIT(int32_t, class_instance_size, (Il2CppClass * klass));
API_INIT(size_t, class_num_fields, (const Il2CppClass* enumKlass));
API_INIT(bool, class_is_valuetype, (const Il2CppClass* klass));
API_INIT(int32_t, class_value_size, (Il2CppClass * klass, uint32_t* align));
API_INIT(bool, class_is_blittable, (const Il2CppClass* klass));
API_INIT(int, class_get_flags, (const Il2CppClass* klass));
API_INIT(bool, class_is_abstract, (const Il2CppClass* klass));
API_INIT(bool, class_is_interface, (const Il2CppClass* klass));
API_INIT(int, class_array_element_size, (const Il2CppClass* klass));
API_INIT(Il2CppClass*, class_from_type, (const Il2CppType* type));
API_INIT(const Il2CppType*, class_get_type, (Il2CppClass * klass));
API_INIT(uint32_t, class_get_type_token, (Il2CppClass * klass));
API_INIT(bool, class_has_attribute, (Il2CppClass * klass, Il2CppClass* attr_class));
API_INIT(bool, class_has_references, (Il2CppClass * klass));
API_INIT(bool, class_is_enum, (const Il2CppClass* klass));
API_INIT(const Il2CppImage*, class_get_image, (Il2CppClass * klass));
API_INIT(const char*, class_get_assemblyname, (const Il2CppClass* klass));
API_INIT(int, class_get_rank, (const Il2CppClass* klass));
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
API_INIT(uint32_t, class_get_data_size, (const Il2CppClass* klass));
API_INIT(void*, class_get_static_field_data, (const Il2CppClass* klass));
#endif
#if defined(UNITY_2019) || defined(UNITY_2021)
API_INIT(size_t, class_get_bitmap_size, (const Il2CppClass* klass));
API_INIT(void, class_get_bitmap, (Il2CppClass * klass, size_t* bitmap));
#endif
API_INIT(bool, stats_dump_to_file, (const char* path));
API_INIT(uint64_t, stats_get_value, (Il2CppStat stat));
API_INIT(Il2CppDomain*, domain_get, ());
API_INIT(const Il2CppAssembly*, domain_assembly_open, (Il2CppDomain * domain, const char* name));
API_INIT(const Il2CppAssembly**, domain_get_assemblies, (const Il2CppDomain* domain, size_t* size));
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
API_INIT(void, raise_exception, (Il2CppException*));
#endif
API_INIT(Il2CppException*, exception_from_name_msg, (const Il2CppImage* image, const char* name_space, const char* name, const char* msg));
API_INIT(Il2CppException*, get_exception_argument_null, (const char* arg));
API_INIT(void, format_exception, (const Il2CppException* ex, char* message, int message_size));
API_INIT(void, format_stack_trace, (const Il2CppException* ex, char* output, int output_size));
API_INIT(void, unhandled_exception, (Il2CppException*));
API_INIT(int, field_get_flags, (FieldInfo * field));
API_INIT(const char*, field_get_name, (FieldInfo * field));
API_INIT(Il2CppClass*, field_get_parent, (FieldInfo * field));
API_INIT(size_t, field_get_offset, (FieldInfo * field));
API_INIT(const Il2CppType*, field_get_type, (FieldInfo * field));
API_INIT(void, field_get_value, (Il2CppObject * obj, FieldInfo* field, void* value));
API_INIT(Il2CppObject*, field_get_value_object, (FieldInfo * field, Il2CppObject* obj));
API_INIT(bool, field_has_attribute, (FieldInfo * field, Il2CppClass* attr_class));
API_INIT(void, field_set_value, (Il2CppObject * obj, FieldInfo* field, void* value));
API_INIT(void, field_static_get_value, (FieldInfo * field, void* value));
API_INIT(void, field_static_set_value, (FieldInfo * field, void* value));
API_INIT(void, field_set_value_object, (Il2CppObject * instance, FieldInfo* field, Il2CppObject* value));
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
API_INIT(bool, field_is_literal, (FieldInfo * field));
#endif
API_INIT(void, gc_collect, (int maxGenerations));
API_INIT(int32_t, gc_collect_a_little, ());
API_INIT(void, gc_disable, ());
API_INIT(void, gc_enable, ());
API_INIT(bool, gc_is_disabled, ());
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
API_INIT(int64_t, gc_get_max_time_slice_ns, ());
API_INIT(void, gc_set_max_time_slice_ns, (int64_t maxTimeSlice));
API_INIT(bool, gc_is_incremental, ());
#endif
API_INIT(int64_t, gc_get_used_size, ());
API_INIT(int64_t, gc_get_heap_size, ());
API_INIT(void, gc_wbarrier_set_field, (Il2CppObject * obj, void** targetAddress, void* object));
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
API_INIT(bool, gc_has_strict_wbarriers, ());
API_INIT(void, gc_set_external_allocation_tracker, (void (*func)(void*, size_t, int)));
API_INIT(void, gc_set_external_wbarrier_tracker, (void (*func)(void**)));
API_INIT(void, gc_foreach_heap, (void (*func)(void* data, void* userData), void* userData));
API_INIT(void*, gc_alloc_fixed, (std::size_t size));
API_INIT(void, gc_free_fixed, (void* addr));
API_INIT(void, stop_gc_world, ());
API_INIT(void, start_gc_world, ());
#endif
API_INIT(uint32_t, gchandle_new, (Il2CppObject * obj, bool pinned));
API_INIT(uint32_t, gchandle_new_weakref, (Il2CppObject * obj, bool track_resurrection));
API_INIT(Il2CppObject*, gchandle_get_target, (uint32_t gchandle));
API_INIT(void, gchandle_free, (uint32_t gchandle));
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
API_INIT(void, gchandle_foreach_get_target, (void (*func)(void* data, void* userData), void* userData));
API_INIT(uint32_t, object_header_size, ());
API_INIT(uint32_t, array_object_header_size, ());
API_INIT(uint32_t, offset_of_array_length_in_array_object_header, ());
API_INIT(uint32_t, offset_of_array_bounds_in_array_object_header, ());
API_INIT(uint32_t, allocation_granularity, ());
#endif
#if !defined(UNITY_2021) && !defined(UNITY_6)
API_INIT(void*, unity_liveness_calculation_begin,
         (Il2CppClass * filter, int max_object_count, il2cpp_register_object_callback callback, void* userdata, il2cpp_WorldChangedCallback onWorldStarted,
          il2cpp_WorldChangedCallback onWorldStopped));
API_INIT(void, unity_liveness_calculation_end, (void* state));
#endif
#if defined(UNITY_2021) || defined(UNITY_6)
API_INIT(void*, unity_liveness_allocate_struct, (Il2CppClass * filter, int max_object_count, il2cpp_register_object_callback callback, void* userdata, il2cpp_liveness_reallocate_callback reallocate));
API_INIT(void, unity_liveness_finalize, (void* state));
API_INIT(void, unity_liveness_free_struct, (void* state));
#endif
API_INIT(void, unity_liveness_calculation_from_root, (Il2CppObject * root, void* state));
API_INIT(void, unity_liveness_calculation_from_statics, (void* state));
API_INIT(const Il2CppType*, method_get_return_type, (const MethodInfo* method));
API_INIT(Il2CppClass*, method_get_declaring_type, (const MethodInfo* method));
API_INIT(const char*, method_get_name, (const MethodInfo* method));
API_INIT(const MethodInfo*, method_get_from_reflection, (const Il2CppReflectionMethod* method));
API_INIT(Il2CppReflectionMethod*, method_get_object, (const MethodInfo* method, Il2CppClass* refclass));
API_INIT(bool, method_is_generic, (const MethodInfo* method));
API_INIT(bool, method_is_inflated, (const MethodInfo* method));
API_INIT(bool, method_is_instance, (const MethodInfo* method));
API_INIT(uint32_t, method_get_param_count, (const MethodInfo* method));
API_INIT(const Il2CppType*, method_get_param, (const MethodInfo* method, uint32_t index));
API_INIT(Il2CppClass*, method_get_class, (const MethodInfo* method));
API_INIT(bool, method_has_attribute, (const MethodInfo* method, Il2CppClass* attr_class));
API_INIT(uint32_t, method_get_flags, (const MethodInfo* method, uint32_t* iflags));
API_INIT(uint32_t, method_get_token, (const MethodInfo* method));
API_INIT(const char*, method_get_param_name, (const MethodInfo* method, uint32_t index));

// ONLY IF THE PROFILER EXISTS FOR UNITY_2019
API_INIT(void, profiler_install, (Il2CppProfiler * prof, Il2CppProfileFunc shutdown_callback));
API_INIT(void, profiler_set_events, (Il2CppProfileFlags events));
API_INIT(void, profiler_install_enter_leave, (Il2CppProfileMethodFunc enter, Il2CppProfileMethodFunc fleave));
API_INIT(void, profiler_install_allocation, (Il2CppProfileAllocFunc callback));
API_INIT(void, profiler_install_gc, (Il2CppProfileGCFunc callback, Il2CppProfileGCResizeFunc heap_resize_callback));
API_INIT(void, profiler_install_fileio, (Il2CppProfileFileIOFunc callback));
API_INIT(void, profiler_install_thread, (Il2CppProfileThreadFunc start, Il2CppProfileThreadFunc end));

API_INIT(uint32_t, property_get_flags, (const PropertyInfo* prop));
API_INIT(const MethodInfo*, property_get_get_method, (const PropertyInfo* prop));
API_INIT(const MethodInfo*, property_get_set_method, (const PropertyInfo* prop));
API_INIT(const char*, property_get_name, (const PropertyInfo* prop));
API_INIT(Il2CppClass*, property_get_parent, (const PropertyInfo* prop));
API_INIT(Il2CppClass*, object_get_class, (Il2CppObject * obj));
API_INIT(uint32_t, object_get_size, (Il2CppObject * obj));
API_INIT(const MethodInfo*, object_get_virtual_method, (Il2CppObject * obj, const MethodInfo* method));
API_INIT(Il2CppObject*, object_new, (const Il2CppClass* klass));
// Always returns (void*, (obj + 1)
API_INIT(void*, object_unbox, (Il2CppObject * obj));
// If klass is not a ValueType, returns (Il2CppObject*, (*data), else boxes
API_INIT(Il2CppObject*, value_box, (Il2CppClass * klass, void* data));
API_INIT(void, monitor_enter, (Il2CppObject * obj));
API_INIT(bool, monitor_try_enter, (Il2CppObject * obj, uint32_t timeout));
API_INIT(void, monitor_exit, (Il2CppObject * obj));
API_INIT(void, monitor_pulse, (Il2CppObject * obj));
API_INIT(void, monitor_pulse_all, (Il2CppObject * obj));
API_INIT(void, monitor_wait, (Il2CppObject * obj));
API_INIT(bool, monitor_try_wait, (Il2CppObject * obj, uint32_t timeout));
API_INIT(Il2CppObject*, runtime_invoke, (const MethodInfo* method, void* obj, void** params, Il2CppException** exc));
API_INIT(Il2CppObject*, runtime_invoke_convert_args, (const MethodInfo* method, void* obj, Il2CppObject** params, int paramCount, Il2CppException** exc));
API_INIT(void, runtime_class_init, (Il2CppClass * klass));
API_INIT(void, runtime_object_init, (Il2CppObject * obj));
API_INIT(void, runtime_object_init_exception, (Il2CppObject * obj, Il2CppException** exc));
API_INIT(void, runtime_unhandled_exception_policy_set, (Il2CppRuntimeUnhandledExceptionPolicy value));
API_INIT(int32_t, string_length, (Il2CppString * str));
API_INIT(Il2CppChar*, string_chars, (Il2CppString * str));
API_INIT(Il2CppString*, string_new, (const char* str));
API_INIT(Il2CppString*, string_new_len, (const char* str, uint32_t length));
API_INIT(Il2CppString*, string_new_utf16, (const Il2CppChar* text, int32_t len));
API_INIT(Il2CppString*, string_new_wrapper, (const char* str));
API_INIT(Il2CppString*, string_intern, (Il2CppString * str));
API_INIT(Il2CppString*, string_is_interned, (Il2CppString * str));
API_INIT(Il2CppThread*, thread_current, ());
API_INIT(Il2CppThread*, thread_attach, (Il2CppDomain * domain));
API_INIT(void, thread_detach, (Il2CppThread * thread));
#if defined(UNITY_2019) || defined(UNITY_2021)
API_INIT(Il2CppThread**, thread_get_all_attached_threads, (size_t * size));
#endif
API_INIT(bool, is_vm_thread, (Il2CppThread * thread));
API_INIT(void, current_thread_walk_frame_stack, (Il2CppFrameWalkFunc func, void* user_data));
API_INIT(void, thread_walk_frame_stack, (Il2CppThread * thread, Il2CppFrameWalkFunc func, void* user_data));
API_INIT(bool, current_thread_get_top_frame, (Il2CppStackFrameInfo * frame));
API_INIT(bool, thread_get_top_frame, (Il2CppThread * thread, Il2CppStackFrameInfo* frame));
API_INIT(bool, current_thread_get_frame_at, (int32_t offset, Il2CppStackFrameInfo* frame));
API_INIT(bool, thread_get_frame_at, (Il2CppThread * thread, int32_t offset, Il2CppStackFrameInfo* frame));
API_INIT(int32_t, current_thread_get_stack_depth, ());
API_INIT(int32_t, thread_get_stack_depth, (Il2CppThread * thread));
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
API_INIT(void, override_stack_backtrace, (Il2CppBacktraceFunc stackBacktraceFunc));
#endif
API_INIT(Il2CppObject*, type_get_object, (const Il2CppType* type));
API_INIT(int, type_get_type, (const Il2CppType* type));
API_INIT(Il2CppClass*, type_get_class_or_element_class, (const Il2CppType* type));
API_INIT(char*, type_get_name, (const Il2CppType* type));
API_INIT(bool, type_is_byref, (const Il2CppType* type));
API_INIT(uint32_t, type_get_attrs, (const Il2CppType* type));
API_INIT(bool, type_equals, (const Il2CppType* type, const Il2CppType* otherType));
API_INIT(char*, type_get_assembly_qualified_name, (const Il2CppType* type));
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
API_INIT(bool, type_is_static, (const Il2CppType* type));
API_INIT(bool, type_is_pointer_type, (const Il2CppType* type));
#endif
API_INIT(const Il2CppAssembly*, image_get_assembly, (const Il2CppImage* image));
API_INIT(const char*, image_get_name, (const Il2CppImage* image));
API_INIT(const char*, image_get_filename, (const Il2CppImage* image));
API_INIT(const MethodInfo*, image_get_entry_point, (const Il2CppImage* image));
API_INIT(size_t, image_get_class_count, (const Il2CppImage* image));
API_INIT(const Il2CppClass*, image_get_class, (const Il2CppImage* image, size_t index));
API_INIT(Il2CppManagedMemorySnapshot*, capture_memory_snapshot, ());
API_INIT(void, free_captured_memory_snapshot, (Il2CppManagedMemorySnapshot * snapshot));
API_INIT(void, set_find_plugin_callback, (Il2CppSetFindPlugInCallback method));
API_INIT(void, register_log_callback, (Il2CppLogCallback method));
API_INIT(void, debugger_set_agent_options, (const char* options));
API_INIT(bool, is_debugger_attached, ());
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
API_INIT(void, register_debugger_agent_transport, (Il2CppDebuggerTransport * debuggerTransport));
API_INIT(bool, debug_get_method_info, (const MethodInfo*, Il2CppMethodDebugInfo* methodDebugInfo));
#endif
API_INIT(void, unity_install_unitytls_interface, (const void* unitytlsInterfaceStruct));
API_INIT(Il2CppCustomAttrInfo*, custom_attrs_from_class, (Il2CppClass * klass));
API_INIT(Il2CppCustomAttrInfo*, custom_attrs_from_method, (const MethodInfo* method));
API_INIT(Il2CppObject*, custom_attrs_get_attr, (Il2CppCustomAttrInfo * ainfo, Il2CppClass* attr_klass));
API_INIT(bool, custom_attrs_has_attr, (Il2CppCustomAttrInfo * ainfo, Il2CppClass* attr_klass));
API_INIT(Il2CppArray*, custom_attrs_construct, (Il2CppCustomAttrInfo * cinfo));
API_INIT(void, custom_attrs_free, (Il2CppCustomAttrInfo * ainfo));
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
API_INIT(void, class_set_userdata, (Il2CppClass * klass, void* userdata));
API_INIT(int, class_get_userdata_offset, ());
#endif

// MANUALLY DEFINED CONST DEFINITIONS
API_INIT(const Il2CppType*, class_get_type_const, (const Il2CppClass* klass));
API_INIT(const char*, class_get_name_const, (const Il2CppClass* klass));
API_INIT_MONO(Il2CppClass*, type_get_class, (Il2CppType* type));

// SELECT NON-API LIBIL2CPP FUNCTIONS:
API_INIT(bool, Class_Init, (Il2CppClass * klass));
#if defined(UNITY_2021) || defined(UNITY_6)
API_INIT(Il2CppClass*, MetadataCache_GetTypeInfoFromHandle, (Il2CppMetadataTypeHandle index));
#endif
API_INIT(Il2CppClass*, MetadataCache_GetTypeInfoFromTypeIndex, (TypeIndex index));

#if defined(UNITY_2021) || defined(UNITY_6)
API_INIT(Il2CppClass*, GlobalMetadata_GetTypeInfoFromTypeDefinitionIndex, (TypeDefinitionIndex index));
API_INIT(Il2CppClass*, GlobalMetadata_GetTypeInfoFromHandle, (Il2CppMetadataTypeHandle handle));
#endif

#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
API_INIT(std::string, _Type_GetName_, (const Il2CppType* type, Il2CppTypeNameFormat format));
#else
API_INIT(gnu_string, _Type_GetName_, (const Il2CppType* type, Il2CppTypeNameFormat format));
#endif
API_INIT(void, GC_free, (void* addr));

API_INIT(void, GarbageCollector_SetWriteBarrier, (void** ptr));
API_INIT(void*, GarbageCollector_AllocateFixed, (size_t sz, void* descr));

API_INIT(Il2CppClass*, Class_FromIl2CppType, (Il2CppType * typ));
API_INIT(Il2CppClass*, Class_GetPtrClass, (Il2CppClass * elementClass));
API_INIT(Il2CppClass*, GenericClass_GetClass, (Il2CppGenericClass * gclass));
API_INIT(Il2CppClass*, GenericClass_CreateClass, (Il2CppGenericClass * gclass, bool throwOnError));

API_INIT(AssemblyVector*, Assembly_GetAllAssemblies, ());

const Il2CppMetadataRegistration** il2cpp_functions::s_Il2CppMetadataRegistrationPtr;
const void** il2cpp_functions::s_GlobalMetadataPtr;
const Il2CppGlobalMetadataHeader** il2cpp_functions::s_GlobalMetadataHeaderPtr;

std::remove_pointer_t<decltype(il2cpp_functions::s_GlobalMetadataPtr)> il2cpp_functions::s_GlobalMetadata;
std::remove_pointer_t<decltype(il2cpp_functions::s_GlobalMetadataHeaderPtr)> il2cpp_functions::s_GlobalMetadataHeader;
std::remove_pointer_t<decltype(il2cpp_functions::s_Il2CppMetadataRegistrationPtr)> il2cpp_functions::s_Il2CppMetadataRegistration;

bool il2cpp_functions::hasGCFuncs;
const Il2CppDefaults* il2cpp_functions::defaults;
bool il2cpp_functions::initialized;

// copies of the highly-inlinable functions
const Il2CppTypeDefinition* il2cpp_functions::MetadataCache_GetTypeDefinitionFromIndex(TypeDefinitionIndex index) {
    CheckS_GlobalMetadata();
    if (index == kTypeDefinitionIndexInvalid) return NULL;

    IL2CPP_ASSERT(index >= 0 && static_cast<uint32_t>(index) < s_GlobalMetadataHeader->typeDefinitionsCount / sizeof(Il2CppTypeDefinition));
    auto typeDefinitions = (const Il2CppTypeDefinition*)((const char*)s_GlobalMetadata + s_GlobalMetadataHeader->typeDefinitionsOffset);
    return typeDefinitions + index;
}

const char* il2cpp_functions::MetadataCache_GetStringFromIndex(StringIndex index) {
    CheckS_GlobalMetadata();
    IL2CPP_ASSERT(index <= s_GlobalMetadataHeader->stringCount);
    const char* strings = ((const char*)s_GlobalMetadata + s_GlobalMetadataHeader->stringOffset) + index;
    return strings;
}

const Il2CppGenericContainer* il2cpp_functions::MetadataCache_GetGenericContainerFromIndex(GenericContainerIndex index) {
    CheckS_GlobalMetadata();
    if (index == kGenericContainerIndexInvalid) return NULL;

    IL2CPP_ASSERT(index >= 0 && static_cast<uint32_t>(index) <= s_GlobalMetadataHeader->genericContainersCount / sizeof(Il2CppGenericContainer));
    const Il2CppGenericContainer* genericContainers = (const Il2CppGenericContainer*)((const char*)s_GlobalMetadata + s_GlobalMetadataHeader->genericContainersOffset);
    return genericContainers + index;
}

const Il2CppGenericParameter* il2cpp_functions::MetadataCache_GetGenericParameterFromIndex(GenericParameterIndex index) {
    CheckS_GlobalMetadata();
    if (index == kGenericParameterIndexInvalid) return NULL;

    IL2CPP_ASSERT(index >= 0 && static_cast<uint32_t>(index) <= s_GlobalMetadataHeader->genericParametersCount / sizeof(Il2CppGenericParameter));
    const Il2CppGenericParameter* genericParameters = (const Il2CppGenericParameter*)((const char*)s_GlobalMetadata + s_GlobalMetadataHeader->genericParametersOffset);
    return genericParameters + index;
}

TypeDefinitionIndex il2cpp_functions::MetadataCache_GetExportedTypeFromIndex(TypeDefinitionIndex index) {
    CheckS_GlobalMetadata();
    if (index == kTypeDefinitionIndexInvalid) return kTypeDefinitionIndexInvalid;

    IL2CPP_ASSERT(index >= 0 && static_cast<uint32_t>(index) < s_GlobalMetadataHeader->exportedTypeDefinitionsCount / sizeof(TypeDefinitionIndex));
    auto exportedTypes = (TypeDefinitionIndex*)((const char*)s_GlobalMetadata + s_GlobalMetadataHeader->exportedTypeDefinitionsOffset);
    return *(exportedTypes + index);
}

Il2CppClass* il2cpp_functions::MetadataCache_GetNestedTypeFromIndex(NestedTypeIndex index) {
    CheckS_GlobalMetadata();
    IL2CPP_ASSERT(index >= 0 && static_cast<uint32_t>(index) <= s_GlobalMetadataHeader->nestedTypesCount / sizeof(TypeDefinitionIndex));
    auto nestedTypeIndices = (const TypeDefinitionIndex*)((const char*)s_GlobalMetadata + s_GlobalMetadataHeader->nestedTypesOffset);

    return il2cpp_functions::GlobalMetadata_GetTypeInfoFromTypeDefinitionIndex(nestedTypeIndices[index]);
}

TypeDefinitionIndex il2cpp_functions::MetadataCache_GetIndexForTypeDefinition(const Il2CppTypeDefinition* typeDefinition) {
    CheckS_GlobalMetadata();
    IL2CPP_ASSERT(klass);
    const Il2CppTypeDefinition* typeDefinitions = (const Il2CppTypeDefinition*)((const char*)s_GlobalMetadata + s_GlobalMetadataHeader->typeDefinitionsOffset);

    IL2CPP_ASSERT(typeDefinition->typeDefinition >= typeDefinitions && typeDefinition->typeDefinition < typeDefinitions + s_GlobalMetadataHeader->typeDefinitionsSize / sizeof(Il2CppTypeDefinition));
    ptrdiff_t index = typeDefinition - typeDefinitions;
    IL2CPP_ASSERT(index <= std::numeric_limits<TypeDefinitionIndex>::max());
    return static_cast<TypeDefinitionIndex>(index);
}

TypeDefinitionIndex il2cpp_functions::MetadataCache_GetIndexForTypeDefinition(const Il2CppClass* klass) {
    CheckS_GlobalMetadata();
    return MetadataCache_GetIndexForTypeDefinition(reinterpret_cast<const Il2CppTypeDefinition*>(klass->typeMetadataHandle));
}

GenericParameterIndex il2cpp_functions::MetadataCache_GetGenericParameterIndexFromParameter(Il2CppMetadataGenericParameterHandle handle) {
    CheckS_GlobalMetadata();
    const Il2CppGenericParameter* genericParameter = reinterpret_cast<const Il2CppGenericParameter*>(handle);
    const Il2CppGenericParameter* genericParameters = (const Il2CppGenericParameter*)((const char*)s_GlobalMetadata + s_GlobalMetadataHeader->genericParametersOffset);

    IL2CPP_ASSERT(genericParameter >= genericParameters && genericParameter < genericParameters + s_GlobalMetadataHeader->genericParametersSize / sizeof(Il2CppGenericParameter));

    ptrdiff_t index = reinterpret_cast<Il2CppGenericParameter const*>(genericParameter) - reinterpret_cast<Il2CppGenericParameter const*>(genericParameters);
    IL2CPP_ASSERT(index <= std::numeric_limits<GenericParameterIndex>::max());
    return static_cast<GenericParameterIndex>(index);
}

const Il2CppTypeDefinition* il2cpp_functions::MetadataCache_GetTypeDefinition(Il2CppClass* klass) {
    CheckS_GlobalMetadata();
    return reinterpret_cast<const Il2CppTypeDefinition*>(klass->typeMetadataHandle);
}

GenericParameterIndex il2cpp_functions::MetadataCache_GetGenericContainerIndex(Il2CppClass* klass) {
    CheckS_GlobalMetadata();
    return MetadataCache_GetTypeDefinition(klass)->genericContainerIndex;
}

char* il2cpp_functions::Type_GetName(const Il2CppType* type, Il2CppTypeNameFormat format) {
    if (!il2cpp__Type_GetName_) return nullptr;
    // TODO debug the ref/lifetime weirdness with _Type_GetName_ to avoid the need for explicit allocation
    const auto str = il2cpp_functions::_Type_GetName_(type, format);
    char* buffer = static_cast<char*>(il2cpp_functions::alloc(str.length() + 1));
    memcpy(buffer, str.c_str(), str.length() + 1);
    return buffer;
}

static std::optional<uint32_t*> blrFind(cs_insn* insn) {
    return insn->id == ARM64_INS_BLR ? std::optional<uint32_t*>(reinterpret_cast<uint32_t*>(insn->address)) : std::nullopt;
}

static std::optional<uint32_t*> findADRP(cs_insn* insn) {
    return (insn->id == ARM64_INS_ADRP) ? std::optional<uint32_t*>(reinterpret_cast<uint32_t*>(insn->address)) : std::nullopt;
}

void il2cpp_functions::find_GC_free(Paper::LoggerContext const& logger) {
    auto gc_free = cs::findNthB<1>(reinterpret_cast<uint32_t*>(il2cpp_functions::il2cpp_gc_free_fixed));
    if (!gc_free) SAFE_ABORT_MSG("Failed to find GC_free!");

    il2cpp_GC_free = reinterpret_cast<decltype(il2cpp_GC_free)>(*gc_free);
    logger.debug("gc::GarbageCollector::FreeFixed found? offset: {:X}", reinterpret_cast<uintptr_t>(il2cpp_GC_free) - getRealOffset(0));
}

bool il2cpp_functions::find_GC_SetWriteBarrier(const uint32_t* set_wbarrier_field) {
    if (!set_wbarrier_field) {
        return false;
    }
    il2cpp_GarbageCollector_SetWriteBarrier = reinterpret_cast<decltype(il2cpp_GarbageCollector_SetWriteBarrier)>(*set_wbarrier_field);
    return true;
}

void* (*wrapped_gc_malloc_uncollectable)(size_t sz, long long type);

void* __wrapper_gc_malloc_uncollectable(size_t sz, [[maybe_unused]] void* desc) {
    // 2 determined from ghidra dump of caller
    return wrapped_gc_malloc_uncollectable(sz, 2);
}

bool il2cpp_functions::trace_GC_AllocFixed(const uint32_t* DomainGetCurrent) {
    // Domain::GetCurrent has a single bl to GarbageCollector::AllocateFixed
    // MetadataCache::InitializeGCSafe is 3rd bl after first b.ne, which is the 6th b(.lt, .ne), t(bz, nz), c(bz, nz)
    auto tmp = cs::findNthBl<1>(DomainGetCurrent);
    if (!tmp) return false;
    il2cpp_GarbageCollector_AllocateFixed = reinterpret_cast<decltype(il2cpp_GarbageCollector_AllocateFixed)>(*tmp);
    return true;
}

bool il2cpp_functions::find_GC_AllocFixed() {
    auto result = cs::findNthB<1>(reinterpret_cast<const uint32_t*>(il2cpp_domain_get));
    if (!result) SAFE_ABORT_MSG("Failed to find Domain::Get!");
    if (!trace_GC_AllocFixed(*result)) {
        bool multipleMatches;
        auto sigMatch = findUniquePatternInLibil2cpp(multipleMatches,
                                                     "f5 0f 1d f8 f4 4f 01 a9 fd 7b 02 a9"
                                                     "fd 83 00 91 ?? ?? ?? ?? ?? ?? ?? ?? 1f 00 20 f1 f3 03 01 2a",
                                                     "GC_Malloc_Uncollectable");

        if (sigMatch && !multipleMatches) {
            // We need to make a wrapper method instead and set that.
            wrapped_gc_malloc_uncollectable = (decltype(wrapped_gc_malloc_uncollectable))sigMatch;
            il2cpp_GarbageCollector_AllocateFixed = &__wrapper_gc_malloc_uncollectable;
        } else {
            return false;
        }
    }
    return true;
}

/*
 * XREF for Class::Init
 * - il2cpp_class_from_system_type (symbol)
 *   - 2nd bl instruction -> Class::Init
 */
void il2cpp_functions::find_class_init(Paper::LoggerContext const& logger) {
    #ifdef UNITY_6
    const auto class_init = cs::findNthBl<2>(reinterpret_cast<const uint32_t*>(il2cpp_class_from_system_type));
    if (!class_init) SAFE_ABORT_MSG("Failed to find Class::Init!");
    il2cpp_Class_Init = reinterpret_cast<decltype(il2cpp_Class_Init)>(*class_init);
    logger.debug("Class::Init found? offset: {:X}", reinterpret_cast<uintptr_t>(il2cpp_Class_Init) - getRealOffset(0));
#else
#error "XREF for Class::Init needs to be updated for this Unity version!"
#endif
}

/*
 * XREF for Type::GetName
 * - il2cpp_type_get_assembly_qualified_name (symbol)
 *   - 1st Bl instruction -> Type::GetName
 */
void il2cpp_functions::find__type_get_name_(Paper::LoggerContext const& logger) {
#ifdef UNITY_6
    auto type_getName = cs::findNthBl<1>(reinterpret_cast<uint32_t*>(il2cpp_type_get_assembly_qualified_name));
    if (!type_getName) SAFE_ABORT_MSG("Failed to find Type::GetName!");
    il2cpp__Type_GetName_ = reinterpret_cast<decltype(il2cpp__Type_GetName_)>(*type_getName);
    logger.debug("Type::GetName found? offset: {:X}", reinterpret_cast<uintptr_t>(il2cpp__Type_GetName_) - getRealOffset(0));
#else
    #error "XREF for Type::GetName needs to be updated for this Unity version!"
#endif
}

/*
 * XREF for GlobalMetadata::GetTypeInfoFromTypeDefinitionIndex
 * - mono_type_get_class (symbol)
 *   - 1st B instruction -> GlobalMetadata::GetTypeInfoFromTypeDefinitionIndex
 */
void il2cpp_functions::find_get_type_info_from_type_definition_index() {
#ifdef UNITY_6
    auto get_type_info_from_type_definition_index = cs::findNthB<1, false, -1, 1024>(reinterpret_cast<uint32_t*>(mono_type_get_class));
    if (!get_type_info_from_type_definition_index) SAFE_ABORT_MSG("Failed to find GlobalMetadata::GetTypeInfoFromTypeDefinitionIndex!");
    il2cpp_GlobalMetadata_GetTypeInfoFromTypeDefinitionIndex = reinterpret_cast<decltype(il2cpp_GlobalMetadata_GetTypeInfoFromTypeDefinitionIndex)>(*get_type_info_from_type_definition_index);
#else
    #error "XREF for GlobalMetadata::GetTypeInfoFromTypeDefinitionIndex needs to be updated for this Unity version!"
#endif
}

/*
 * XREF for GlobalMetadata::GetTypeInfoFromTypeDefinitionIndex
 * - il2cpp_class_from_il2cpp_type (symbol)
 *   - 1st B instruction -> Class::FromIl2CppType
 */
void il2cpp_functions::find_class_from_il2cpp_type(Paper::LoggerContext const& logger) {
#ifdef UNITY_6
    const auto result = cs::findNthB<1, false, -1, 1024>(reinterpret_cast<uint32_t*>(il2cpp_class_from_il2cpp_type));
    if (!result) SAFE_ABORT_MSG("Failed to find Class::FromIl2CppType!");
    il2cpp_Class_FromIl2CppType = reinterpret_cast<decltype(il2cpp_Class_FromIl2CppType)>(*result);
    logger.debug("Class::FromIl2CppType found? offset: {:X}", reinterpret_cast<uintptr_t>(il2cpp_Class_FromIl2CppType) - getRealOffset(0));
#else
    #error "XREF for Class::FromIl2CppType needs to be updated for this Unity version!"
#endif
}

Il2CppClass* il2cpp_functions::type_info_from_handle(Il2CppMetadataTypeHandle handle) {
#if defined(UNITY_6)
    Il2CppType _type = {};
    _type.data.typeHandle = handle;
    Il2CppType* type = &_type;
    return mono_type_get_class(type);
#else
    #error "XREF for type_info_from_handle needs to be updated for this Unity version!"
#endif
}


void il2cpp_functions::find_generic_class_create_class(Paper::LoggerContext const& logger) {
#ifdef UNITY_6
    const auto GenericClass_CreateClass = cs::findNthB<6, false, -1, 1024>(reinterpret_cast<uint32_t*>(il2cpp_Class_FromIl2CppType));
    if (!GenericClass_CreateClass) SAFE_ABORT_MSG("Failed to find GenericClass::CreateClass!");
    il2cpp_GenericClass_CreateClass = reinterpret_cast<decltype(il2cpp_GenericClass_CreateClass)>(*GenericClass_CreateClass);
    logger.debug("GenericClass::CreateClass found? offset: {:X}", reinterpret_cast<uintptr_t>(il2cpp_Class_FromIl2CppType) - getRealOffset(0));
#else
    #error "XREF for GenericClass::CreateClass needs to be updated for this Unity version!"
#endif
}

/*
 * Original Function has become inlined, so recreate from decomp
 */
Il2CppClass* il2cpp_functions::generic_class_get_class(Il2CppGenericClass* gclass) {
#ifdef UNITY_6
    if (Il2CppClass* cachedClass = gclass->cached_class)
        return cachedClass;
    return il2cpp_GenericClass_CreateClass(gclass, true);
#else
    #error "XREF for GenericClass::GetClass needs to be updated for this Unity version!"
#endif
}

/*
 * XREF for Class::GetPtrClass
 * - il2cpp_Class_FromIl2CppType (XREF_FOUND)
 *   - 6th B instruction -> GenericClass::GetClass
 */
void il2cpp_functions::find_class_get_ptr_class(Paper::LoggerContext const& logger) {
#ifdef UNITY_6
    const auto Class_GetPtrClass_addr = cs::findNthB<6, false>(reinterpret_cast<uint32_t*>(il2cpp_Class_FromIl2CppType));
    if (!Class_GetPtrClass_addr) SAFE_ABORT_MSG("Failed to find Class::GetPtrClass!");
    il2cpp_Class_GetPtrClass = reinterpret_cast<decltype(il2cpp_Class_GetPtrClass)>(*Class_GetPtrClass_addr);
    logger.debug("Class::GetPtrClass(Il2CppClass*) found? offset: {:X}", reinterpret_cast<uintptr_t>(il2cpp_Class_GetPtrClass) - getRealOffset(0));
#else
    #error "XREF for Class::GetPtrClass needs to be updated for this Unity version!"
#endif
}

/*
 * XREF for il2cpp_defaults
 * - il2cpp_init (XREF_FOUND)
 *   - 2nd Bl instruction -> Runtime::Init
 *   - Runtime::Init
 *     - 13th ADRP -> ???
 *     - PC Address<1,1> -> defaults
 */
void il2cpp_functions::find_il2cpp_defaults(Paper::LoggerContext const& logger) {
#ifdef UNITY_6
    auto runtimeInit = cs::findNthBl<2>(reinterpret_cast<const uint32_t*>(il2cpp_init));
    if (!runtimeInit) SAFE_ABORT_MSG("Failed to find Runtime::Init!");

    auto ldr = cs::findNth<13, &findADRP, &cs::insnMatch<>, 1>(*runtimeInit);
    if (!ldr) SAFE_ABORT_MSG("Failed to find 13th ADRP in Runtime::Init!");

    auto defaults_addr = cs::getpcaddr<1, 1>(*ldr);
    if (!defaults_addr) SAFE_ABORT_MSG("Failed to find pcaddr around 13th ADRP in Runtime::Init!");

    defaults = reinterpret_cast<decltype(defaults)>(std::get<2>(*defaults_addr));

    logger.debug("il2cpp_defaults found: {} (offset: {:X})", fmt::ptr(defaults), reinterpret_cast<uintptr_t>(defaults) - getRealOffset(0));
#else
    #error "XREF for il2cpp_defaults needs to be updated for this Unity version!"
#endif
}


#define API_SYM(name)                                                \
*(void**)(&il2cpp_##name) = dlsym(imagehandle, "il2cpp_" #name); \
logger.debug("Loaded: " #name ", error: {}", bs_hooks::nullable(dlerror()))

void il2cpp_functions::init_api(Paper::LoggerContext const& logger, void* imagehandle) {
    #if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
    API_SYM(init);
    API_SYM(init_utf16);
#else
    API_SYM(init);
    API_SYM(init_utf16);
#endif
    API_SYM(shutdown);
    API_SYM(set_config_dir);
    API_SYM(set_data_dir);
    API_SYM(set_temp_dir);
    API_SYM(set_commandline_arguments);
    API_SYM(set_commandline_arguments_utf16);
    API_SYM(set_config_utf16);
    API_SYM(set_config);
    API_SYM(set_memory_callbacks);
    API_SYM(get_corlib);
    API_SYM(add_internal_call);
    API_SYM(resolve_icall);
    API_SYM(alloc);
    API_SYM(free);
    API_SYM(array_class_get);
    API_SYM(array_length);
    API_SYM(array_get_byte_length);
    API_SYM(array_new);
    API_SYM(array_new_specific);
    API_SYM(array_new_full);
    API_SYM(bounded_array_class_get);
    API_SYM(array_element_size);
    API_SYM(assembly_get_image);
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
    API_SYM(class_for_each);
#endif
    API_SYM(class_enum_basetype);
    API_SYM(class_is_generic);
    API_SYM(class_is_inflated);
    API_SYM(class_is_assignable_from);
    API_SYM(class_is_subclass_of);
    API_SYM(class_has_parent);
    API_SYM(class_from_il2cpp_type);
    API_SYM(class_from_name);
    API_SYM(class_from_system_type);
    API_SYM(class_get_element_class);
    API_SYM(class_get_events);
    API_SYM(class_get_fields);
    API_SYM(class_get_nested_types);
    API_SYM(class_get_interfaces);
    API_SYM(class_get_properties);
    API_SYM(class_get_property_from_name);
    API_SYM(class_get_field_from_name);
    API_SYM(class_get_methods);
    API_SYM(class_get_method_from_name);
    API_SYM(class_get_name);
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
    API_SYM(type_get_name_chunked);
#endif
    API_SYM(class_get_namespace);
    API_SYM(class_get_parent);
    API_SYM(class_get_declaring_type);
    API_SYM(class_instance_size);
    API_SYM(class_num_fields);
    API_SYM(class_is_valuetype);
    API_SYM(class_value_size);
    API_SYM(class_is_blittable);
    API_SYM(class_get_flags);
    API_SYM(class_is_abstract);
    API_SYM(class_is_interface);
    API_SYM(class_array_element_size);
    API_SYM(class_from_type);
    API_SYM(class_get_type);
    API_SYM(class_get_type_token);
    API_SYM(class_has_attribute);
    API_SYM(class_has_references);
    API_SYM(class_is_enum);
    API_SYM(class_get_image);
    API_SYM(class_get_assemblyname);
    API_SYM(class_get_rank);
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
    API_SYM(class_get_data_size);
    API_SYM(class_get_static_field_data);
#endif
#if defined(UNITY_2019)
    API_SYM(class_get_bitmap_size);
    API_SYM(class_get_bitmap);
#endif
    API_SYM(stats_dump_to_file);
    API_SYM(stats_get_value);
    API_SYM(domain_get);
    API_SYM(domain_assembly_open);
    API_SYM(domain_get_assemblies);
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
    API_SYM(raise_exception);
#endif
    API_SYM(exception_from_name_msg);
    API_SYM(get_exception_argument_null);
    API_SYM(format_exception);
    API_SYM(format_stack_trace);
    API_SYM(unhandled_exception);
    API_SYM(field_get_flags);
    API_SYM(field_get_name);
    API_SYM(field_get_parent);
    API_SYM(field_get_offset);
    API_SYM(field_get_type);
    API_SYM(field_get_value);
    API_SYM(field_get_value_object);
    API_SYM(field_has_attribute);
    API_SYM(field_set_value);
    API_SYM(field_static_get_value);
    API_SYM(field_static_set_value);
    API_SYM(field_set_value_object);
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
    API_SYM(field_is_literal);
#endif
    API_SYM(gc_collect);
    API_SYM(gc_collect_a_little);
    API_SYM(gc_disable);
    API_SYM(gc_enable);
    API_SYM(gc_is_disabled);
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
    API_SYM(gc_get_max_time_slice_ns);
    API_SYM(gc_set_max_time_slice_ns);
    API_SYM(gc_is_incremental);
#endif
    API_SYM(gc_get_used_size);
    API_SYM(gc_get_heap_size);
    API_SYM(gc_wbarrier_set_field);
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
    API_SYM(gc_has_strict_wbarriers);
    API_SYM(gc_set_external_allocation_tracker);
    API_SYM(gc_set_external_wbarrier_tracker);
    API_SYM(gc_foreach_heap);
    API_SYM(gc_free_fixed);
    API_SYM(gc_alloc_fixed);
    API_SYM(stop_gc_world);
    API_SYM(start_gc_world);
#endif
    API_SYM(gchandle_new);
    API_SYM(gchandle_new_weakref);
    API_SYM(gchandle_get_target);
    API_SYM(gchandle_free);
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
    API_SYM(gchandle_foreach_get_target);
    API_SYM(object_header_size);
    API_SYM(array_object_header_size);
    API_SYM(offset_of_array_length_in_array_object_header);
    API_SYM(offset_of_array_bounds_in_array_object_header);
    API_SYM(allocation_granularity);
#endif
#if !defined(UNITY_2021) && !defined(UNITY_6)
    API_SYM(unity_liveness_calculation_begin);
    API_SYM(unity_liveness_calculation_end);
#endif
#if defined(UNITY_2021) || defined(UNITY_6)
    API_SYM(unity_liveness_allocate_struct);
    API_SYM(unity_liveness_finalize);
    API_SYM(unity_liveness_free_struct);
#endif
    API_SYM(unity_liveness_calculation_from_root);
    API_SYM(unity_liveness_calculation_from_statics);
    API_SYM(method_get_return_type);
    API_SYM(method_get_declaring_type);
    API_SYM(method_get_name);
    API_SYM(method_get_from_reflection);
    API_SYM(method_get_object);
    API_SYM(method_is_generic);
    API_SYM(method_is_inflated);
    API_SYM(method_is_instance);
    API_SYM(method_get_param_count);
    API_SYM(method_get_param);
    API_SYM(method_get_class);
    API_SYM(method_has_attribute);
    API_SYM(method_get_flags);
    API_SYM(method_get_token);
    API_SYM(method_get_param_name);

    // ONLY IF THE PROFILER EXISTS FOR UNITY_2019
    API_SYM(profiler_install);
    API_SYM(profiler_set_events);
    API_SYM(profiler_install_enter_leave);
    API_SYM(profiler_install_allocation);
    API_SYM(profiler_install_gc);
    API_SYM(profiler_install_fileio);
    API_SYM(profiler_install_thread);

    API_SYM(property_get_flags);
    API_SYM(property_get_get_method);
    API_SYM(property_get_set_method);
    API_SYM(property_get_name);
    API_SYM(property_get_parent);
    API_SYM(object_get_class);
    API_SYM(object_get_size);
    API_SYM(object_get_virtual_method);
    API_SYM(object_new);
    // Always returns (void*, (obj + 1)
    API_SYM(object_unbox);
    // If klass is not a ValueType, returns (Il2CppObject*, (*data), else boxes
    API_SYM(value_box);
    API_SYM(monitor_enter);
    API_SYM(monitor_try_enter);
    API_SYM(monitor_exit);
    API_SYM(monitor_pulse);
    API_SYM(monitor_pulse_all);
    API_SYM(monitor_wait);
    API_SYM(monitor_try_wait);
    API_SYM(runtime_invoke);
    API_SYM(runtime_invoke_convert_args);
    API_SYM(runtime_class_init);
    API_SYM(runtime_object_init);
    API_SYM(runtime_object_init_exception);
    API_SYM(runtime_unhandled_exception_policy_set);
    API_SYM(string_length);
    API_SYM(string_chars);
    API_SYM(string_new);
    API_SYM(string_new_len);
    API_SYM(string_new_utf16);
    API_SYM(string_new_wrapper);
    API_SYM(string_intern);
    API_SYM(string_is_interned);
    API_SYM(thread_current);
    API_SYM(thread_attach);
    API_SYM(thread_detach);
#if defined(UNITY_2019) || defined(UNITY_2021)
    API_SYM(thread_get_all_attached_threads);
#endif
    API_SYM(is_vm_thread);
    API_SYM(current_thread_walk_frame_stack);
    API_SYM(thread_walk_frame_stack);
    API_SYM(current_thread_get_top_frame);
    API_SYM(thread_get_top_frame);
    API_SYM(current_thread_get_frame_at);
    API_SYM(thread_get_frame_at);
    API_SYM(current_thread_get_stack_depth);
    API_SYM(thread_get_stack_depth);
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
    API_SYM(override_stack_backtrace);
#endif
    API_SYM(type_get_object);
    API_SYM(type_get_type);
    API_SYM(type_get_class_or_element_class);
    API_SYM(type_get_name);
    API_SYM(type_is_byref);
    API_SYM(type_get_attrs);
    API_SYM(type_equals);
    API_SYM(type_get_assembly_qualified_name);
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
    API_SYM(type_is_static);
    API_SYM(type_is_pointer_type);
#endif
    API_SYM(image_get_assembly);
    API_SYM(image_get_name);
    API_SYM(image_get_filename);
    API_SYM(image_get_entry_point);
    API_SYM(image_get_class_count);
    API_SYM(image_get_class);
    API_SYM(capture_memory_snapshot);
    API_SYM(free_captured_memory_snapshot);
    API_SYM(set_find_plugin_callback);
    API_SYM(register_log_callback);
    API_SYM(debugger_set_agent_options);
    API_SYM(is_debugger_attached);
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
    API_SYM(register_debugger_agent_transport);
    API_SYM(debug_get_method_info);
#endif
    API_SYM(unity_install_unitytls_interface);
    API_SYM(custom_attrs_from_class);
    API_SYM(custom_attrs_from_method);
    API_SYM(custom_attrs_get_attr);
    API_SYM(custom_attrs_has_attr);
    API_SYM(custom_attrs_construct);
    API_SYM(custom_attrs_free);
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
    API_SYM(class_set_userdata);
    API_SYM(class_get_userdata_offset);
#endif
}

void LogOffset(Paper::LoggerContext const& logger, const char* name, uintptr_t ptr) {
    logger.debug("{}: offset {:X}", name, ptr - getRealOffset(0));
}

// Autogenerated
// Initializes all of the IL2CPP functions via dlopen and dlsym for use.
void il2cpp_functions::Init() {
    if (initialized) {
        return;
    }
    auto const& logger = il2cpp_utils::Logger;

    // Register for paper to write file
    Paper::Logger::RegisterFileContextId(logger.tag);

    logger.info("il2cpp_functions: Init: Initializing all IL2CPP Functions...");
    dlerror();  // clears existing errors

    void* imagehandle = modloader_libil2cpp_handle;

    if (!imagehandle) {
        logger.error("Failed to grab modloader libil2cpp.so handle: {}", fmt::ptr(imagehandle));
        return;
    }

    init_api(logger, imagehandle);

    // MANUALLY DEFINED CONST DEFINITIONS
    *(void**)(&il2cpp_class_get_type_const) = dlsym(imagehandle, "il2cpp_class_get_type");
    logger.info("Loaded: il2cpp_class_get_type CONST VERSION!");
    *(void**)(&il2cpp_class_get_name_const) = dlsym(imagehandle, "il2cpp_class_get_name");
    logger.info("Loaded: il2cpp_class_get_name CONST VERSION!");
    *(void**)(&mono_type_get_class) = dlsym(imagehandle, "mono_type_get_class");
    logger.info("Loaded: mono_type_get_class");

    find_class_init(logger);

    il2cpp_MetadataCache_GetTypeInfoFromHandle = type_info_from_handle;
    il2cpp_GlobalMetadata_GetTypeInfoFromHandle = type_info_from_handle;
    find_get_type_info_from_type_definition_index();

    find__type_get_name_(logger);
    find_class_from_il2cpp_type(logger);
    find_generic_class_create_class(logger);
    il2cpp_GenericClass_GetClass = generic_class_get_class;
    find_class_get_ptr_class(logger);

    auto get_type_info_from_type_definition_index = cs::findNthB<1, false, -1, 1024>(reinterpret_cast<uint32_t*>(mono_type_get_class));
    if (!get_type_info_from_type_definition_index) SAFE_ABORT_MSG("Failed to find GlobalMetadata::GetTypeInfoFromTypeDefinitionIndex!");
    il2cpp_GlobalMetadata_GetTypeInfoFromTypeDefinitionIndex = reinterpret_cast<decltype(il2cpp_GlobalMetadata_GetTypeInfoFromTypeDefinitionIndex)>(*get_type_info_from_type_definition_index);

    {
        // Assembly::GetAllAssemblies
        auto result = cs::findNthBl<1>(reinterpret_cast<const uint32_t*>(il2cpp_domain_get_assemblies));
        if (!result) SAFE_ABORT_MSG("Failed to find Assembly::GetAllAssemblies!");
        il2cpp_Assembly_GetAllAssemblies = reinterpret_cast<decltype(il2cpp_Assembly_GetAllAssemblies)>(*result);
        logger.debug("Assembly::GetAllAssemblies found? offset: {:X}", reinterpret_cast<uintptr_t>(il2cpp_Assembly_GetAllAssemblies) - getRealOffset(0));
    }


    CRASH_UNLESS(il2cpp_shutdown);
    // GC_free
    find_GC_free(logger);

    // GarbageCollector::SetWriteBarrier(void*)
    /*if (find_GC_SetWriteBarrier(reinterpret_cast<const uint32_t*>())) {
        logger.debug("GarbageCollector::SetWriteBarrier found? offset: {:X}", reinterpret_cast<uintptr_t>(il2cpp_GarbageCollector_SetWriteBarrier) - getRealOffset(0));
    }*/

    // GarbageCollector::AllocateFixed(size_t, void*)
    if (find_GC_AllocFixed()) {
        logger.debug("GarbageCollector::AllocateFixed found? offset: {:X}", reinterpret_cast<uintptr_t>(il2cpp_GarbageCollector_AllocateFixed) - getRealOffset(0));
    }


    hasGCFuncs = il2cpp_GarbageCollector_AllocateFixed != nullptr && il2cpp_GC_free != nullptr;

    find_il2cpp_defaults(logger);
    {
        // FIELDS
        // Extract locations of s_GlobalMetadataHeader, s_Il2CppMetadataRegistration, & s_GlobalMetadata

        auto tmp = cs::getpcaddr<3, 1>(reinterpret_cast<const uint32_t*>(il2cpp_GlobalMetadata_GetTypeInfoFromTypeDefinitionIndex));
        if (!tmp) SAFE_ABORT_MSG("Failed to find 3rd pcaddr for s_GlobalMetadataHeaderPtr!");
        s_GlobalMetadataHeaderPtr = reinterpret_cast<decltype(s_GlobalMetadataHeaderPtr)>(std::get<2>(*tmp));

        tmp = cs::getpcaddr<5, 1>(reinterpret_cast<const uint32_t*>(il2cpp_GlobalMetadata_GetTypeInfoFromTypeDefinitionIndex));
        if (!tmp) SAFE_ABORT_MSG("Failed to find 4th pcaddr for s_Il2CppMetadataRegistrationPtr!");
        s_Il2CppMetadataRegistrationPtr = reinterpret_cast<decltype(s_Il2CppMetadataRegistrationPtr)>(std::get<2>(*tmp));

        tmp = cs::getpcaddr<4, 1>(reinterpret_cast<const uint32_t*>(il2cpp_GlobalMetadata_GetTypeInfoFromTypeDefinitionIndex));
        if (!tmp) SAFE_ABORT_MSG("Failed to find 5th pcaddr for s_GlobalMetadataPtr!");
        s_GlobalMetadataPtr = reinterpret_cast<decltype(s_GlobalMetadataPtr)>(std::get<2>(*tmp));

        logger.debug("{:X} {:X} {:X} metadata pointers offsets", (uintptr_t)s_GlobalMetadataHeaderPtr - getRealOffset(0), (uintptr_t)s_Il2CppMetadataRegistrationPtr - getRealOffset(0), (uintptr_t)s_GlobalMetadataPtr - getRealOffset(0));
        logger.debug("All global constants found!");
    }

    logger.debug("il2cpp_functions: Printing offsets");
    LogOffset(logger, "Class::Init", reinterpret_cast<uintptr_t>(il2cpp_Class_Init));
    LogOffset(logger, "GlobalMetadata::GetTypeInfoFromTypeDefinitionIndex", reinterpret_cast<uintptr_t>(il2cpp_GlobalMetadata_GetTypeInfoFromTypeDefinitionIndex));
    LogOffset(logger, "Type::GetName", reinterpret_cast<uintptr_t>(il2cpp__Type_GetName_));
    LogOffset(logger, "Class::FromIl2cppType", reinterpret_cast<uintptr_t>(il2cpp_Class_FromIl2CppType));
    LogOffset(logger, "GenericClass::GetClass", reinterpret_cast<uintptr_t>(il2cpp_GenericClass_GetClass));
    LogOffset(logger, "Class::GetPtrClass", reinterpret_cast<uintptr_t>(il2cpp_Class_GetPtrClass));
    LogOffset(logger, "s_GlobalMetadataHeaderPtr", reinterpret_cast<uintptr_t>(s_GlobalMetadataHeaderPtr));
    LogOffset(logger, "s_Il2CppMetadataRegistrationPtr", reinterpret_cast<uintptr_t>(s_Il2CppMetadataRegistrationPtr));
    LogOffset(logger, "s_GlobalMetadataPtr", reinterpret_cast<uintptr_t>(s_GlobalMetadataPtr));
    LogOffset(logger, "GC::Free", reinterpret_cast<uintptr_t>(il2cpp_GC_free));
    LogOffset(logger, "GarbageCollector::SetWriteBarrier", reinterpret_cast<uintptr_t>(il2cpp_GarbageCollector_SetWriteBarrier));
    LogOffset(logger, "GarbageCollector::AllocateFixed", reinterpret_cast<uintptr_t>(il2cpp_GarbageCollector_AllocateFixed));
    initialized = true;
    logger.info("il2cpp_functions: Init: Successfully loaded all il2cpp functions!");
}
