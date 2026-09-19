foreach(program IN ITEMS
    ck_ring_mpmc
    ck_ring_mpmc_template
    ck_ring_spmc
    ck_ring_spmc_template
    ck_ring_spsc
)
    ck_add_regression(${program} ARGS ${CK_REGRESSION_CORES} 1 2048)
endforeach()
