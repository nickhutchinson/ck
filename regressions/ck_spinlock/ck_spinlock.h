#define LOCK_NAME "ck_spinlock"
#define LOCK_DEFINE static CK_CC_CACHELINE ck_spinlock_t lock = CK_SPINLOCK_INITIALIZER
#define LOCK ck_spinlock_lock_eb(&lock)
#define UNLOCK ck_spinlock_unlock(&lock)
#define LOCKED ck_spinlock_locked(&lock)

