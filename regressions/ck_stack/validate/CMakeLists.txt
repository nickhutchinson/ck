ck_add_regression(serial)
ck_add_regression(mpmc_pair SOURCES pair.c DEFINITIONS MPMC ARGS ${CK_REGRESSION_CORES} 1 0)
ck_add_regression(mpmc_pop SOURCES pop.c DEFINITIONS MPMC ARGS ${CK_REGRESSION_CORES} 1 0)
ck_add_regression(mpmc_push SOURCES push.c DEFINITIONS MPMC ARGS ${CK_REGRESSION_CORES} 1 0)
ck_add_regression(mpmc_trypair SOURCES pair.c DEFINITIONS TRYMPMC ARGS ${CK_REGRESSION_CORES} 1 0)
ck_add_regression(mpmc_trypop SOURCES pop.c DEFINITIONS TRYMPMC ARGS ${CK_REGRESSION_CORES} 1 0)
ck_add_regression(mpmc_trypush SOURCES push.c DEFINITIONS TRYMPMC ARGS ${CK_REGRESSION_CORES} 1 0)
ck_add_regression(mpnc_push SOURCES push.c DEFINITIONS MPNC ARGS ${CK_REGRESSION_CORES} 1 0)
ck_add_regression(upmc_pop SOURCES pop.c DEFINITIONS UPMC ARGS ${CK_REGRESSION_CORES} 1 0)
ck_add_regression(upmc_push SOURCES push.c DEFINITIONS UPMC ARGS ${CK_REGRESSION_CORES} 1 0)
ck_add_regression(upmc_trypop SOURCES pop.c DEFINITIONS TRYUPMC ARGS ${CK_REGRESSION_CORES} 1 0)
ck_add_regression(upmc_trypush SOURCES push.c DEFINITIONS TRYUPMC ARGS ${CK_REGRESSION_CORES} 1 0)

# Make builds these comparison programs but does not invoke them from check.
ck_add_regression(pthreads_pair
    SOURCES pair.c
    DEFINITIONS PTHREADS
    ARGS ${CK_REGRESSION_CORES} 1 0
    DISABLED
)
ck_add_regression(spinlock_eb_pair
    SOURCES pair.c
    DEFINITIONS EB SPINLOCK
    ARGS ${CK_REGRESSION_CORES} 1 0
    DISABLED
)
ck_add_regression(spinlock_eb_pop
    SOURCES pop.c
    DEFINITIONS EB SPINLOCK
    ARGS ${CK_REGRESSION_CORES} 1 0
    DISABLED
)
ck_add_regression(spinlock_eb_push
    SOURCES push.c
    DEFINITIONS EB SPINLOCK
    ARGS ${CK_REGRESSION_CORES} 1 0
    DISABLED
)
ck_add_regression(spinlock_pair
    SOURCES pair.c
    DEFINITIONS SPINLOCK
    ARGS ${CK_REGRESSION_CORES} 1 0
    DISABLED
)
ck_add_regression(spinlock_pop
    SOURCES pop.c
    DEFINITIONS SPINLOCK
    ARGS ${CK_REGRESSION_CORES} 1 0
    DISABLED
)
ck_add_regression(spinlock_push
    SOURCES push.c
    DEFINITIONS SPINLOCK
    ARGS ${CK_REGRESSION_CORES} 1 0
    DISABLED
)
