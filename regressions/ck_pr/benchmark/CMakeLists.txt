foreach(program IN ITEMS
    ck_pr_add_64
    ck_pr_cas_64
    ck_pr_cas_64_2
    ck_pr_faa_64
    ck_pr_fas_64
    ck_pr_neg_64
    fp
)
    ck_add_benchmark(${program})
endforeach()
