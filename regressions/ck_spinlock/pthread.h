#include <pthread.h>

#define LOCK_NAME "pthread_mutex"
#define LOCK_DEFINE pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER
#define LOCK pthread_mutex_lock(&lock)
#define UNLOCK pthread_mutex_unlock(&lock)

