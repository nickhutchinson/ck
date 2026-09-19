foreach(program IN ITEMS
    barrier_centralized
    barrier_combining
    barrier_dissemination
    barrier_mcs
    barrier_tournament
)
    ck_add_regression(${program} ARGS ${CK_REGRESSION_CORES} 1 1)
endforeach()
