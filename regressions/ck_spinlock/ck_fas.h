#define LOCK_NAME "ck_fas"
#define LOCK_DEFINE static CK_CC_CACHELINE ck_spinlock_fas_t lock = CK_SPINLOCK_FAS_INITIALIZER
#define LOCK ck_spinlock_fas_lock_eb(&lock)
#define UNLOCK ck_spinlock_fas_unlock(&lock)
#define LOCKED ck_spinlock_fas_locked(&lock)

