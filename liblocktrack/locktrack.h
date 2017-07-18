#ifndef LOCKTRACK_H
#define LOCKTRACK_H

#include <pthread.h>
#include <stdint.h>

/**
 * Print the contention stats for the locks.
 */
void
lt_stats_print(void);

/**
 * @defgroup mutex PThread mutex operations
 * @todo Figure out how handle static initialization
 * (PTHREAD_MUTEX_INITIALIZER).
 * I think we're going to have to assign a dummy value and check for it in the
 * lock ops. If we find it, allocate a lock, lock it and CAS into place.
 * @todo Unimplemented:
 * - pthread_mutex_consistent()
 * - pthread_mutex_getprioceiling()
 * - pthread_mutex_setprioceiling()
 * @{
 */

/**
 * The main lock structure.
 * This contains both the actual lock as well as the metadata we are tracking
 * about the lock.
 */
typedef struct lt_mutex_s {
        /// The underlying pthread lock.
        pthread_mutex_t lock;
        /// Number of times we acquired the lock w/o contention (protected by
        /// mutex).
        uint64_t uncontended;
        /// Number of times we had to wait for the lock (proteced by mutex).
        uint64_t contended;
        /// Name of this lock (for display purposes)
        char *name;
        /// Prev pointer for dll of locks (protected by main list lock).
        struct lt_mutex_s *prev;
        /// Next pointer for dll of locks (protected by main list lock).
        struct lt_mutex_s *next;
} lt_mutex_t;

/**
 * Initialize a mutex (see pthread_mutex_init).
 * This initializes the underlying lock as well as the tracking data for it.
 * @param [out] mutex Initialized mutex
 * @param [in] attr Pthread lock attributes for the lock
 * @param [in] mutex_name Name for the mutex to be used for printing stats
 * @returns Same error codes as pthread_mutex_init()
 */
int
lt_mutex_init(lt_mutex_t *mutex,
              const pthread_mutexattr_t *attr,
              const char *mutex_name);

/**
 * Destroy a mutex (see pthread_mutex_destroy).
 * @param [in] mutex Mutex to destroy
 * @returns Same error codes as pthread_mutex_destroy()
 */
int
lt_mutex_destroy(lt_mutex_t *mutex);

/**
 * Lock a mutex (see pthread_mutex_lock).
 * @param [in] mutex The mutex to lock
 * @returns Same error codes as pthread_mutex_lock()
 */
int
lt_mutex_lock(lt_mutex_t *mutex);

/**
 * Lock a mutex (see pthread_mutex_trylock).
 * @param [in] mutex The mutex to lock
 * @returns Same error codes as pthread_mutex_trylock()
 */
int
lt_mutex_trylock(lt_mutex_t *mutex);

/**
 * Lock a mutex with a specified timeout (see pthread_mutex_timedlock).
 * NOTE: This will not update the contended count if a timeout occurs!
 * @param [in] mutex The mutex to lock
 * @param [in] abstime The amount of time to wait
 * @returns Same error codes as pthread_mutex_timedlock()
 */
int
lt_mutex_timedlock(lt_mutex_t *mutex,
                   const struct timespec *abstime);

/**
 * Unlock a mutex (see pthread_mutex_unlock).
 * @param [in] mutex The mutex to unlock
 * @returns Same error codes as pthread_mutex_unlock()
 */
int
lt_mutex_unlock(lt_mutex_t *mutex);

/// @}



/**
 * @defgroup rwlock PThread R/W lock operations
 * @todo Figure out how handle static initialization
 * (PTHREAD_RWLOCK_INITIALIZER).
 * @todo Unimplemented:
 * - pthread_rwlock_init()
 * - pthread_rwlock_destroy()
 * - pthread_rwlock_rdlock()
 * - pthread_rwlock_timedrdlock()
 * - pthread_rwlock_timedwrlock()
 * - pthread_rwlock_tryrdlock()
 * - pthread_rwlock_trywrlock()
 * - pthread_rwlock_unlock()
 * - pthread_rwlock_wrlock()
 * @{
 */
/// @}



/**
 * @defgroup spin PThread spinlock operations
 * @{
 */
typedef struct lt_spinlock_s {
        /// The underlying pthread lock.
        pthread_spinlock_t lock;
        /// Number of times we acquired the lock w/o contention (protected by
        /// lock).
        uint64_t uncontended;
        /// Number of times we had to wait for the locki (proteced by lock).
        uint64_t contended;
        /// Name of this lock (for display purposes)
        char *name;
        /// Prev pointer for dll of locks (protected by main list lock).
        struct lt_spinlock_s *prev;
        /// Next pointer for dll of locks (protected by main list lock).
        struct lt_spinlock_s *next;
} lt_spinlock_t;

/**
 * Initialize a spinlock (see pthread_spinlock_init).
 * @param [inout] lock The lock to initialize
 * @param [in] pshared Whether the lock is shared (passed through to pthreads)
 * @param [in] name A name for the lock
 * @returns Same error codes as pthread_spinlock_init()
 */
int
lt_spin_init(lt_spinlock_t *lock,
             int pshared,
             const char *name);

/**
 * Destroy a spinlock (see pthread_spinlock_destroy).
 * @param [inout] lock The lock to destroy
 * @returns Same error codes as pthread_spinlock_destroy()
 */
int
lt_spin_destroy(lt_spinlock_t *lock);

/**
 * Lock a spinlock (see pthread_spinlock_lock).
 * @param [inout] lock The lock to lock
 * @returns Same error codes as pthread_spinlock_lock()
 */
int
lt_spin_lock(lt_spinlock_t *lock);

/**
 * Lock a spinlock without waiting (see pthread_spinlock_trylock).
 * @param [inout] lock The lock to lock
 * @returns Same error codes as pthread_spinlock_trylock()
 */
int
lt_spin_trylock(lt_spinlock_t *lock);

/**
 * Unlock a spinlock (see pthread_spinlock_unlock).
 * @param [inout] lock The lock to unlock
 * @returns Same error codes as pthread_spinlock_unlock()
 */
int
lt_spin_unlock(lt_spinlock_t *lock);

/// @}

/**
 * @mainpage
 * This library tracks pthread lock contention with simple counters. See the
 * documentation for:
 * - @ref mutex
 * - @ref spin
 * - @ref rwlock
 */
#endif
