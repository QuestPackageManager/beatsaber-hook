# include some defines automatically made by qpm
include(qpm_defines.cmake)

cmake_minimum_required(VERSION 3.22)
project(${COMPILE_ID})

# c++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

# define that stores the actual source directory
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# compile options used
add_compile_options(-frtti -fPIE -fPIC -fexceptions)
add_compile_options(-Wall -Wextra -Werror -Wno-unused-function)
# compile definitions used
add_compile_definitions(VERSION=\"${MOD_VERSION}\")
add_compile_definitions(ID=\"${MOD_ID}\")
add_compile_definitions(UNITY_2019)


if (DEFINED TEST_BUILD)
    MESSAGE(STATUS "Compiling with test defines")
    add_compile_definitions(TEST_CALLBACKS)
    add_compile_definitions(TEST_SAFEPTR)
    add_compile_definitions(TEST_BYREF)
    add_compile_definitions(TEST_ARRAY)
    add_compile_definitions(TEST_LIST)
    add_compile_definitions(TEST_STRING)
endif()

add_library(
    ${COMPILE_ID} 
    SHARED 
)

# recursively get all src files
RECURSE_FILES(cpp_file_list_utils ${SOURCE_DIR}/utils/*.cpp)
RECURSE_FILES(c_file_list_utils ${SOURCE_DIR}/utils/*.c)
target_sources(${COMPILE_ID} PRIVATE ${cpp_file_list_utils})
target_sources(${COMPILE_ID} PRIVATE ${c_file_list_utils})

RECURSE_FILES(cpp_file_list_config ${SOURCE_DIR}/config/*.cpp)
RECURSE_FILES(c_file_list_config ${SOURCE_DIR}/config/*.c)
target_sources(${COMPILE_ID} PRIVATE ${cpp_file_list_config})
target_sources(${COMPILE_ID} PRIVATE ${c_file_list_config})

if (DEFINED TEST_BUILD)
RECURSE_FILES(cpp_file_list_tests ${SOURCE_DIR}/tests/*.cpp)
RECURSE_FILES(c_file_list_tests ${SOURCE_DIR}/tests/*.c)
target_sources(${COMPILE_ID} PRIVATE ${cpp_file_list_tests})
target_sources(${COMPILE_ID} PRIVATE ${c_file_list_tests})
endif()

# add root dir as include dir
target_include_directories(${COMPILE_ID} PRIVATE ${CMAKE_SOURCE_DIR})
# add src dir as include dir
target_include_directories(${COMPILE_ID} PRIVATE ${SOURCE_DIR})
# add include dir as include dir
target_include_directories(${COMPILE_ID} PRIVATE ${INCLUDE_DIR})
# add shared dir as include dir
target_include_directories(${COMPILE_ID} PUBLIC ${SHARED_DIR})

target_link_libraries(${COMPILE_ID} PRIVATE -llog)
# add extern stuff like libs and other includes
include(extern.cmake)

add_custom_command(TARGET ${COMPILE_ID} POST_BUILD
            COMMAND ${CMAKE_STRIP} -g -S -d --strip-all
            "lib${COMPILE_ID}.so" -o "stripped_lib${COMPILE_ID}.so"
            COMMENT "Strip debug symbols done on final binary.")

add_custom_command(TARGET ${COMPILE_ID} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E rename lib${COMPILE_ID}.so debug_lib${COMPILE_ID}.so
            COMMENT "Rename the lib to debug_ since it has debug symbols"
            )

add_custom_command(TARGET ${COMPILE_ID} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E rename stripped_lib${COMPILE_ID}.so lib${COMPILE_ID}.so
            COMMENT "Rename the stripped lib to regular"
            )