ck_add_regression(ck_epoch_call)
ck_add_regression(ck_epoch_poll ARGS ${CK_REGRESSION_CORES} 1 1)
ck_add_regression(ck_epoch_section)
ck_add_regression(ck_stack ARGS ${CK_REGRESSION_CORES} 1)

foreach(program IN ITEMS ck_epoch_section_2 ck_epoch_synchronize torture)
    ck_add_regression(${program} ARGS ${CK_REGRESSION_HALF_CORES} ${CK_REGRESSION_HALF_CORES} 1)
endforeach()
