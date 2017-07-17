#ifndef LOCKTRACK_H
#define LOCKTRACK_H

#include <pthread.h>
#include <stdint.h>

/// Bytes in a CPU cache line
#define CACHE_LINE 64

/**
 * The main lock structure.
 * This contains both the actual lock as well as the metadata we are tracking
 * about the lock. This structure should be cache aligned to minimize false
 * sharing.
 */
typedef struct lt_mutex_s {
        /// The underlying pthread lock.
        pthread_mutex_t mutex;
        /// Number of times we acquired the lock w/o contention (protected by
        /// mutex).
        uint64_t uncontended;
        /// Number of times we had to wait for the locki (proteced by mutex).
        uint64_t contended;
        /// Name of this lock (for display purposes)
        char *name;
        /// Prev pointer for dll of locks (protected by main list lock).
        struct lt_mutex_s *prev;
        /// Next pointer for dll of locks (protected by main list lock).
        struct lt_mutex_s *next;
} lt_mutex_t __attribute__ ((aligned(CACHE_LINE)));

/*
 *  XXX: How do we handle static initialization?
 * I think we're going to have to assign a dummy value and check for it in the
 * lock ops. If we find it, allocate a lock, lock it and CAS into place.
 */

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
 * Unlock a mutex (see pthread_mutex_unlock).
 * @param [in] mutex The mutex to unlock
 * @returns Same error codes as pthread_mutex_unlock()
 */
int
lt_mutex_unlock(lt_mutex_t *mutex);

// pthread_mutex_timedlock()

#endif
