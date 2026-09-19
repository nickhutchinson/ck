#define LOCK_NAME "ck_mcs"
#define LOCK_DEFINE static CK_CC_CACHELINE ck_spinlock_mcs_t lock = NULL
#define LOCK_STATE CK_CC_CACHELINE ck_spinlock_mcs_context_t node;
#define LOCK ck_spinlock_mcs_lock(&lock, &node)
#define UNLOCK ck_spinlock_mcs_unlock(&lock, &node)
#define LOCKED ck_spinlock_mcs_locked(&lock)

