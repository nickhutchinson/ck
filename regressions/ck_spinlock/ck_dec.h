#define LOCK_NAME "ck_dec"
#define LOCK_DEFINE static CK_CC_CACHELINE ck_spinlock_dec_t lock = CK_SPINLOCK_DEC_INITIALIZER
#define LOCK ck_spinlock_dec_lock_eb(&lock)
#define UNLOCK ck_spinlock_dec_unlock(&lock)
#define LOCKED ck_spinlock_dec_locked(&lock)

