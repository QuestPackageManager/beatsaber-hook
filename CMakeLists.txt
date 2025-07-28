# include some defines automatically made by qpm
include(qpm_defines.cmake)

cmake_minimum_required(VERSION 3.22)
project(${COMPILE_ID})

# c++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)
# LTO
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# define that stores the actual source directory
set(SHARED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shared)
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# compile options used
add_compile_options(-frtti -fPIE -fPIC -fexceptions -flto)
add_link_options(-Wl,--exclude-libs,ALL)
add_compile_options(-Wall -Wextra -Werror -Wno-unused-function -Wno-extra-qualification)
# clangd bug https://github.com/clangd/clangd/issues/1167
add_compile_options(-Wno-pragma-pack)
add_compile_options(-Wno-vla-cxx-extension)
# compile definitions used
add_compile_definitions(VERSION=\"${MOD_VERSION}\")
add_compile_definitions(MOD_ID=\"${MOD_ID}\")
add_compile_definitions(VERSION_NUMBER=300150000)
add_compile_definitions(UNITY_6)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

function(setup_target target add_test)
message(STATUS "Setting up target ${target}")

string(LENGTH "${CMAKE_CURRENT_LIST_DIR}" FOLDER_LENGTH)
add_compile_definitions("PAPER_ROOT_FOLDER_LENGTH=(${FOLDER_LENGTH} + 1)")

add_library(
    ${target}
    SHARED
    )
    
    if (add_test)
        target_compile_definitions(
            ${target}
            PRIVATE

            TEST_CALLBACKS
            TEST_BETTER_SPAN
            TEST_SAFEPTR
            TEST_BYREF
            TEST_ARRAY
            TEST_LIST
            TEST_STRING
            TEST_HOOK
            TEST_THREAD
            TEST_UNITYW
        )
    endif()

    # recursively get all src files
    RECURSE_FILES(cpp_file_list_utils ${SOURCE_DIR}/utils/*.cpp)
    RECURSE_FILES(c_file_list_utils ${SOURCE_DIR}/utils/*.c)
    target_sources(${target} PRIVATE ${cpp_file_list_utils})
    target_sources(${target} PRIVATE ${c_file_list_utils})

    # recursively get all src files
    RECURSE_FILES(cpp_file_list_hook ${SHARED_DIR}/inline-hook/*.cpp)
    RECURSE_FILES(c_file_list_hook ${SHARED_DIR}/inline-hook/*.c)
    target_sources(${target} PRIVATE ${cpp_file_list_hook})
    target_sources(${target} PRIVATE ${c_file_list_hook})

    RECURSE_FILES(cpp_file_list_config ${SOURCE_DIR}/config/*.cpp)
    RECURSE_FILES(c_file_list_config ${SOURCE_DIR}/config/*.c)
    target_sources(${target} PRIVATE ${cpp_file_list_config})
    target_sources(${target} PRIVATE ${c_file_list_config})

    if (add_test)
        message(STATUS "Adding tests source files to target ${target}")
        RECURSE_FILES(cpp_file_list_tests ${SOURCE_DIR}/tests/*.cpp)
        RECURSE_FILES(c_file_list_tests ${SOURCE_DIR}/tests/*.c)
        target_sources(${target} PRIVATE ${cpp_file_list_tests})
        target_sources(${target} PRIVATE ${c_file_list_tests})
    endif()

    # add root dir as include dir
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR})
    # add src dir as include dir
    target_include_directories(${target} PRIVATE ${SOURCE_DIR})
    # add include dir as include dir
    target_include_directories(${target} PRIVATE ${INCLUDE_DIR})
    # add shared dir as include dir
    target_include_directories(${target} PUBLIC ${SHARED_DIR})

    target_include_directories(${target} SYSTEM PRIVATE ${EXTERN_DIR}/includes/libil2cpp/il2cpp/external/baselib/Include)
    target_include_directories(${target} SYSTEM PRIVATE ${EXTERN_DIR}/includes/libil2cpp/il2cpp/external/baselib/Platforms/Android/Include)

    target_include_directories(${target} SYSTEM PRIVATE ${EXTERN_DIR}/includes/fmt/fmt/include/)
    target_compile_options(${target} PRIVATE -DFMT_HEADER_ONLY)

    target_link_directories(${target} PRIVATE ${EXTERN_DIR}/libs)
    target_link_libraries(${target} PRIVATE -llog)
    target_link_options(${target} PRIVATE "-fuse-ld=ld")

    file(GLOB_RECURSE so_libs ${EXTERN_DIR}/libs/*.so)
    file(GLOB_RECURSE a_libs ${EXTERN_DIR}/libs/*.a)

    target_link_libraries(
        ${target}
        PRIVATE
        ${so_libs}
        ${a_libs}
    )

    target_include_directories(${target} PRIVATE ${EXTERN_DIR}/includes)
    target_include_directories(${target} SYSTEM PRIVATE ${EXTERN_DIR}/includes/libil2cpp/il2cpp/libil2cpp)
endfunction()

# compile test as one project
setup_target(test-${COMPILE_ID} YES)
# regular bs hook
setup_target(${COMPILE_ID} NO)

add_dependencies(${COMPILE_ID} test-${COMPILE_ID})

target_compile_definitions(
    ${COMPILE_ID}
    PRIVATE
    NO_TEST
)

# make bs hook build depend on the build of bs hook tests
# add_dependencies(${COMPILE_ID} ${COMPILE_ID}-test)

add_custom_command(TARGET ${COMPILE_ID} POST_BUILD
            COMMAND ${CMAKE_STRIP} -g -S -d --strip-all
            "lib${COMPILE_ID}.so" -o "stripped_lib${COMPILE_ID}.so"
            COMMENT "Strip debug symbols done on final binary.")

add_custom_command(TARGET ${COMPILE_ID} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory debug
            COMMENT "Create the debug dir"
)
add_custom_command(TARGET ${COMPILE_ID} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E rename lib${COMPILE_ID}.so debug/lib${COMPILE_ID}.so
            COMMENT "Rename the lib to debug_ since it has debug symbols"
            )

add_custom_command(TARGET ${COMPILE_ID} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E rename stripped_lib${COMPILE_ID}.so lib${COMPILE_ID}.so
            COMMENT "Rename the stripped lib to regular"
            )
