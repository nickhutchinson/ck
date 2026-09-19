include(ProcessorCount)

find_package(Threads REQUIRED)

if(NOT DEFINED CK_REGRESSION_CORES)
    if(CMAKE_CROSSCOMPILING)
        set(CK_REGRESSION_CORE_DEFAULT 2)
    else()
        ProcessorCount(CK_REGRESSION_CORE_DEFAULT)
        if(NOT CK_REGRESSION_CORE_DEFAULT)
            set(CK_REGRESSION_CORE_DEFAULT 2)
        endif()
    endif()
    set(CK_REGRESSION_CORES "${CK_REGRESSION_CORE_DEFAULT}" CACHE STRING
        "Number of cores used by regression programs")
else()
    set(CK_REGRESSION_CORES "${CK_REGRESSION_CORES}" CACHE STRING
        "Number of cores used by regression programs")
endif()
if(NOT CK_REGRESSION_CORES MATCHES "^[1-9][0-9]*$")
    message(FATAL_ERROR "CK_REGRESSION_CORES must be a positive integer")
endif()
math(EXPR CK_REGRESSION_HALF_CORES "${CK_REGRESSION_CORES} / 2")
if(CK_REGRESSION_HALF_CORES LESS 1)
    set(CK_REGRESSION_HALF_CORES 1)
endif()

set(CK_EC_FUZZER none CACHE STRING "EC property harness: none or libfuzzer")
if(NOT CK_EC_FUZZER MATCHES "^(none|libfuzzer)$")
    message(FATAL_ERROR "CK_EC_FUZZER must be none or libfuzzer")
endif()

add_compile_definitions(_GNU_SOURCE)

# Strip the -NDEBUG / /NDEBUG flags CMake adds to non-debug configs.
foreach(config RELEASE RELWITHDEBINFO MINSIZEREL)
    string(REGEX REPLACE "[ ]*[/-]DNDEBUG[ ]*" " " CMAKE_C_FLAGS_${config} "${CMAKE_C_FLAGS_${config}}")
endforeach()

set(CK_VALIDATION_TARGETS)
set(CK_BENCHMARK_TARGETS)

function(ck_add_regression component kind output)
    set(options THREADS MATH)
    set(one_value_args C_STANDARD)
    set(multi_value_args SOURCES DEFINITIONS)
    cmake_parse_arguments(CK_REG "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if(NOT "${kind}" MATCHES "^(validate|benchmark)$")
        message(FATAL_ERROR "Unknown regression kind: ${kind}")
    endif()
    if(NOT CK_REG_SOURCES)
        message(FATAL_ERROR "Regression ck_${component}/${kind}/${output} has no sources")
    endif()

    string(MAKE_C_IDENTIFIER "${output}" output_identifier)
    set(target "ck_regression_${component}_${kind}_${output_identifier}")
    add_executable(${target} EXCLUDE_FROM_ALL ${CK_REG_SOURCES})
    target_link_libraries(${target} PRIVATE ck_headers)
    target_compile_definitions(${target} PRIVATE CORES=${CK_REGRESSION_CORES} ${CK_REG_DEFINITIONS})
    if(CK_REG_THREADS)
        target_link_libraries(${target} PRIVATE Threads::Threads)
    endif()
    if(CK_REG_MATH)
        target_link_libraries(${target} PRIVATE m)
    endif()
    if(CK_REG_C_STANDARD)
        set_property(TARGET ${target} PROPERTY C_STANDARD "${CK_REG_C_STANDARD}")
    endif()

    set(output_directory "${PROJECT_BINARY_DIR}/regressions/ck_${component}/${kind}")
    set_target_properties(${target} PROPERTIES
        OUTPUT_NAME "${output}"
        RUNTIME_OUTPUT_DIRECTORY "${output_directory}"
    )

    if("${kind}" STREQUAL "validate")
        list(APPEND CK_VALIDATION_TARGETS ${target})
        set(CK_VALIDATION_TARGETS "${CK_VALIDATION_TARGETS}" PARENT_SCOPE)
    else()
        list(APPEND CK_BENCHMARK_TARGETS ${target})
        set(CK_BENCHMARK_TARGETS "${CK_BENCHMARK_TARGETS}" PARENT_SCOPE)
    endif()
endfunction()

function(ck_add_validation_test component output)
    string(MAKE_C_IDENTIFIER "${output}" output_identifier)
    set(target "ck_regression_${component}_validate_${output_identifier}")
    add_test(NAME "ck_${component}/${output}" COMMAND "$<TARGET_FILE:${target}>" ${ARGN})
endfunction()

# Validation executables.
ck_add_regression(array validate serial
    SOURCES ck_array/validate/serial.c ../src/ck_array.c)
ck_add_regression(backoff validate validate
    SOURCES ck_backoff/validate/validate.c)

foreach(output IN ITEMS barrier_centralized barrier_combining barrier_dissemination barrier_tournament barrier_mcs)
    ck_add_regression(barrier validate ${output}
        SOURCES ck_barrier/validate/${output}.c ../src/ck_${output}.c THREADS)
endforeach()

ck_add_regression(bitmap validate serial SOURCES ck_bitmap/validate/serial.c)
ck_add_regression(brlock validate validate SOURCES ck_brlock/validate/validate.c THREADS)
ck_add_regression(bytelock validate validate SOURCES ck_bytelock/validate/validate.c THREADS)
ck_add_regression(cc validate ck_cc SOURCES ck_cc/validate/ck_cc.c)
ck_add_regression(cohort validate validate SOURCES ck_cohort/validate/validate.c THREADS)

set(CK_EC_PROPERTY_OUTPUTS
    prop_test_timeutil_add
    prop_test_timeutil_add_ns
    prop_test_timeutil_cmp
    prop_test_timeutil_scale
    prop_test_value
    prop_test_wakeup
    prop_test_slow_wakeup
)
ck_add_regression(ec validate ck_ec_smoke_test
    SOURCES ck_ec/validate/ck_ec_smoke_test.c ../src/ck_ec.c THREADS C_STANDARD 11)
foreach(output IN LISTS CK_EC_PROPERTY_OUTPUTS)
    ck_add_regression(ec validate ${output}
        SOURCES ck_ec/validate/${output}.c ../src/ck_ec.c THREADS)
    string(MAKE_C_IDENTIFIER "${output}" output_identifier)
    set(property_target "ck_regression_ec_validate_${output_identifier}")
    if(CK_EC_FUZZER STREQUAL "libfuzzer")
        target_compile_definitions(${property_target} PRIVATE USE_LIBFUZZER)
        target_compile_options(${property_target} PRIVATE -fsanitize=fuzzer,memory,undefined)
        target_link_options(${property_target} PRIVATE -fsanitize=fuzzer,memory,undefined)
    endif()
endforeach()

foreach(output IN ITEMS ck_stack ck_epoch_synchronize ck_epoch_poll ck_epoch_call ck_epoch_section ck_epoch_section_2 torture)
    ck_add_regression(epoch validate ${output}
        SOURCES ck_epoch/validate/${output}.c ../src/ck_epoch.c THREADS)
endforeach()

foreach(output IN ITEMS ck_fifo_spsc ck_fifo_mpmc ck_fifo_spsc_iterator ck_fifo_mpmc_iterator)
    ck_add_regression(fifo validate ${output}
        SOURCES ck_fifo/validate/${output}.c THREADS)
endforeach()

foreach(output IN ITEMS ck_hp_stack nbds_haz_test serial ck_hp_fifo ck_hp_fifo_donner)
    ck_add_regression(hp validate ${output}
        SOURCES ck_hp/validate/${output}.c ../src/ck_hp.c THREADS)
endforeach()

ck_add_regression(hs validate serial SOURCES ck_hs/validate/serial.c ../src/ck_hs.c)
ck_add_regression(hs validate hs_init_opts SOURCES ck_hs/validate/hs_init_opts.c ../src/ck_hs.c)
ck_add_regression(ht validate serial SOURCES ck_ht/validate/serial.c ../src/ck_ht.c)
ck_add_regression(ht validate serial.delete
    SOURCES ck_ht/validate/serial.c ../src/ck_ht.c DEFINITIONS HT_DELETE)
ck_add_regression(pflock validate validate SOURCES ck_pflock/validate/validate.c THREADS)

set(CK_PR_VALIDATION_OUTPUTS
    ck_pr_cas ck_pr_faa ck_pr_inc ck_pr_dec ck_pr_bts ck_pr_btr ck_pr_btc
    ck_pr_load ck_pr_store ck_pr_and ck_pr_or ck_pr_xor ck_pr_add ck_pr_sub
    ck_pr_fas ck_pr_bin ck_pr_btx ck_pr_fax ck_pr_n ck_pr_unary ck_pr_fence
    ck_pr_dec_zero ck_pr_inc_zero
)
foreach(output IN LISTS CK_PR_VALIDATION_OUTPUTS)
    ck_add_regression(pr validate ${output} SOURCES ck_pr/validate/${output}.c)
endforeach()

foreach(output IN ITEMS ck_list ck_slist ck_stailq)
    ck_add_regression(queue validate ${output}
        SOURCES ck_queue/validate/${output}.c THREADS)
endforeach()

ck_add_regression(rhs validate probe_bound SOURCES ck_rhs/validate/probe_bound.c)
ck_add_regression(rhs validate serial SOURCES ck_rhs/validate/serial.c ../src/ck_rhs.c)

foreach(output IN ITEMS ck_ring_spsc ck_ring_spmc ck_ring_spmc_template ck_ring_mpmc ck_ring_mpmc_template)
    ck_add_regression(ring validate ${output}
        SOURCES ck_ring/validate/${output}.c ../src/ck_barrier_centralized.c THREADS)
endforeach()

ck_add_regression(rtm validate validate SOURCES ck_rtm/validate/validate.c THREADS)
foreach(output IN ITEMS ck_neutral ck_rp ck_wp)
    ck_add_regression(rwcohort validate ${output}
        SOURCES ck_rwcohort/validate/${output}.c THREADS)
endforeach()
ck_add_regression(rwlock validate validate SOURCES ck_rwlock/validate/validate.c THREADS)
ck_add_regression(sequence validate ck_sequence SOURCES ck_sequence/validate/ck_sequence.c THREADS)

set(CK_SPINLOCK_VALIDATION_OUTPUTS
    ck_ticket ck_mcs ck_dec ck_cas ck_fas ck_clh linux_spinlock ck_ticket_pb
    ck_anderson ck_spinlock ck_hclh
)
foreach(output IN LISTS CK_SPINLOCK_VALIDATION_OUTPUTS)
    ck_add_regression(spinlock validate ${output}
        SOURCES ck_spinlock/validate/${output}.c THREADS MATH)
endforeach()

ck_add_regression(stack validate serial SOURCES ck_stack/validate/serial.c THREADS)
ck_add_regression(stack validate upmc_trypush SOURCES ck_stack/validate/push.c DEFINITIONS TRYUPMC THREADS)
ck_add_regression(stack validate mpmc_trypush SOURCES ck_stack/validate/push.c DEFINITIONS TRYMPMC THREADS)
ck_add_regression(stack validate mpnc_push SOURCES ck_stack/validate/push.c DEFINITIONS MPNC THREADS)
ck_add_regression(stack validate mpmc_push SOURCES ck_stack/validate/push.c DEFINITIONS MPMC THREADS)
ck_add_regression(stack validate upmc_push SOURCES ck_stack/validate/push.c DEFINITIONS UPMC THREADS)
ck_add_regression(stack validate spinlock_push SOURCES ck_stack/validate/push.c DEFINITIONS SPINLOCK THREADS)
ck_add_regression(stack validate spinlock_eb_push SOURCES ck_stack/validate/push.c DEFINITIONS SPINLOCK EB THREADS)
ck_add_regression(stack validate mpmc_trypop SOURCES ck_stack/validate/pop.c DEFINITIONS TRYMPMC THREADS)
ck_add_regression(stack validate upmc_trypop SOURCES ck_stack/validate/pop.c DEFINITIONS TRYUPMC THREADS)
ck_add_regression(stack validate mpmc_pop SOURCES ck_stack/validate/pop.c DEFINITIONS MPMC THREADS)
ck_add_regression(stack validate upmc_pop SOURCES ck_stack/validate/pop.c DEFINITIONS UPMC THREADS)
ck_add_regression(stack validate spinlock_pop SOURCES ck_stack/validate/pop.c DEFINITIONS SPINLOCK THREADS)
ck_add_regression(stack validate spinlock_eb_pop SOURCES ck_stack/validate/pop.c DEFINITIONS SPINLOCK EB THREADS)
ck_add_regression(stack validate mpmc_trypair SOURCES ck_stack/validate/pair.c DEFINITIONS TRYMPMC THREADS)
ck_add_regression(stack validate mpmc_pair SOURCES ck_stack/validate/pair.c DEFINITIONS MPMC THREADS)
ck_add_regression(stack validate spinlock_pair SOURCES ck_stack/validate/pair.c DEFINITIONS SPINLOCK THREADS)
ck_add_regression(stack validate spinlock_eb_pair SOURCES ck_stack/validate/pair.c DEFINITIONS SPINLOCK EB THREADS)
ck_add_regression(stack validate pthreads_pair SOURCES ck_stack/validate/pair.c DEFINITIONS PTHREADS THREADS)

ck_add_regression(swlock validate validate SOURCES ck_swlock/validate/validate.c THREADS)
ck_add_regression(tflock validate validate SOURCES ck_tflock/validate/validate.c THREADS)

# Benchmark executables.
ck_add_regression(barrier benchmark throughput
    SOURCES ck_barrier/benchmark/throughput.c ../src/ck_barrier_centralized.c THREADS)
foreach(component IN ITEMS brlock pflock rwlock swlock tflock)
    foreach(output IN ITEMS latency throughput)
        ck_add_regression(${component} benchmark ${output}
            SOURCES ck_${component}/benchmark/${output}.c THREADS)
    endforeach()
endforeach()
ck_add_regression(bytelock benchmark latency SOURCES ck_bytelock/benchmark/latency.c THREADS)
ck_add_regression(cohort benchmark ck_cohort.THROUGHPUT
    SOURCES ck_cohort/benchmark/throughput.c THREADS MATH)
ck_add_regression(cohort benchmark ck_cohort.LATENCY
    SOURCES ck_cohort/benchmark/ck_cohort.c DEFINITIONS LATENCY THREADS MATH)
ck_add_regression(ec benchmark ck_ec
    SOURCES ck_ec/benchmark/ck_ec.c ../src/ck_ec.c)
ck_add_regression(fifo benchmark latency SOURCES ck_fifo/benchmark/latency.c THREADS)
ck_add_regression(hp benchmark fifo_latency
    SOURCES ck_hp/benchmark/fifo_latency.c ../src/ck_hp.c THREADS)
ck_add_regression(hp benchmark stack_latency
    SOURCES ck_hp/benchmark/stack_latency.c ../src/ck_hp.c THREADS)
ck_add_regression(hs benchmark serial SOURCES ck_hs/benchmark/serial.c ../src/ck_hs.c)
ck_add_regression(hs benchmark apply SOURCES ck_hs/benchmark/apply.c ../src/ck_hs.c)
ck_add_regression(hs benchmark parallel_bytestring
    SOURCES ck_hs/benchmark/parallel_bytestring.c ../src/ck_hs.c ../src/ck_epoch.c THREADS)
ck_add_regression(hs benchmark parallel_bytestring.delete
    SOURCES ck_hs/benchmark/parallel_bytestring.c ../src/ck_hs.c ../src/ck_epoch.c
    DEFINITIONS HS_DELETE THREADS)
ck_add_regression(ht benchmark serial SOURCES ck_ht/benchmark/serial.c ../src/ck_ht.c)
ck_add_regression(ht benchmark serial.delete
    SOURCES ck_ht/benchmark/serial.c ../src/ck_ht.c DEFINITIONS HT_DELETE)
ck_add_regression(ht benchmark parallel_bytestring
    SOURCES ck_ht/benchmark/parallel_bytestring.c ../src/ck_ht.c ../src/ck_epoch.c THREADS)
ck_add_regression(ht benchmark parallel_bytestring.delete
    SOURCES ck_ht/benchmark/parallel_bytestring.c ../src/ck_ht.c ../src/ck_epoch.c
    DEFINITIONS HT_DELETE THREADS)
ck_add_regression(ht benchmark parallel_direct
    SOURCES ck_ht/benchmark/parallel_direct.c ../src/ck_ht.c ../src/ck_epoch.c THREADS)

ck_add_regression(pr benchmark fp SOURCES ck_pr/benchmark/fp.c THREADS)
foreach(output IN ITEMS ck_pr_cas_64 ck_pr_fas_64 ck_pr_cas_64_2 ck_pr_add_64 ck_pr_faa_64 ck_pr_neg_64)
    ck_add_regression(pr benchmark ${output}
        SOURCES ck_pr/benchmark/${output}.c THREADS MATH)
endforeach()

ck_add_regression(rhs benchmark serial SOURCES ck_rhs/benchmark/serial.c ../src/ck_rhs.c)
ck_add_regression(rhs benchmark parallel_bytestring
    SOURCES ck_rhs/benchmark/parallel_bytestring.c ../src/ck_rhs.c ../src/ck_epoch.c THREADS)
ck_add_regression(ring benchmark latency SOURCES ck_ring/benchmark/latency.c)

foreach(lock IN ITEMS ck_neutral ck_rp ck_wp)
    foreach(mode IN ITEMS THROUGHPUT LATENCY)
        ck_add_regression(rwcohort benchmark ${lock}.${mode}
            SOURCES ck_rwcohort/benchmark/${lock}.c DEFINITIONS ${mode} THREADS)
    endforeach()
endforeach()

ck_add_regression(sequence benchmark ck_sequence
    SOURCES ck_sequence/benchmark/ck_sequence.c
    THREADS)
set(CK_SPINLOCK_BENCHMARK_LOCKS
    ck_ticket ck_mcs ck_dec ck_cas ck_fas ck_clh linux_spinlock ck_ticket_pb
    ck_anderson ck_spinlock pthread ck_hclh
)
foreach(lock IN LISTS CK_SPINLOCK_BENCHMARK_LOCKS)
    foreach(mode IN ITEMS THROUGHPUT LATENCY)
        ck_add_regression(spinlock benchmark ${lock}.${mode}
            SOURCES ck_spinlock/benchmark/${lock}.c DEFINITIONS ${mode} THREADS MATH)
    endforeach()
endforeach()
ck_add_regression(stack benchmark latency SOURCES ck_stack/benchmark/latency.c THREADS)

add_custom_target(regressions)
add_dependencies(regressions ${CK_VALIDATION_TARGETS} ${CK_BENCHMARK_TARGETS})

set(CK_EC_VALIDATION_TARGETS ck_regression_ec_validate_ck_ec_smoke_test)
foreach(output IN LISTS CK_EC_PROPERTY_OUTPUTS)
    string(MAKE_C_IDENTIFIER "${output}" output_identifier)
    list(APPEND CK_EC_VALIDATION_TARGETS "ck_regression_ec_validate_${output_identifier}")
endforeach()
add_custom_target(quickfuzz
    COMMAND "$<TARGET_FILE:ck_regression_ec_validate_prop_test_slow_wakeup>" -max_total_time=5
    COMMAND "$<TARGET_FILE:ck_regression_ec_validate_prop_test_timeutil_add>" -max_total_time=5
    COMMAND "$<TARGET_FILE:ck_regression_ec_validate_prop_test_timeutil_add_ns>" -max_total_time=5
    COMMAND "$<TARGET_FILE:ck_regression_ec_validate_prop_test_timeutil_cmp>" -max_total_time=5
    COMMAND "$<TARGET_FILE:ck_regression_ec_validate_prop_test_timeutil_scale>" -max_total_time=5
    COMMAND "$<TARGET_FILE:ck_regression_ec_validate_prop_test_value>" -max_total_time=5
    COMMAND "$<TARGET_FILE:ck_regression_ec_validate_prop_test_wakeup>" -max_total_time=5
    DEPENDS ${CK_EC_VALIDATION_TARGETS}
    USES_TERMINAL
)

if(BUILD_TESTING)
    ck_add_validation_test(array serial)
    ck_add_validation_test(backoff validate)
    foreach(output IN ITEMS barrier_centralized barrier_combining barrier_dissemination barrier_tournament barrier_mcs)
        ck_add_validation_test(barrier ${output} ${CK_REGRESSION_CORES} 1 1)
    endforeach()
    ck_add_validation_test(bitmap serial)
    ck_add_validation_test(brlock validate ${CK_REGRESSION_CORES} 1)
    ck_add_validation_test(bytelock validate ${CK_REGRESSION_CORES} 1)
    ck_add_validation_test(cc ck_cc)
    ck_add_validation_test(cohort validate ${CK_REGRESSION_HALF_CORES} 2 1)
    ck_add_validation_test(ec ck_ec_smoke_test)
    foreach(output IN LISTS CK_EC_PROPERTY_OUTPUTS)
        ck_add_validation_test(ec ${output} -max_total_time=60)
    endforeach()
    ck_add_validation_test(epoch ck_stack ${CK_REGRESSION_CORES} 1)
    ck_add_validation_test(epoch ck_epoch_synchronize ${CK_REGRESSION_HALF_CORES} ${CK_REGRESSION_HALF_CORES} 1)
    ck_add_validation_test(epoch ck_epoch_poll ${CK_REGRESSION_CORES} 1 1)
    ck_add_validation_test(epoch ck_epoch_section)
    ck_add_validation_test(epoch ck_epoch_section_2 ${CK_REGRESSION_HALF_CORES} ${CK_REGRESSION_HALF_CORES} 1)
    ck_add_validation_test(epoch torture ${CK_REGRESSION_HALF_CORES} ${CK_REGRESSION_HALF_CORES} 1)
    ck_add_validation_test(fifo ck_fifo_spsc ${CK_REGRESSION_CORES} 1 64000)
    ck_add_validation_test(fifo ck_fifo_mpmc ${CK_REGRESSION_CORES} 1 16000)
    ck_add_validation_test(fifo ck_fifo_spsc_iterator)
    ck_add_validation_test(fifo ck_fifo_mpmc_iterator)
    ck_add_validation_test(hp serial)
    ck_add_validation_test(hp ck_hp_stack ${CK_REGRESSION_HALF_CORES} 64 1)
    ck_add_validation_test(hp ck_hp_fifo ${CK_REGRESSION_HALF_CORES} 1 1024 100)
    ck_add_validation_test(hp nbds_haz_test ${CK_REGRESSION_HALF_CORES} 15 1)
    ck_add_validation_test(hp ck_hp_fifo_donner ${CK_REGRESSION_HALF_CORES} 16384)
    ck_add_validation_test(hs serial)
    ck_add_validation_test(ht serial)
    ck_add_validation_test(ht serial.delete)
    ck_add_validation_test(pflock validate ${CK_REGRESSION_CORES} 1)
    foreach(output IN LISTS CK_PR_VALIDATION_OUTPUTS)
        ck_add_validation_test(pr ${output})
    endforeach()
    ck_add_validation_test(queue ck_list ${CK_REGRESSION_CORES} 5)
    ck_add_validation_test(queue ck_slist ${CK_REGRESSION_CORES} 5)
    ck_add_validation_test(queue ck_stailq ${CK_REGRESSION_CORES} 1000000)
    ck_add_validation_test(rhs probe_bound)
    ck_add_validation_test(rhs serial)
    foreach(output IN ITEMS ck_ring_spsc ck_ring_spmc ck_ring_spmc_template ck_ring_mpmc ck_ring_mpmc_template)
        ck_add_validation_test(ring ${output} ${CK_REGRESSION_CORES} 1 2048)
    endforeach()
    ck_add_validation_test(rtm validate ${CK_REGRESSION_CORES} 1)
    ck_add_validation_test(rwlock validate ${CK_REGRESSION_CORES} 1)
    ck_add_validation_test(sequence ck_sequence ${CK_REGRESSION_CORES} 1)
    foreach(output IN LISTS CK_SPINLOCK_VALIDATION_OUTPUTS)
        ck_add_validation_test(spinlock ${output} ${CK_REGRESSION_CORES} 1)
    endforeach()
    ck_add_validation_test(stack serial)
    foreach(output IN ITEMS mpmc_pair upmc_trypop mpmc_trypop mpmc_trypair mpmc_pop upmc_pop mpnc_push mpmc_push upmc_push mpmc_trypush upmc_trypush)
        ck_add_validation_test(stack ${output} ${CK_REGRESSION_CORES} 1 0)
    endforeach()
    ck_add_validation_test(swlock validate ${CK_REGRESSION_CORES} 1)
    ck_add_validation_test(tflock validate ${CK_REGRESSION_CORES} 1)
endif()
