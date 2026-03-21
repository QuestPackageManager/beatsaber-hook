#include "api.hpp"

#include "binary.hpp"
#include "capstone.hpp"

// #include <unistd.h>

#define API_INIT(rt, name, ...) rt(*i2c::functions::name) __VA_ARGS__

// All the fields...
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
API_INIT(int, init, (char const* domain_name));
API_INIT(int, init_utf16, (Il2CppChar const* domain_name));
#else
API_INIT(void, init, (char const* domain_name));
API_INIT(void, init_utf16, (Il2CppChar const* domain_name));
#endif
API_INIT(void, shutdown, ());
API_INIT(void, set_config_dir, (char const* config_path));
API_INIT(void, set_data_dir, (char const* data_path));
API_INIT(void, set_temp_dir, (char const* temp_path));
API_INIT(void, set_commandline_arguments, (int argc, char const* const argv[], char const* basedir));
API_INIT(void, set_commandline_arguments_utf16, (int argc, Il2CppChar const* const argv[], char const* basedir));
API_INIT(void, set_config_utf16, (Il2CppChar const* executablePath));
API_INIT(void, set_config, (char const* executablePath));
API_INIT(void, set_memory_callbacks, (Il2CppMemoryCallbacks * callbacks));
API_INIT(Il2CppImage const*, get_corlib, ());
API_INIT(void, add_internal_call, (char const* name, Il2CppMethodPointer method));
API_INIT(Il2CppMethodPointer, resolve_icall, (char const* name));
API_INIT(void*, alloc, (size_t size));
API_INIT(void, free, (void* ptr));
API_INIT(Il2CppClass*, array_class_get, (Il2CppClass * element_class, uint32_t rank));
API_INIT(uint32_t, array_length, (Il2CppArray * array));
API_INIT(uint32_t, array_get_byte_length, (Il2CppArray * array));
API_INIT(Il2CppArray*, array_new, (Il2CppClass * elementTypeInfo, il2cpp_array_size_t length));
API_INIT(Il2CppArray*, array_new_specific, (Il2CppClass * arrayTypeInfo, il2cpp_array_size_t length));
API_INIT(Il2CppArray*, array_new_full, (Il2CppClass * array_class, il2cpp_array_size_t* lengths, il2cpp_array_size_t* lower_bounds));
API_INIT(Il2CppClass*, bounded_array_class_get, (Il2CppClass * element_class, uint32_t rank, bool bounded));
API_INIT(int, array_element_size, (Il2CppClass const* array_class));
API_INIT(Il2CppImage const*, assembly_get_image, (Il2CppAssembly const* assembly));
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
API_INIT(void, class_for_each, (void (*klassReportFunc)(Il2CppClass* klass, void* userData), void* userData));
#endif
API_INIT(const Il2CppType*, class_enum_basetype, (Il2CppClass * klass));
API_INIT(bool, class_is_generic, (Il2CppClass const* klass));
API_INIT(bool, class_is_inflated, (Il2CppClass const* klass));
API_INIT(bool, class_is_assignable_from, (Il2CppClass * klass, Il2CppClass* oklass));
API_INIT(bool, class_is_subclass_of, (Il2CppClass * klass, Il2CppClass* klassc, bool check_interfaces));
API_INIT(bool, class_has_parent, (Il2CppClass * klass, Il2CppClass* klassc));
API_INIT(Il2CppClass*, class_from_il2cpp_type, (Il2CppType const* type));
API_INIT(Il2CppClass*, class_from_name, (Il2CppImage const* image, char const* namespaze, char const* name));
API_INIT(Il2CppClass*, class_from_system_type, (Il2CppReflectionType * type));
API_INIT(Il2CppClass*, class_get_element_class, (Il2CppClass * klass));
API_INIT(EventInfo const*, class_get_events, (Il2CppClass * klass, void** iter));
API_INIT(FieldInfo*, class_get_fields, (Il2CppClass * klass, void** iter));
API_INIT(Il2CppClass*, class_get_nested_types, (Il2CppClass * klass, void** iter));
API_INIT(Il2CppClass*, class_get_interfaces, (Il2CppClass * klass, void** iter));
API_INIT(PropertyInfo const*, class_get_properties, (Il2CppClass * klass, void** iter));
API_INIT(PropertyInfo const*, class_get_property_from_name, (Il2CppClass * klass, char const* name));
API_INIT(FieldInfo*, class_get_field_from_name, (Il2CppClass * klass, char const* name));
API_INIT(MethodInfo const*, class_get_methods, (Il2CppClass * klass, void** iter));
API_INIT(MethodInfo const*, class_get_method_from_name, (Il2CppClass const* klass, char const* name, int argsCount));
API_INIT(char const*, class_get_name, (Il2CppClass const* klass));
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
API_INIT(void, type_get_name_chunked, (const Il2CppType* type, void (*chunkReportFunc)(void* data, void* userData), void* userData));
#endif
API_INIT(const char*, class_get_namespace, (const Il2CppClass* klass));
API_INIT(Il2CppClass*, class_get_parent, (Il2CppClass * klass));
API_INIT(Il2CppClass*, class_get_declaring_type, (Il2CppClass const* klass));
API_INIT(int32_t, class_instance_size, (Il2CppClass * klass));
API_INIT(size_t, class_num_fields, (Il2CppClass const* enumKlass));
API_INIT(bool, class_is_valuetype, (Il2CppClass const* klass));
API_INIT(int32_t, class_value_size, (Il2CppClass * klass, uint32_t* align));
API_INIT(bool, class_is_blittable, (Il2CppClass const* klass));
API_INIT(int, class_get_flags, (Il2CppClass const* klass));
API_INIT(bool, class_is_abstract, (Il2CppClass const* klass));
API_INIT(bool, class_is_interface, (Il2CppClass const* klass));
API_INIT(int, class_array_element_size, (Il2CppClass const* klass));
API_INIT(Il2CppClass*, class_from_type, (Il2CppType const* type));
API_INIT(Il2CppType const*, class_get_type, (Il2CppClass * klass));
API_INIT(uint32_t, class_get_type_token, (Il2CppClass * klass));
API_INIT(bool, class_has_attribute, (Il2CppClass * klass, Il2CppClass* attr_class));
API_INIT(bool, class_has_references, (Il2CppClass * klass));
API_INIT(bool, class_is_enum, (Il2CppClass const* klass));
API_INIT(Il2CppImage const*, class_get_image, (Il2CppClass * klass));
API_INIT(char const*, class_get_assemblyname, (Il2CppClass const* klass));
API_INIT(int, class_get_rank, (Il2CppClass const* klass));
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
API_INIT(uint32_t, class_get_data_size, (const Il2CppClass* klass));
API_INIT(void*, class_get_static_field_data, (Il2CppClass const* klass));
#endif
#if defined(UNITY_2019) || defined(UNITY_2021)
API_INIT(size_t, class_get_bitmap_size, (const Il2CppClass* klass));
API_INIT(void, class_get_bitmap, (Il2CppClass * klass, size_t* bitmap));
#endif
API_INIT(bool, stats_dump_to_file, (const char* path));
API_INIT(uint64_t, stats_get_value, (Il2CppStat stat));
API_INIT(Il2CppDomain*, domain_get, ());
API_INIT(Il2CppAssembly const*, domain_assembly_open, (Il2CppDomain * domain, char const* name));
API_INIT(Il2CppAssembly const**, domain_get_assemblies, (Il2CppDomain const* domain, size_t* size));
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
API_INIT(void, raise_exception, (Il2CppException*) );
#endif
API_INIT(Il2CppException*, exception_from_name_msg, (const Il2CppImage* image, const char* name_space, const char* name, const char* msg));
API_INIT(Il2CppException*, get_exception_argument_null, (char const* arg));
API_INIT(void, format_exception, (Il2CppException const* ex, char* message, int message_size));
API_INIT(void, format_stack_trace, (Il2CppException const* ex, char* output, int output_size));
API_INIT(void, unhandled_exception, (Il2CppException*) );
API_INIT(int, field_get_flags, (FieldInfo * field));
API_INIT(char const*, field_get_name, (FieldInfo * field));
API_INIT(Il2CppClass*, field_get_parent, (FieldInfo * field));
API_INIT(size_t, field_get_offset, (FieldInfo * field));
API_INIT(Il2CppType const*, field_get_type, (FieldInfo * field));
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
API_INIT(
    void*,
    unity_liveness_calculation_begin,
    (Il2CppClass * filter,
     int max_object_count,
     il2cpp_register_object_callback callback,
     void* userdata,
     il2cpp_WorldChangedCallback onWorldStarted,
     il2cpp_WorldChangedCallback onWorldStopped)
);
API_INIT(void, unity_liveness_calculation_end, (void* state));
#endif
#if defined(UNITY_2021) || defined(UNITY_6)
API_INIT(
    void*,
    unity_liveness_allocate_struct,
    (Il2CppClass * filter,
     int max_object_count,
     il2cpp_register_object_callback callback,
     void* userdata,
     il2cpp_liveness_reallocate_callback reallocate)
);
API_INIT(void, unity_liveness_finalize, (void* state));
API_INIT(void, unity_liveness_free_struct, (void* state));
#endif
API_INIT(void, unity_liveness_calculation_from_root, (Il2CppObject * root, void* state));
API_INIT(void, unity_liveness_calculation_from_statics, (void* state));
API_INIT(Il2CppType const*, method_get_return_type, (MethodInfo const* method));
API_INIT(Il2CppClass*, method_get_declaring_type, (MethodInfo const* method));
API_INIT(char const*, method_get_name, (MethodInfo const* method));
API_INIT(MethodInfo const*, method_get_from_reflection, (Il2CppReflectionMethod const* method));
API_INIT(Il2CppReflectionMethod*, method_get_object, (MethodInfo const* method, Il2CppClass* refclass));
API_INIT(bool, method_is_generic, (MethodInfo const* method));
API_INIT(bool, method_is_inflated, (MethodInfo const* method));
API_INIT(bool, method_is_instance, (MethodInfo const* method));
API_INIT(uint32_t, method_get_param_count, (MethodInfo const* method));
API_INIT(Il2CppType const*, method_get_param, (MethodInfo const* method, uint32_t index));
API_INIT(Il2CppClass*, method_get_class, (MethodInfo const* method));
API_INIT(bool, method_has_attribute, (MethodInfo const* method, Il2CppClass* attr_class));
API_INIT(uint32_t, method_get_flags, (MethodInfo const* method, uint32_t* iflags));
API_INIT(uint32_t, method_get_token, (MethodInfo const* method));
API_INIT(char const*, method_get_param_name, (MethodInfo const* method, uint32_t index));

// ONLY IF THE PROFILER EXISTS FOR UNITY_2019
API_INIT(void, profiler_install, (Il2CppProfiler * prof, Il2CppProfileFunc shutdown_callback));
API_INIT(void, profiler_set_events, (Il2CppProfileFlags events));
API_INIT(void, profiler_install_enter_leave, (Il2CppProfileMethodFunc enter, Il2CppProfileMethodFunc fleave));
API_INIT(void, profiler_install_allocation, (Il2CppProfileAllocFunc callback));
API_INIT(void, profiler_install_gc, (Il2CppProfileGCFunc callback, Il2CppProfileGCResizeFunc heap_resize_callback));
API_INIT(void, profiler_install_fileio, (Il2CppProfileFileIOFunc callback));
API_INIT(void, profiler_install_thread, (Il2CppProfileThreadFunc start, Il2CppProfileThreadFunc end));

API_INIT(uint32_t, property_get_flags, (PropertyInfo const* prop));
API_INIT(MethodInfo const*, property_get_get_method, (PropertyInfo const* prop));
API_INIT(MethodInfo const*, property_get_set_method, (PropertyInfo const* prop));
API_INIT(char const*, property_get_name, (PropertyInfo const* prop));
API_INIT(Il2CppClass*, property_get_parent, (PropertyInfo const* prop));
API_INIT(Il2CppClass*, object_get_class, (Il2CppObject * obj));
API_INIT(uint32_t, object_get_size, (Il2CppObject * obj));
API_INIT(MethodInfo const*, object_get_virtual_method, (Il2CppObject * obj, MethodInfo const* method));
API_INIT(Il2CppObject*, object_new, (Il2CppClass const* klass));
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
API_INIT(Il2CppObject*, runtime_invoke, (MethodInfo const* method, void* obj, void** params, Il2CppException** exc));
API_INIT(
    Il2CppObject*, runtime_invoke_convert_args, (MethodInfo const* method, void* obj, Il2CppObject** params, int paramCount, Il2CppException** exc)
);
API_INIT(void, runtime_class_init, (Il2CppClass * klass));
API_INIT(void, runtime_object_init, (Il2CppObject * obj));
API_INIT(void, runtime_object_init_exception, (Il2CppObject * obj, Il2CppException** exc));
API_INIT(void, runtime_unhandled_exception_policy_set, (Il2CppRuntimeUnhandledExceptionPolicy value));
API_INIT(int32_t, string_length, (Il2CppString * str));
API_INIT(Il2CppChar*, string_chars, (Il2CppString * str));
API_INIT(Il2CppString*, string_new, (char const* str));
API_INIT(Il2CppString*, string_new_len, (char const* str, uint32_t length));
API_INIT(Il2CppString*, string_new_utf16, (Il2CppChar const* text, int32_t len));
API_INIT(Il2CppString*, string_new_wrapper, (char const* str));
API_INIT(Il2CppString*, string_intern, (Il2CppString * str));
API_INIT(Il2CppString*, string_is_interned, (Il2CppString * str));
API_INIT(Il2CppThread*, thread_current, ());
API_INIT(Il2CppThread*, thread_attach, (Il2CppDomain * domain));
API_INIT(void, thread_detach, (Il2CppThread * thread));
#if defined(UNITY_2019) || defined(UNITY_2021)
API_INIT(Il2CppThread**, thread_get_all_attached_threads, (size_t* size));
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
API_INIT(int, type_get_type, (Il2CppType const* type));
API_INIT(Il2CppClass*, type_get_class_or_element_class, (Il2CppType const* type));
API_INIT(char*, type_get_name, (Il2CppType const* type));
API_INIT(bool, type_is_byref, (Il2CppType const* type));
API_INIT(uint32_t, type_get_attrs, (Il2CppType const* type));
API_INIT(bool, type_equals, (Il2CppType const* type, Il2CppType const* otherType));
API_INIT(char*, type_get_assembly_qualified_name, (Il2CppType const* type));
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
API_INIT(bool, type_is_static, (const Il2CppType* type));
API_INIT(bool, type_is_pointer_type, (Il2CppType const* type));
#endif
API_INIT(const Il2CppAssembly*, image_get_assembly, (const Il2CppImage* image));
API_INIT(char const*, image_get_name, (Il2CppImage const* image));
API_INIT(char const*, image_get_filename, (Il2CppImage const* image));
API_INIT(MethodInfo const*, image_get_entry_point, (Il2CppImage const* image));
API_INIT(size_t, image_get_class_count, (Il2CppImage const* image));
API_INIT(Il2CppClass const*, image_get_class, (Il2CppImage const* image, size_t index));
API_INIT(Il2CppManagedMemorySnapshot*, capture_memory_snapshot, ());
API_INIT(void, free_captured_memory_snapshot, (Il2CppManagedMemorySnapshot * snapshot));
API_INIT(void, set_find_plugin_callback, (Il2CppSetFindPlugInCallback method));
API_INIT(void, register_log_callback, (Il2CppLogCallback method));
API_INIT(void, debugger_set_agent_options, (char const* options));
API_INIT(bool, is_debugger_attached, ());
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
API_INIT(void, register_debugger_agent_transport, (Il2CppDebuggerTransport * debuggerTransport));
API_INIT(bool, debug_get_method_info, (MethodInfo const*, Il2CppMethodDebugInfo* methodDebugInfo));
#endif
API_INIT(void, unity_install_unitytls_interface, (const void* unitytlsInterfaceStruct));
API_INIT(Il2CppCustomAttrInfo*, custom_attrs_from_class, (Il2CppClass * klass));
API_INIT(Il2CppCustomAttrInfo*, custom_attrs_from_method, (MethodInfo const* method));
API_INIT(Il2CppObject*, custom_attrs_get_attr, (Il2CppCustomAttrInfo * ainfo, Il2CppClass* attr_klass));
API_INIT(bool, custom_attrs_has_attr, (Il2CppCustomAttrInfo * ainfo, Il2CppClass* attr_klass));
API_INIT(Il2CppArray*, custom_attrs_construct, (Il2CppCustomAttrInfo * cinfo));
API_INIT(void, custom_attrs_free, (Il2CppCustomAttrInfo * ainfo));
#if defined(UNITY_2019) || defined(UNITY_2021) || defined(UNITY_6)
API_INIT(void, class_set_userdata, (Il2CppClass * klass, void* userdata));
API_INIT(int, class_get_userdata_offset, ());
#endif

// MANUALLY DEFINED CONST DEFINITIONS
API_INIT(Il2CppType const*, class_get_type_const, (Il2CppClass const* klass));
API_INIT(char const*, class_get_name_const, (Il2CppClass const* klass));
API_INIT(Il2CppClass*, type_get_class, (Il2CppType * type));

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

#if defined(UNITY_2019) || defined(UNITY_2021)
API_INIT(AssemblyVector*, Assembly_GetAllAssemblies, ());
#else
Il2CppAssemblyVector* i2c::functions::s_Assemblies = nullptr;
#endif

void const* i2c::functions::s_GlobalMetadata = nullptr;
Il2CppGlobalMetadataHeader const* i2c::functions::s_GlobalMetadataHeader = nullptr;
Il2CppMetadataRegistration const* i2c::functions::s_Il2CppMetadataRegistration = nullptr;

static decltype(i2c::functions::s_GlobalMetadata)* s_GlobalMetadataPtr = nullptr;
static decltype(i2c::functions::s_GlobalMetadataHeader)* s_GlobalMetadataHeaderPtr = nullptr;
static decltype(i2c::functions::s_Il2CppMetadataRegistration)* s_Il2CppMetadataRegistrationPtr = nullptr;

bool i2c::functions::has_gc_funcs = false;
Il2CppDefaults const* i2c::functions::defaults = nullptr;
bool i2c::functions::initialized = false;

void i2c::functions::CheckS_GlobalMetadata() {
    if (!s_GlobalMetadataHeader) {
        s_GlobalMetadata = *CRASH_UNLESS(s_GlobalMetadataPtr);
        s_GlobalMetadataHeader = *CRASH_UNLESS(s_GlobalMetadataHeaderPtr);
        s_Il2CppMetadataRegistration = *CRASH_UNLESS(s_Il2CppMetadataRegistrationPtr);
        i2c::logger.debug("sanity: {:X} (should be 0xFAB11BAF)", s_GlobalMetadataHeader->sanity);
        i2c::logger.debug("version: {}", s_GlobalMetadataHeader->version);
        CRASH_UNLESS((uint32_t) s_GlobalMetadataHeader->sanity == 0xFAB11BAF);
        i2c::logger.debug("typeDefinitionsOffset: {}", s_GlobalMetadataHeader->typeDefinitionsOffset);
        i2c::logger.debug("exportedTypeDefinitionsOffset: {}", s_GlobalMetadataHeader->exportedTypeDefinitionsOffset);
        i2c::logger.debug("nestedTypesOffset: {}", s_GlobalMetadataHeader->nestedTypesOffset);
        // TODO: use il2cpp_functions::defaults to define the il2cpp_defaults variable mentioned in il2cpp-class-internals.h
    }
}

// copies of the highly-inlinable functions
Il2CppTypeDefinition const* i2c::functions::MetadataCache_GetTypeDefinitionFromIndex(TypeDefinitionIndex index) {
    CheckS_GlobalMetadata();
    if (index == kTypeDefinitionIndexInvalid) {
        return NULL;
    }

    IL2CPP_ASSERT(index >= 0 && static_cast<uint32_t>(index) < s_GlobalMetadataHeader->typeDefinitionsCount / sizeof(Il2CppTypeDefinition));
    auto typeDefinitions = (Il2CppTypeDefinition const*) ((char const*) s_GlobalMetadata + s_GlobalMetadataHeader->typeDefinitionsOffset);
    return typeDefinitions + index;
}

char const* i2c::functions::MetadataCache_GetStringFromIndex(StringIndex index) {
    CheckS_GlobalMetadata();
    IL2CPP_ASSERT(index <= s_GlobalMetadataHeader->stringCount);
    char const* strings = ((char const*) s_GlobalMetadata + s_GlobalMetadataHeader->stringOffset) + index;
    return strings;
}

Il2CppGenericContainer const* i2c::functions::MetadataCache_GetGenericContainerFromIndex(GenericContainerIndex index) {
    CheckS_GlobalMetadata();
    if (index == kGenericContainerIndexInvalid) {
        return NULL;
    }

    IL2CPP_ASSERT(index >= 0 && static_cast<uint32_t>(index) <= s_GlobalMetadataHeader->genericContainersCount / sizeof(Il2CppGenericContainer));
    Il2CppGenericContainer const* genericContainers =
        (Il2CppGenericContainer const*) ((char const*) s_GlobalMetadata + s_GlobalMetadataHeader->genericContainersOffset);
    return genericContainers + index;
}

Il2CppGenericParameter const* i2c::functions::MetadataCache_GetGenericParameterFromIndex(GenericParameterIndex index) {
    CheckS_GlobalMetadata();
    if (index == kGenericParameterIndexInvalid) {
        return NULL;
    }

    IL2CPP_ASSERT(index >= 0 && static_cast<uint32_t>(index) <= s_GlobalMetadataHeader->genericParametersCount / sizeof(Il2CppGenericParameter));
    Il2CppGenericParameter const* genericParameters =
        (Il2CppGenericParameter const*) ((char const*) s_GlobalMetadata + s_GlobalMetadataHeader->genericParametersOffset);
    return genericParameters + index;
}

TypeDefinitionIndex i2c::functions::MetadataCache_GetExportedTypeFromIndex(TypeDefinitionIndex index) {
    CheckS_GlobalMetadata();
    if (index == kTypeDefinitionIndexInvalid) {
        return kTypeDefinitionIndexInvalid;
    }

    IL2CPP_ASSERT(index >= 0 && static_cast<uint32_t>(index) < s_GlobalMetadataHeader->exportedTypeDefinitionsCount / sizeof(TypeDefinitionIndex));
    auto exportedTypes = (TypeDefinitionIndex*) ((char const*) s_GlobalMetadata + s_GlobalMetadataHeader->exportedTypeDefinitionsOffset);
    return *(exportedTypes + index);
}

Il2CppClass* i2c::functions::MetadataCache_GetNestedTypeFromIndex(NestedTypeIndex index) {
    CheckS_GlobalMetadata();
    IL2CPP_ASSERT(index >= 0 && static_cast<uint32_t>(index) <= s_GlobalMetadataHeader->nestedTypesCount / sizeof(TypeDefinitionIndex));
    auto nestedTypeIndices = (TypeDefinitionIndex const*) ((char const*) s_GlobalMetadata + s_GlobalMetadataHeader->nestedTypesOffset);

    return i2c::functions::GlobalMetadata_GetTypeInfoFromTypeDefinitionIndex(nestedTypeIndices[index]);
}

TypeDefinitionIndex i2c::functions::MetadataCache_GetIndexForTypeDefinition(Il2CppTypeDefinition const* typeDefinition) {
    CheckS_GlobalMetadata();
    IL2CPP_ASSERT(klass);
    Il2CppTypeDefinition const* typeDefinitions =
        (Il2CppTypeDefinition const*) ((char const*) s_GlobalMetadata + s_GlobalMetadataHeader->typeDefinitionsOffset);

    IL2CPP_ASSERT(
        typeDefinition->typeDefinition >= typeDefinitions &&
        typeDefinition->typeDefinition < typeDefinitions + s_GlobalMetadataHeader->typeDefinitionsSize / sizeof(Il2CppTypeDefinition)
    );
    ptrdiff_t index = typeDefinition - typeDefinitions;
    IL2CPP_ASSERT(index <= std::numeric_limits<TypeDefinitionIndex>::max());
    return static_cast<TypeDefinitionIndex>(index);
}

TypeDefinitionIndex i2c::functions::MetadataCache_GetIndexForTypeDefinition(Il2CppClass const* klass) {
    CheckS_GlobalMetadata();
    return MetadataCache_GetIndexForTypeDefinition(reinterpret_cast<Il2CppTypeDefinition const*>(klass->typeMetadataHandle));
}

GenericParameterIndex i2c::functions::MetadataCache_GetGenericParameterIndexFromParameter(Il2CppMetadataGenericParameterHandle handle) {
    CheckS_GlobalMetadata();
    Il2CppGenericParameter const* genericParameter = reinterpret_cast<Il2CppGenericParameter const*>(handle);
    Il2CppGenericParameter const* genericParameters =
        (Il2CppGenericParameter const*) ((char const*) s_GlobalMetadata + s_GlobalMetadataHeader->genericParametersOffset);

    IL2CPP_ASSERT(
        genericParameter >= genericParameters &&
        genericParameter < genericParameters + s_GlobalMetadataHeader->genericParametersSize / sizeof(Il2CppGenericParameter)
    );

    ptrdiff_t index =
        reinterpret_cast<Il2CppGenericParameter const*>(genericParameter) - reinterpret_cast<Il2CppGenericParameter const*>(genericParameters);
    IL2CPP_ASSERT(index <= std::numeric_limits<GenericParameterIndex>::max());
    return static_cast<GenericParameterIndex>(index);
}

Il2CppTypeDefinition const* i2c::functions::MetadataCache_GetTypeDefinition(Il2CppClass* klass) {
    CheckS_GlobalMetadata();
    return reinterpret_cast<Il2CppTypeDefinition const*>(klass->typeMetadataHandle);
}

GenericParameterIndex i2c::functions::MetadataCache_GetGenericContainerIndex(Il2CppClass* klass) {
    CheckS_GlobalMetadata();
    auto td = MetadataCache_GetTypeDefinition(klass);
    if (td) {
        return td->genericContainerIndex;
    }
    return 0;
}

char* i2c::functions::Type_GetName(Il2CppType const* type, Il2CppTypeNameFormat format) {
    if (!_Type_GetName_) {
        return nullptr;
    }
    // TODO debug the ref/lifetime weirdness with _Type_GetName_ to avoid the need for explicit allocation
    auto const str = i2c::functions::_Type_GetName_(type, format);
    char* buffer = static_cast<char*>(i2c::functions::alloc(str.length() + 1));
    memcpy(buffer, str.c_str(), str.length() + 1);
    return buffer;
}

// static std::optional<uint32_t*> blrFind(cs_insn* insn) {
//     return insn->id == ARM64_INS_BLR ? std::optional<uint32_t*>(reinterpret_cast<uint32_t*>(insn->address)) : std::nullopt;
// }

// static std::optional<uint32_t*> findADRP(cs_insn* insn) {
//     return (insn->id == ARM64_INS_ADRP) ? std::optional<uint32_t*>(reinterpret_cast<uint32_t*>(insn->address)) : std::nullopt;
// }

static void find_GC_free(Paper::LoggerContext const& logger) {
    using namespace i2c::functions;
    static auto gc_free = cs::find_nth_b<1>(reinterpret_cast<uint32_t*>(gc_free_fixed));
    if (!gc_free) {
        SAFE_ABORT("Failed to find GC_free!");
    }

    GC_free = reinterpret_cast<decltype(GC_free)>(*gc_free);
    logger.debug("gc::GarbageCollector::FreeFixed found? offset: {:X}", reinterpret_cast<uintptr_t>(GC_free) - i2c::binary::get_real_offset(0));
}

// static bool find_GC_SetWriteBarrier(uint32_t const* set_wbarrier_field) {
//     using namespace i2c::functions;
//     if (!set_wbarrier_field) {
//         return false;
//     }
//     GarbageCollector_SetWriteBarrier = reinterpret_cast<decltype(GarbageCollector_SetWriteBarrier)>(*set_wbarrier_field);
//     return true;
// }

void* (*wrapped_gc_malloc_uncollectable)(size_t sz, long long type);

void* __wrapper_gc_malloc_uncollectable(size_t sz, [[maybe_unused]] void* desc) {
    // 2 determined from ghidra dump of caller
    return wrapped_gc_malloc_uncollectable(sz, 2);
}

static bool trace_GC_AllocFixed(uint32_t const* DomainGetCurrent) {
    using namespace i2c::functions;
    // Domain::GetCurrent has a single bl to GarbageCollector::AllocateFixed
    // MetadataCache::InitializeGCSafe is 3rd bl after first b.ne, which is the 6th b(.lt, .ne), t(bz, nz), c(bz, nz)
    auto tmp = cs::find_nth_bl<1>(DomainGetCurrent);
    if (!tmp) {
        return false;
    }
    GarbageCollector_AllocateFixed = reinterpret_cast<decltype(GarbageCollector_AllocateFixed)>(*tmp);
    return true;
}

static bool find_GC_AllocFixed() {
    using namespace i2c::functions;
    auto result = cs::find_nth_b<1>(reinterpret_cast<uint32_t const*>(domain_get));
    if (!result) {
        SAFE_ABORT("Failed to find Domain::Get!");
    }
    if (!trace_GC_AllocFixed(*result)) {
        bool multipleMatches;
        auto sigMatch = i2c::binary::libil2cpp_unique_pattern(
            multipleMatches,
            "f5 0f 1d f8 f4 4f 01 a9 fd 7b 02 a9"
            "fd 83 00 91 ?? ?? ?? ?? ?? ?? ?? ?? 1f 00 20 f1 f3 03 01 2a",
            "GC_Malloc_Uncollectable"
        );

        if (sigMatch && !multipleMatches) {
            // We need to make a wrapper method instead and set that.
            wrapped_gc_malloc_uncollectable = (decltype(wrapped_gc_malloc_uncollectable)) sigMatch;
            GarbageCollector_AllocateFixed = &__wrapper_gc_malloc_uncollectable;
        } else {
            return false;
        }
    }
    return true;
}

/*
 * XREF for Class::Init
 * - class_from_system_type (symbol)
 *   - 2nd bl instruction -> Class::Init
 */
static void find_class_init(Paper::LoggerContext const& logger) {
    using namespace i2c::functions;
#ifdef UNITY_6
    const auto class_init = cs::find_nth_bl<2>(reinterpret_cast<const uint32_t*>(class_from_system_type));
    if (!class_init) {
        SAFE_ABORT("Failed to find Class::Init!");
    }
    Class_Init = reinterpret_cast<decltype(Class_Init)>(*class_init);
    logger.debug("Class::Init found? offset: {:X}", reinterpret_cast<uintptr_t>(Class_Init) - i2c::binary::get_real_offset(0));
#else
#error "XREF for Class::Init needs to be updated for this Unity version!"
#endif
}

/*
 * XREF for Type::GetName
 * - type_get_assembly_qualified_name (symbol)
 *   - 1st Bl instruction -> Type::GetName
 */
static void find__type_get_name_(Paper::LoggerContext const& logger) {
    using namespace i2c::functions;
#ifdef UNITY_6
    auto type_getName = cs::find_nth_bl<1>(reinterpret_cast<uint32_t*>(type_get_assembly_qualified_name));
    if (!type_getName) {
        SAFE_ABORT("Failed to find Type::GetName!");
    }
    _Type_GetName_ = reinterpret_cast<decltype(_Type_GetName_)>(*type_getName);
    logger.debug("Type::GetName found? offset: {:X}", reinterpret_cast<uintptr_t>(_Type_GetName_) - i2c::binary::get_real_offset(0));
#else
#error "XREF for Type::GetName needs to be updated for this Unity version!"
#endif
}

/*
 * XREF for GlobalMetadata::GetTypeInfoFromTypeDefinitionIndex
 * - mono_type_get_class (symbol)
 *   - 1st B instruction -> GlobalMetadata::GetTypeInfoFromTypeDefinitionIndex
 */
static void find_get_type_info_from_type_definition_index() {
    using namespace i2c::functions;
#ifdef UNITY_6
    auto get_type_info_from_type_definition_index = cs::find_nth_b<1, false, -1, 1024>(reinterpret_cast<uint32_t*>(type_get_class));
    if (!get_type_info_from_type_definition_index) {
        SAFE_ABORT("Failed to find GlobalMetadata::GetTypeInfoFromTypeDefinitionIndex!");
    }
    GlobalMetadata_GetTypeInfoFromTypeDefinitionIndex =
        reinterpret_cast<decltype(GlobalMetadata_GetTypeInfoFromTypeDefinitionIndex)>(*get_type_info_from_type_definition_index);
#else
#error "XREF for GlobalMetadata::GetTypeInfoFromTypeDefinitionIndex needs to be updated for this Unity version!"
#endif
}

/*
 * XREF for GlobalMetadata::GetTypeInfoFromTypeDefinitionIndex
 * - class_from_il2cpp_type (symbol)
 *   - 1st B instruction -> Class::FromIl2CppType
 */
static void find_class_from_il2cpp_type(Paper::LoggerContext const& logger) {
    using namespace i2c::functions;
#ifdef UNITY_6
    const auto result = cs::find_nth_b<1, false, -1, 1024>(reinterpret_cast<uint32_t*>(class_from_il2cpp_type));
    if (!result) {
        SAFE_ABORT("Failed to find Class::FromIl2CppType!");
    }
    Class_FromIl2CppType = reinterpret_cast<decltype(Class_FromIl2CppType)>(*result);
    logger.debug("Class::FromIl2CppType found? offset: {:X}", reinterpret_cast<uintptr_t>(Class_FromIl2CppType) - i2c::binary::get_real_offset(0));
#else
#error "XREF for Class::FromIl2CppType needs to be updated for this Unity version!"
#endif
}

static Il2CppClass* type_info_from_handle(Il2CppMetadataTypeHandle handle) {
    using namespace i2c::functions;
#if defined(UNITY_6)
    Il2CppType _type = {};
    _type.data.typeHandle = handle;
    Il2CppType* type = &_type;
    return type_get_class(type);
#else
#error "XREF for type_info_from_handle needs to be updated for this Unity version!"
#endif
}

static void find_generic_class_create_class(Paper::LoggerContext const& logger) {
    using namespace i2c::functions;
#ifdef UNITY_6
    const auto GenericClass_CreateClass_ptr = cs::find_nth_b<18, false, -1, 1024>(reinterpret_cast<uint32_t*>(Class_FromIl2CppType));
    if (!GenericClass_CreateClass_ptr) {
        SAFE_ABORT("Failed to find GenericClass::CreateClass!");
    }
    GenericClass_CreateClass = reinterpret_cast<decltype(GenericClass_CreateClass)>(*GenericClass_CreateClass_ptr);
    logger.debug(
        "GenericClass::CreateClass found? offset: {:X}", reinterpret_cast<uintptr_t>(GenericClass_CreateClass) - i2c::binary::get_real_offset(0)
    );
#else
#error "XREF for GenericClass::CreateClass needs to be updated for this Unity version!"
#endif
}

/*
 * Original Function has become inlined, so recreate from decomp
 */
static Il2CppClass* generic_class_get_class(Il2CppGenericClass* gclass) {
    using namespace i2c::functions;
#ifdef UNITY_6
    if (gclass->cached_class) {
        return gclass->cached_class;
    }
    return GenericClass_CreateClass(gclass, true);
#else
#error "XREF for GenericClass::GetClass needs to be updated for this Unity version!"
#endif
}

/*
 * XREF for Class::GetPtrClass
 * - Class_FromIl2CppType (XREF_FOUND)
 *   - 6th B instruction -> GenericClass::GetClass
 */
static void find_class_get_ptr_class(Paper::LoggerContext const& logger) {
    using namespace i2c::functions;
#ifdef UNITY_6
    const auto Class_GetPtrClass_addr = cs::find_nth_b<17, false>(reinterpret_cast<uint32_t*>(Class_FromIl2CppType));
    if (!Class_GetPtrClass_addr) {
        SAFE_ABORT("Failed to find Class::GetPtrClass!");
    }
    Class_GetPtrClass = reinterpret_cast<decltype(Class_GetPtrClass)>(*Class_GetPtrClass_addr);
    logger.debug(
        "Class::GetPtrClass(Il2CppClass*) found? offset: {:X}", reinterpret_cast<uintptr_t>(Class_GetPtrClass) - i2c::binary::get_real_offset(0)
    );
#else
#error "XREF for Class::GetPtrClass needs to be updated for this Unity version!"
#endif
}

/*
 * XREF for il2cpp_defaults
 * - get_corlib (XREF_FOUND)
 *   - PC Address<1,1> -> defaults
 */
void i2c::functions::find_il2cpp_defaults(Paper::LoggerContext const& logger) {
#ifdef UNITY_6
    auto pcAddr = cs::getpcaddr<1, 1>(reinterpret_cast<uint32_t*>(get_corlib));
    defaults = reinterpret_cast<decltype(defaults)>(std::get<2>(*pcAddr));

    logger.debug("il2cpp_defaults found: {} (offset: {:X})", fmt::ptr(defaults), reinterpret_cast<uintptr_t>(defaults) - binary::get_real_offset(0));
#else
#error "XREF for il2cpp_defaults needs to be updated for this Unity version!"
#endif
}

#if !defined(UNITY_2019) && !defined(UNITY_2021)

void i2c::functions::find_s_Assemblies(Paper::LoggerContext const& logger) {
#ifdef UNITY_6
    auto Assembly_GetImage = cs::find_nth_b<1>(reinterpret_cast<const uint32_t*>(domain_assembly_open));
    auto pcAddr = cs::getpcaddr<1, 1>(*Assembly_GetImage);
    s_Assemblies = reinterpret_cast<decltype(s_Assemblies)>(std::get<2>(*pcAddr));

    logger.debug(
        "s_Assemblies found: {} (offset: {:X})", fmt::ptr(s_Assemblies), reinterpret_cast<uintptr_t>(s_Assemblies) - binary::get_real_offset(0)
    );
#else
#error "XREF for s_Assemblies needs to be updated for this Unity version!"
#endif
}

Il2CppAssemblyVector* i2c::functions::Assembly_GetAllAssemblies() {
    return s_Assemblies;
}
#endif

struct nullable {
    char const* str;
    explicit constexpr nullable(char const* s) : str(s) {}
};

inline char const* format_as(nullable n) {
    return n.str ? n.str : "(null)";
}

#define API_SYM(name)                                                \
*(void**)(&i2c::functions::name) = dlsym(imagehandle, "il2cpp_" #name); \
logger.debug("Loaded: " #name ", error: {}", nullable(dlerror()))

static void init_api(Paper::LoggerContext const& logger, void* imagehandle) {
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
    API_SYM(class_from_type);
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

static void log_offset(Paper::LoggerContext const& logger, char const* name, uintptr_t ptr) {
    logger.debug("{}: offset {:X}", name, ptr - i2c::binary::get_real_offset(0));
}

// Autogenerated
// Initializes all of the IL2CPP functions via dlopen and dlsym for use.
void i2c::functions::initialize() {
    if (initialized) {
        return;
    }

    // Register for paper to write file
    Paper::Logger::RegisterFileContextId(logger.tag);

    logger.info("i2c::functions: Init: Initializing all IL2CPP Functions...");
    dlerror();  // clears existing errors

    void* imagehandle = modloader_libil2cpp_handle;

    if (!imagehandle) {
        logger.error("Failed to grab modloader libil2cpp.so handle: {}", fmt::ptr(imagehandle));
        return;
    }

    init_api(logger, imagehandle);

    // MANUALLY DEFINED CONST DEFINITIONS
    *(void**) (&class_get_type_const) = dlsym(imagehandle, "il2cpp_class_get_type");
    logger.info("Loaded: class_get_type CONST VERSION!");
    *(void**) (&class_get_name_const) = dlsym(imagehandle, "il2cpp_class_get_name");
    logger.info("Loaded: class_get_name CONST VERSION!");
    *(void**) (&type_get_class) = dlsym(imagehandle, "mono_type_get_class");
    logger.info("Loaded: type_get_class");

    find_class_init(logger);

    MetadataCache_GetTypeInfoFromHandle = type_info_from_handle;
    GlobalMetadata_GetTypeInfoFromHandle = type_info_from_handle;
    find_get_type_info_from_type_definition_index();

    find__type_get_name_(logger);
    find_class_from_il2cpp_type(logger);
    find_generic_class_create_class(logger);
    GenericClass_GetClass = generic_class_get_class;
    find_class_get_ptr_class(logger);
    find_s_Assemblies(logger);

    auto get_type_info_from_type_definition_index = cs::find_nth_b<1, false, -1, 1024>(reinterpret_cast<uint32_t*>(type_get_class));
    if (!get_type_info_from_type_definition_index) {
        SAFE_ABORT("Failed to find GlobalMetadata::GetTypeInfoFromTypeDefinitionIndex!");
    }
    GlobalMetadata_GetTypeInfoFromTypeDefinitionIndex =
        reinterpret_cast<decltype(GlobalMetadata_GetTypeInfoFromTypeDefinitionIndex)>(*get_type_info_from_type_definition_index);

#if defined(UNITY_2019) || defined(UNITY_2021)
    {
        // Assembly::GetAllAssemblies
        auto result = cs::find_nth_bl<1>(reinterpret_cast<uint32_t const*>(domain_get_assemblies));
        if (!result) {
            SAFE_ABORT_MSG("Failed to find Assembly::GetAllAssemblies!");
        }
        Assembly_GetAllAssemblies = reinterpret_cast<decltype(Assembly_GetAllAssemblies)>(*result);
        logger.debug("Assembly::GetAllAssemblies found? offset: {:X}", reinterpret_cast<uintptr_t>(Assembly_GetAllAssemblies) - getRealOffset(0));
    }
#endif

    CRASH_UNLESS(shutdown);
    // GC_free
    find_GC_free(logger);

    // GarbageCollector::SetWriteBarrier(void*)
    /*if (find_GC_SetWriteBarrier(reinterpret_cast<const uint32_t*>())) {
        logger.debug("GarbageCollector::SetWriteBarrier found? offset: {:X}", reinterpret_cast<uintptr_t>(GarbageCollector_SetWriteBarrier) -
    getRealOffset(0));
    }*/

    // GarbageCollector::AllocateFixed(size_t, void*)
    if (find_GC_AllocFixed()) {
        logger.debug(
            "GarbageCollector::AllocateFixed found? offset: {:X}",
            reinterpret_cast<uintptr_t>(GarbageCollector_AllocateFixed) - binary::get_real_offset(0)
        );
    }

    has_gc_funcs = GarbageCollector_AllocateFixed != nullptr && GC_free != nullptr;

    find_il2cpp_defaults(logger);
    {
        // FIELDS
        // Extract locations of s_GlobalMetadataHeader, s_Il2CppMetadataRegistration, & s_GlobalMetadata

        auto tmp = cs::getpcaddr<3, 1>(reinterpret_cast<uint32_t const*>(GlobalMetadata_GetTypeInfoFromTypeDefinitionIndex));
        if (!tmp) {
            SAFE_ABORT("Failed to find 3rd pcaddr for s_GlobalMetadataHeaderPtr!");
        }
        s_GlobalMetadataHeaderPtr = reinterpret_cast<decltype(s_GlobalMetadataHeaderPtr)>(std::get<2>(*tmp));

        tmp = cs::getpcaddr<5, 1>(reinterpret_cast<uint32_t const*>(GlobalMetadata_GetTypeInfoFromTypeDefinitionIndex));
        if (!tmp) {
            SAFE_ABORT("Failed to find 4th pcaddr for s_Il2CppMetadataRegistrationPtr!");
        }
        s_Il2CppMetadataRegistrationPtr = reinterpret_cast<decltype(s_Il2CppMetadataRegistrationPtr)>(std::get<2>(*tmp));

        tmp = cs::getpcaddr<4, 1>(reinterpret_cast<uint32_t const*>(GlobalMetadata_GetTypeInfoFromTypeDefinitionIndex));
        if (!tmp) {
            SAFE_ABORT("Failed to find 5th pcaddr for s_GlobalMetadataPtr!");
        }
        s_GlobalMetadataPtr = reinterpret_cast<decltype(s_GlobalMetadataPtr)>(std::get<2>(*tmp));

        logger.debug(
            "{:X} {:X} {:X} metadata pointers offsets",
            (uintptr_t) s_GlobalMetadataHeaderPtr - binary::get_real_offset(0),
            (uintptr_t) s_Il2CppMetadataRegistrationPtr - binary::get_real_offset(0),
            (uintptr_t) s_GlobalMetadataPtr - binary::get_real_offset(0)
        );
        logger.debug("All global constants found!");
    }

    logger.debug("i2c::functions: Printing offsets");
    log_offset(logger, "Class::Init", reinterpret_cast<uintptr_t>(Class_Init));
    log_offset(
        logger, "GlobalMetadata::GetTypeInfoFromTypeDefinitionIndex", reinterpret_cast<uintptr_t>(GlobalMetadata_GetTypeInfoFromTypeDefinitionIndex)
    );
    log_offset(logger, "Type::GetName", reinterpret_cast<uintptr_t>(_Type_GetName_));
    log_offset(logger, "Class::FromIl2cppType", reinterpret_cast<uintptr_t>(Class_FromIl2CppType));
    log_offset(logger, "GenericClass::GetClass", reinterpret_cast<uintptr_t>(GenericClass_GetClass));
    log_offset(logger, "Class::GetPtrClass", reinterpret_cast<uintptr_t>(Class_GetPtrClass));
    log_offset(logger, "s_GlobalMetadataHeaderPtr", reinterpret_cast<uintptr_t>(s_GlobalMetadataHeaderPtr));
    log_offset(logger, "s_Il2CppMetadataRegistrationPtr", reinterpret_cast<uintptr_t>(s_Il2CppMetadataRegistrationPtr));
    log_offset(logger, "s_GlobalMetadataPtr", reinterpret_cast<uintptr_t>(s_GlobalMetadataPtr));
    log_offset(logger, "GC::Free", reinterpret_cast<uintptr_t>(GC_free));
    log_offset(logger, "GarbageCollector::SetWriteBarrier", reinterpret_cast<uintptr_t>(GarbageCollector_SetWriteBarrier));
    log_offset(logger, "GarbageCollector::AllocateFixed", reinterpret_cast<uintptr_t>(GarbageCollector_AllocateFixed));
    initialized = true;
    logger.info("i2c::functions: initialize: Successfully loaded all il2cpp functions!");
}

// static void find_class_init(Paper::LoggerContext const& logger);
// // MetadataCache_GetTypeInfoFromHandle = type_info_from_handle
// // MetadataCache_GetTypeInfoFromTypeIndex = null I think
// static void find_get_type_info_from_type_definition_index();
// // GlobalMetadata_GetTypeInfoFromHandle = type_info_from_handle
// static void find__type_get_name_(Paper::LoggerContext const& logger);
// static void find_GC_free(Paper::LoggerContext const& logger);
// static bool find_GC_SetWriteBarrier(uint32_t const* set_wbarrier_field);
// static bool trace_GC_AllocFixed(uint32_t const* DomainGetCurrent);
// static bool find_GC_AllocFixed();
// static void find_class_from_il2cpp_type(Paper::LoggerContext const& logger);
// static void find_class_get_ptr_class(Paper::LoggerContext const& logger);
// // GenericClass_GetClass = generic_class_get_class
// static void find_generic_class_create_class(Paper::LoggerContext const& logger);

// static Il2CppClass* type_info_from_handle(Il2CppMetadataTypeHandle handle);
// static Il2CppClass* generic_class_get_class(Il2CppGenericClass* gclass);

// static void init_api(Paper::LoggerContext const& logger, void* imagehandle);

[[nodiscard]] void* i2c::gc_alloc_specific(size_t sz) {
    // This function assumes il2cpp_functions will be called at a reasonable time, instead will warn you on allocating unsafe memory.
    if (functions::has_gc_funcs) {
        // We should absolutely panic if we thought we had the allocation function, but it gave us null.
        auto ptr = CRASH_UNLESS(functions::GarbageCollector_AllocateFixed(sz, nullptr));
        return ptr;
    } else {
        auto ptr = calloc(1, sz);
        logger.warn("Allocation at: {} for size: {} fallback to calloc!", fmt::ptr(ptr), sz);
        return ptr;
    }
}

void i2c::gc_free_specific(void* ptr) noexcept {
    // TODO: Also check if a GC free would have been valid?
    if (functions::has_gc_funcs) {
        functions::GC_free(ptr);
    } else {
        free(ptr);
    }
}

[[nodiscard]] void* i2c::gc_realloc_specific(void* ptr, size_t new_size) {
    auto new_ptr = gc_alloc_specific(new_size);
    memcpy(new_ptr, ptr, new_size);
    gc_free_specific(ptr);
    return new_ptr;
}

void* i2c::__allocate_unsafe(size_t size) {
    functions::initialize();
    // Because we want to allocate this object using C# GC, we will do a bit of a hack here.
    // Essentially, we take advantage of the instance size of System.Object, and then IMMEDIATELY revert it.
    // If we fail for ANY REASON in here, VERY BAD THINGS can happen.
    static auto object_class = CRASH_UNLESS(functions::defaults->object_class);
    // Ideally, we make this atomic, but because we aren't using locks anywhere, we hope for the best...
    // TODO: Acquire object class special lock
    auto original_size = object_class->instance_size;
    object_class->instance_size = static_cast<decltype(original_size)>(size);
    auto instance = functions::object_new(object_class);
    object_class->instance_size = original_size;
    return instance;
}
