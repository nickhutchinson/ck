set(CK_EC_FUZZER none CACHE STRING "EC property harness: none or libfuzzer")
if(NOT CK_EC_FUZZER MATCHES "^(none|libfuzzer)$")
    message(FATAL_ERROR "CK_EC_FUZZER must be none or libfuzzer")
endif()

set(ec_property_library ${CK_REGRESSION_LIBRARY})
if(CK_EC_FUZZER STREQUAL "libfuzzer")
    add_library(ck_regression_ec_instrumented STATIC ../../../src/ck_ec.c)
    target_link_libraries(ck_regression_ec_instrumented
        PUBLIC ck_headers
        PRIVATE ck_regression_support
    )
    target_compile_definitions(ck_regression_ec_instrumented INTERFACE USE_LIBFUZZER)
    target_compile_options(ck_regression_ec_instrumented PUBLIC -fsanitize=fuzzer,memory,undefined)
    target_link_options(ck_regression_ec_instrumented INTERFACE -fsanitize=fuzzer,memory,undefined)
    set(ec_property_library ck_regression_ec_instrumented)
endif()

ck_add_regression(ck_ec_smoke_test OUT_TARGET smoke_target)
set_target_properties(${smoke_target} PROPERTIES C_STANDARD 11)

set(ec_property_tests
    prop_test_slow_wakeup
    prop_test_timeutil_add
    prop_test_timeutil_add_ns
    prop_test_timeutil_cmp
    prop_test_timeutil_scale
    prop_test_value
    prop_test_wakeup
)
set(ec_property_targets)
set(quickfuzz_commands)
foreach(program IN LISTS ec_property_tests)
    ck_add_regression(${program}
        LIBRARY ${ec_property_library}
        OUT_TARGET target
        ARGS -max_total_time=60
    )
    list(APPEND ec_property_targets ${target})
    list(APPEND quickfuzz_commands COMMAND ${target} -max_total_time=5)
endforeach()

add_custom_target(quickfuzz
    ${quickfuzz_commands}
    DEPENDS ${smoke_target} ${ec_property_targets}
    USES_TERMINAL
    VERBATIM
)
