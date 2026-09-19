set(locks
    ck_anderson
    ck_cas
    ck_clh
    ck_dec
    ck_fas
    ck_hclh
    ck_mcs
    ck_spinlock
    ck_ticket
    ck_ticket_pb
    linux_spinlock
    pthread
)
foreach(lock IN LISTS locks)
    foreach(mode IN ITEMS THROUGHPUT LATENCY)
        ck_add_benchmark(${lock}.${mode} SOURCES ${lock}.c DEFINITIONS ${mode})
    endforeach()
endforeach()
