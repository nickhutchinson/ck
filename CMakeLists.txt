cmake_minimum_required(VERSION 3.26)

project(ck VERSION 0.7.2 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

if(NOT DEFINED CMAKE_POSITION_INDEPENDENT_CODE)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

set(CK_CFLAGS_GNU
    -pedantic
    -Wall
    -W
    -Wundef
    -Wendif-labels
    -Wshadow
    -Wpointer-arith
    -Wcast-align
    -Wcast-qual
    -Wwrite-strings
    -Wstrict-prototypes
    -Wmissing-prototypes
    -Wnested-externs
    -Winline
    -Wdisabled-optimization
    -Wno-parentheses

    -fstrict-aliasing
    -pipe
)

# Clang takes most of the GCC flags. But we don't want strict aliasing when targeting the MSVC ABI
# -- Windows SDK headers violate TBAA. (Clang-cl already default-disables strict aliasing when
# invoked via the clang-cl driver, but not via the GNU-style driver, so we forcibly disable strict
# aliasing.)
set(CK_CFLAGS_CLANG ${CK_CFLAGS_GNU})
if(MSVC_VERSION)
    list(REMOVE_ITEM CK_CFLAGS_CLANG -fstrict-aliasing)
    list(APPEND CK_CFLAGS_CLANG -fno-strict-aliasing)
endif()

set(CK_CFLAGS_MSVC
    /W4
    /wd4100  # warning C4100: '<param>': unreferenced parameter
    /wd4146  # warning C4146: unary minus operator applied to unsigned type, result still unsigned
    /wd4324  # warning C4324: '<type>': structure was padded due to alignment specifier
    /wd4702  # warning C4702: unreachable code

    /permissive-
)

# Qualify GNU-style options for the clang-cl driver -- not all are accepted directly by clang-cl,
# and others have different meanings (e.g. -Wall).
list(TRANSFORM CK_CFLAGS_CLANG PREPEND "/clang:" OUTPUT_VARIABLE CK_CFLAGS_CLANGCL)

if(MSVC AND CMAKE_C_COMPILER_ID MATCHES "Clang")
    add_compile_options(${CK_CFLAGS_CLANGCL})
elseif(MSVC)
    add_compile_options(${CK_CFLAGS_MSVC})
elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
    add_compile_options(${CK_CFLAGS_CLANG})
else()
    add_compile_options(${CK_CFLAGS_GNU})
endif()

if(NOT MSVC_VERSION)
    add_compile_definitions(_BSD_SOURCE _DEFAULT_SOURCE)

    if(CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
        add_compile_definitions(_NETBSD_SOURCE)
    endif()
    if(NOT BSD AND NOT APPLE)
        add_compile_definitions(_XOPEN_SOURCE=600)
    endif()
    if(MINGW OR CYGWIN)
        add_link_options(-mthreads)
    endif()
endif()

if(WIN32)
    add_compile_definitions(_CRT_NONSTDC_NO_WARNINGS _CRT_SECURE_NO_WARNINGS)
endif()

# Determine the effective target processor -- CMAKE_SYSTEM_PROCESSOR may reflect the host system
# and not the target.
if(APPLE AND CMAKE_OSX_ARCHITECTURES)
    # Disallow specifying multiple architectures when building for Apple platforms -- ck_md.h bakes
    # in a single architecture.
    list(LENGTH CMAKE_OSX_ARCHITECTURES _ck_apple_arch_count)
    if(NOT _ck_apple_arch_count EQUAL 1)
        message(FATAL_ERROR "Universal builds are unsupported")
    endif()
    string(TOLOWER "${CMAKE_OSX_ARCHITECTURES}" CK_SYSTEM_PROCESSOR)
elseif(CMAKE_CROSSCOMPILING)
    # Honour CMAKE_SYSTEM_PROCESSOR if was explicitly set.
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" CK_SYSTEM_PROCESSOR)
elseif(CMAKE_C_COMPILER_ARCHITECTURE_ID)
    # Historically set only for MSVC, but set for other compilers from CMake 4.1.
    string(TOLOWER "${CMAKE_C_COMPILER_ARCHITECTURE_ID}" CK_SYSTEM_PROCESSOR)
else()
    # Fallback.
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" CK_SYSTEM_PROCESSOR)
endif()

set(CK_PLATFORM unknown)
set(CK_DEFAULT_MEMORY_MODEL RMO)
set(CK_COMPAT_DEFINITIONS)

if(CK_SYSTEM_PROCESSOR MATCHES "^(i[3-6]86|x86)$")
    set(CK_PLATFORM x86)
    set(CK_DEFAULT_MEMORY_MODEL TSO)
    list(APPEND CK_COMPAT_DEFINITIONS __x86__)
elseif(CK_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|x64)$")
    set(CK_PLATFORM x86_64)
    set(CK_DEFAULT_MEMORY_MODEL TSO)
elseif(CK_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64)$")
    set(CK_PLATFORM aarch64)
elseif(CK_SYSTEM_PROCESSOR MATCHES "^arm")
    set(CK_PLATFORM arm)
elseif(CK_SYSTEM_PROCESSOR MATCHES "^(ppc64|ppc64le|powerpc64|powerpc64le)$")
    set(CK_PLATFORM ppc64)
    list(APPEND CK_COMPAT_DEFINITIONS __ppc64__)
elseif(CK_SYSTEM_PROCESSOR MATCHES "^(ppc|powerpc|powerpcspe|macppc)$")
    set(CK_PLATFORM ppc)
    list(APPEND CK_COMPAT_DEFINITIONS __ppc__)
elseif(CK_SYSTEM_PROCESSOR MATCHES "^(sparcv9|sparc64|sun4u|sun4v)$")
    set(CK_PLATFORM sparcv9)
    set(CK_DEFAULT_MEMORY_MODEL TSO)
    list(APPEND CK_COMPAT_DEFINITIONS __sparcv9__)
elseif(CK_SYSTEM_PROCESSOR STREQUAL "s390x")
    set(CK_PLATFORM s390x)
elseif(CK_SYSTEM_PROCESSOR STREQUAL "riscv64")
    set(CK_PLATFORM riscv64)
elseif(CK_SYSTEM_PROCESSOR MATCHES "^(riscv32|riscv)$")
    set(CK_PLATFORM riscv32)
elseif(CK_SYSTEM_PROCESSOR STREQUAL "loongarch64")
    set(CK_PLATFORM loongarch64)
endif()

set(CK_MEMORY_MODEL "${CK_DEFAULT_MEMORY_MODEL}" CACHE STRING "Memory model: TSO, PSO, or RMO")
if(NOT CK_MEMORY_MODEL MATCHES "^(TSO|PSO|RMO)$")
    message(FATAL_ERROR "CK_MEMORY_MODEL must be TSO, PSO, or RMO")
endif()

option(CK_ENABLE_POINTER_PACKING "Enable pointer-packing optimizations" OFF)
option(CK_ENABLE_PPC32_LWSYNC "Enable lwsync on 32-bit PowerPC" OFF)
option(CK_ENABLE_RTM "Enable restricted transactional memory" OFF)
option(CK_ENABLE_LSE "Enable AArch64 large system extensions" OFF)
option(CK_ENABLE_SSE "Enable x86 SSE operations" ON)
option(CK_ENABLE_DOUBLE "Enable double-width primitive operations" ON)

if(CK_PLATFORM MATCHES "^(riscv32|loongarch64|unknown)$")
    set(CK_USE_CC_BUILTINS_DEFAULT ON)
else()
    set(CK_USE_CC_BUILTINS_DEFAULT OFF)
endif()
option(CK_USE_CC_BUILTINS "Use compiler atomic builtins" "${CK_USE_CC_BUILTINS_DEFAULT}")

if(CK_ENABLE_RTM AND NOT CK_PLATFORM MATCHES "^(x86|x86_64)$")
    message(FATAL_ERROR "CK_ENABLE_RTM requires an x86/x86_64 target (got ${CK_PLATFORM})")
endif()
if(CK_ENABLE_LSE AND NOT CK_PLATFORM STREQUAL "aarch64")
    message(FATAL_ERROR "CK_ENABLE_LSE requires an AArch64 target (got ${CK_PLATFORM})")
endif()

set(CK_DETECTED_VMA_BITS unknown)
if(CK_PLATFORM STREQUAL "x86")
    set(CK_DETECTED_VMA_BITS 32)
elseif(CK_PLATFORM STREQUAL "x86_64" AND NOT CMAKE_CROSSCOMPILING)
    if(APPLE)
        execute_process(
            COMMAND sysctl -n machdep.cpu.address_bits.virtual
            OUTPUT_VARIABLE CK_NATIVE_VMA_BITS
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            RESULT_VARIABLE CK_VMA_RESULT
        )
        if(CK_VMA_RESULT EQUAL 0 AND CK_NATIVE_VMA_BITS MATCHES "^[1-9][0-9]*$")
            set(CK_DETECTED_VMA_BITS "${CK_NATIVE_VMA_BITS}")
        endif()
    elseif(LINUX AND EXISTS "/proc/cpuinfo")
        file(STRINGS "/proc/cpuinfo" CK_ADDRESS_SIZE_LINE REGEX "address sizes" LIMIT_COUNT 1)
        if(CK_ADDRESS_SIZE_LINE MATCHES "([1-9][0-9]*)[ ]+bits virtual")
            set(CK_DETECTED_VMA_BITS "${CMAKE_MATCH_1}")
        endif()
    endif()
endif()

set(CK_VMA_BITS "${CK_DETECTED_VMA_BITS}" CACHE STRING "Virtual address bits (positive integer, or unknown)")
if(NOT CK_VMA_BITS MATCHES "^(unknown|[1-9][0-9]*)$")
    message(FATAL_ERROR "CK_VMA_BITS must be a positive integer or 'unknown'")
endif()

set(RTM_ENABLE CK_MD_RTM_DISABLE)
if(CK_ENABLE_RTM)
    set(RTM_ENABLE CK_MD_RTM_ENABLE)
endif()
set(LSE_ENABLE CK_MD_LSE_DISABLE)
if(CK_ENABLE_LSE)
    set(LSE_ENABLE CK_MD_LSE_ENABLE)
endif()
set(SSE_DISABLE CK_MD_SSE_DISABLE)
if(CK_ENABLE_SSE)
    set(SSE_DISABLE CK_MD_SSE_ENABLE)
endif()
set(PPC32_LWSYNC_ENABLE CK_MD_PPC32_LWSYNC_DISABLE)
if(CK_ENABLE_PPC32_LWSYNC)
    set(PPC32_LWSYNC_ENABLE CK_MD_PPC32_LWSYNC)
endif()

if(CK_VMA_BITS STREQUAL "unknown")
    set(POINTER_PACK_ENABLE CK_MD_POINTER_PACK_DISABLE)
    set(VMA_BITS CK_MD_VMA_BITS_UNKNOWN)
    set(VMA_BITS_VALUE "")
else()
    if(CK_ENABLE_POINTER_PACKING)
        set(POINTER_PACK_ENABLE CK_MD_POINTER_PACK_ENABLE)
    else()
        set(POINTER_PACK_ENABLE CK_MD_POINTER_PACK_DISABLE)
    endif()
    set(VMA_BITS CK_MD_VMA_BITS)
    set(VMA_BITS_VALUE "${CK_VMA_BITS}ULL")
endif()

set(MM "CK_MD_${CK_MEMORY_MODEL}")
set(DISABLE_DOUBLE CK_PR_DISABLE_DOUBLE)
if(CK_ENABLE_DOUBLE)
    set(DISABLE_DOUBLE CK_PR_ENABLE_DOUBLE)
endif()

set(VERSION "${PROJECT_VERSION}")
set(GIT_SHA "")
find_package(Git QUIET)
if(Git_FOUND)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_SHA
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE CK_GIT_RESULT
    )
    if(NOT CK_GIT_RESULT EQUAL 0)
        set(GIT_SHA "")
    endif()
endif()

configure_file(include/ck_md.h.in include/ck_md.h @ONLY)
configure_file(include/freebsd/ck_md.h.in include/freebsd/ck_md.h @ONLY)

add_library(ck_headers INTERFACE)
target_include_directories(ck_headers INTERFACE
    "${PROJECT_BINARY_DIR}/include"
    include
)
if(CK_USE_CC_BUILTINS)
    target_compile_definitions(ck_headers INTERFACE CK_USE_CC_BUILTINS=1)
endif()
if(CK_COMPAT_DEFINITIONS)
    target_compile_definitions(ck_headers INTERFACE ${CK_COMPAT_DEFINITIONS})
endif()

message(STATUS "CK platform: ${CK_PLATFORM}")
message(STATUS "CK memory model: ${CK_MEMORY_MODEL}")
message(STATUS "CK compiler atomics: ${CK_USE_CC_BUILTINS}")
message(STATUS "CK VMA bits: ${CK_VMA_BITS}")

option(CK_BUILD_SHARED "Build the shared libck variant" ON)
option(CK_BUILD_STATIC "Build the static libck variant" ON)

if(NOT CK_BUILD_SHARED AND NOT CK_BUILD_STATIC)
    message(FATAL_ERROR "At least one of CK_BUILD_SHARED or CK_BUILD_STATIC must be enabled")
endif()
if(CK_BUILD_SHARED AND NOT CMAKE_POSITION_INDEPENDENT_CODE)
    message(FATAL_ERROR "CK_BUILD_SHARED requires CMAKE_POSITION_INDEPENDENT_CODE=ON")
endif()

set(CK_LIBRARY_SOURCES
    src/ck_barrier_centralized.c
    src/ck_barrier_combining.c
    src/ck_barrier_dissemination.c
    src/ck_barrier_tournament.c
    src/ck_barrier_mcs.c
    src/ck_ec.c
    src/ck_epoch.c
    src/ck_ht.c
    src/ck_hp.c
    src/ck_hs.c
    src/ck_rhs.c
    src/ck_array.c
)

add_library(ck_objects OBJECT ${CK_LIBRARY_SOURCES})
target_link_libraries(ck_objects PRIVATE ck_headers)

set(CK_LIBRARY_TARGETS)
if(CK_BUILD_SHARED)
    add_library(ck_shared SHARED $<TARGET_OBJECTS:ck_objects>)
    target_link_libraries(ck_shared PUBLIC ck_headers)
    set_target_properties(ck_shared PROPERTIES
        OUTPUT_NAME ck
        VERSION "${PROJECT_VERSION}"
        SOVERSION "${PROJECT_VERSION_MAJOR}"
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
    list(APPEND CK_LIBRARY_TARGETS ck_shared)
endif()

if(CK_BUILD_STATIC)
    add_library(ck_static STATIC $<TARGET_OBJECTS:ck_objects>)
    target_link_libraries(ck_static PUBLIC ck_headers)
    if(MSVC_VERSION)
        # Give the static library a distinct name to avoid clashing with the shared library's
        # importlib.
        set_target_properties(ck_static PROPERTIES OUTPUT_NAME ck_static)
    else()
        set_target_properties(ck_static PROPERTIES OUTPUT_NAME ck)
    endif()
    list(APPEND CK_LIBRARY_TARGETS ck_static)
endif()

add_custom_target(ck)
add_dependencies(ck ${CK_LIBRARY_TARGETS})

include(CTest)
option(CK_BUILD_REGRESSIONS "Build regression validation and benchmark programs" ON)
if(CK_BUILD_REGRESSIONS)
    add_subdirectory(regressions)
endif()
