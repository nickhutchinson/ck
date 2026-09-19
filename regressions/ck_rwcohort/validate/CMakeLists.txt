# The parent Makefile does not dispatch this leaf's check.
foreach(program IN ITEMS ck_neutral ck_rp ck_wp)
    ck_add_regression(${program} ARGS ${CK_REGRESSION_HALF_CORES} 2 1 DISABLED)
endforeach()
