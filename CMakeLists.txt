cmake_minimum_required (VERSION 2.9)

project (jsonlib)

# This version number.
set (JSONLIB_VERSION_MAJOR 0)
set (JSONLIB_VERSION_MINOR 2)

# json library can be built to contain the minimum useful set of features.
# The core component that is required in all configurations is jsonvalue.
# The following optional components are defined:
# - JSON_PARSER creates a json object tree from the text passed as argument
# - JSON_EDITOR create json values, insert them or replace existing values,
#               remove values
# - JSON_SERIALISER serialize a json object tree into an output buffer or
#                   output it to a file (json_print).

option (JSON_PARSER     "Generate json library with parser" ON)
option (JSON_EDITOR     "Generate json library with data manipulation" ON)
option (JSON_SERIALIZER "Generate json library with serializer" ON)

# Uncomment the following 2 lines to print the actual options
#MESSAGE( STATUS "Debug: JSON_PARSER=" ${JSON_PARSER}
# " JSON_EDITOR=" ${JSON_EDITOR} " JSON_SERIALIZER=" ${JSON_SERIALIZER})

# The core component can be compiled with or without the option
# JSON_FAST_ACCESS_LARGER_CODE. This allows generating the smallest library
# size for a minimum parser & object tree access (about 10k on 64-bit x86)
# The complete package takes about 21k on 64-bit x86, which might not justify
# the choice of a slower random access to object members or array elements.

# JSON_EDITTOR requires the option JSON_FAST_ACCESS_LARGER_CODE by default.
# JSON_SERIALIZER does not require JSON_FAST_ACCESS_LARGER_CODE.

option (FAST_N_LARGER
        "Generate faster and larger json library" ON)

if (JSON_EDITOR)
    set (FAST_N_LARGER ON)
endif (JSON_EDITOR)

#uncomment the following line to print the actual FAST_N_LARGER option
#MESSAGE( STATUS "FAST_N_LARGER=" ${FAST_N_LARGER})

if (FAST_N_LARGER)
    set (CMAKE_C_FLAGS "-g -Wall -Wextra -pedantic -std=c99 -D_JSON_FAST_ACCESS_LARGER_CODE -D_POSIX_C_SOURCE=200809L")
else ()
    set (CMAKE_C_FLAGS "-g -Wall -Wextra -pedantic -std=c99 -D_POSIX_C_SOURCE=200809L")
endif (FAST_N_LARGER)

#uncomment the following line to enable internal asserts during debugging
# note that calling APIs with wrong args will likely trigger an assert.
# do not use with unit testing that may do negative testing on API.
# and usually do not keep it enabled once debugging is completed
set (JSONLIB_DEBUG " -D_JSON_DEBUG")

set (CMAKE_C_FLAGS ${CMAKE_C_FLAGS}${JSONLIB_DEBUG})

set (SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
include_directories(. ${SOURCE_DIR})
set (TEST_DIR ${PROJECT_SOURCE_DIR}/test)
set (CHECK_DIR ${PROJECT_SOURCE_DIR}/test/JSON_checker)
set (ARRAYS_DIR ${PROJECT_SOURCE_DIR}/test/arrays)
set (DICTS_DIR ${PROJECT_SOURCE_DIR}/test/dictionaries)
set (CONFORM_DIR ${PROJECT_SOURCE_DIR}/test/test_parsing)

MESSAGE ( STATUS "gcc: " ${CMAKE_C_FLAGS})

if (JSON_PARSER)
    set (EXTRA_COMPONENTS ${EXTRA_COMPONENTS} ${SOURCE_DIR}/jsonparse.c)
endif (JSON_PARSER)
if (JSON_EDITOR)
    set (EXTRA_COMPONENTS ${EXTRA_COMPONENTS} ${SOURCE_DIR}/jsonedit.c)
endif (JSON_EDITOR)
if (JSON_SERIALIZER)
    set (EXTRA_COMPONENTS ${EXTRA_COMPONENTS} ${SOURCE_DIR}/jsonserial.c)
endif (JSON_SERIALIZER)

add_library(jsonlib     ${SOURCE_DIR}/jsonvalue.c ${SOURCE_DIR}/jsonutf8.c ${EXTRA_COMPONENTS})
add_executable(jsonc    ${SOURCE_DIR}/jsonc.c)
add_executable(utest    ${TEST_DIR}/utest.c)
add_executable(check    ${CHECK_DIR}/test_driver.c)
add_executable(arrays   ${ARRAYS_DIR}/test_driver.c)
add_executable(dicts    ${DICTS_DIR}/test_driver.c)
add_executable(conform  ${CONFORM_DIR}/test_driver.c)

target_link_libraries (jsonc jsonlib)
target_link_libraries (utest jsonlib)
target_link_libraries (check jsonlib)
target_link_libraries (arrays jsonlib)
target_link_libraries (dicts jsonlib)
target_link_libraries (conform jsonlib)
