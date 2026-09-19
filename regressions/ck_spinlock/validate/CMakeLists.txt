foreach(program IN ITEMS
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
)
    ck_add_regression(${program} ARGS ${CK_REGRESSION_CORES} 1)
endforeach()
