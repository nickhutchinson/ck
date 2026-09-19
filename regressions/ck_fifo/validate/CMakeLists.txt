ck_add_regression(ck_fifo_mpmc ARGS ${CK_REGRESSION_CORES} 1 16000)
ck_add_regression(ck_fifo_mpmc_iterator)
ck_add_regression(ck_fifo_spsc ARGS ${CK_REGRESSION_CORES} 1 64000)
ck_add_regression(ck_fifo_spsc_iterator)
