#ifndef LOCKTRACK_H
#define LOCKTRACK_H

#include <pthread.h>
#include <stdint.h>

#define CACHE_LINE 64

typedef struct lt_mutex_s {
        pthread_mutex_t mutex;
        uint64_t uncontended;
        uint64_t contended;
        char *name;
        struct lt_mutex_s *prev;
        struct lt_mutex_s *next;
} lt_mutex_t __attribute__ ((aligned(CACHE_LINE)));

/*
 *  XXX: How do we handle static initialization?
 * I think we're going to have to assign a dummy value and check for it in the
 * lock ops. If we find it, allocate a lock, lock it and CAS into place.
 */

int
lt_mutex_init(lt_mutex_t *mutex,
              const pthread_mutexattr_t *attr,
              const char *mutex_name);

int
lt_mutex_destroy(lt_mutex_t *mutex);

int
lt_mutex_lock(lt_mutex_t *mutex);

int
lt_mutex_trylock(lt_mutex_t *mutex);

int
lt_mutex_unlock(lt_mutex_t *mutex);

// pthread_mutex_timedlock()

#endif
