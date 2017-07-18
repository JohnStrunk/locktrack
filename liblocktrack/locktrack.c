#include "locktrack.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// List for tracking the outstanding mutexes.
struct {
        pthread_mutex_t lock;
        lt_mutex_t *head;
        lt_mutex_t *tail;
} mlist = {PTHREAD_MUTEX_INITIALIZER, 0, 0};

/// List for tracking the outstanding spinlocks.
struct {
        pthread_spinlock_t lock;
        lt_spinlock_t *head;
        lt_spinlock_t *tail;
} slist;
/// pthread_spinlock_t doesn't have a static initializer.
bool slist_initialized = false;


void
lt_stats_print()
{
        printf("=== Tracking stats ===\n");
        printf("T %-15s %10s %10s %6s\n", "name", "un-", "contended",
               "c-pct");

        // printf while holding locks makes me sad. :(
        pthread_mutex_lock(&mlist.lock);
        for (lt_mutex_t *p = mlist.head; p; p = p->next) {
                if (p->contended || p->uncontended) {
                        printf("M %-15s %10lu %10lu %5.0f%%\n", p->name,
                               p->uncontended, p->contended,
                               100.0*p->contended /
                               (p->contended + p->uncontended));
                } else {
                        printf("M %-15s %10lu %10lu %5.0f%%\n", p->name,
                               p->uncontended, p->contended,
                               0.0);
                }
        }
        pthread_mutex_unlock(&mlist.lock);

        pthread_spin_lock(&slist.lock);
        for (lt_spinlock_t *p = slist.head; p; p = p->next) {
                if (p->contended || p->uncontended) {
                        printf("S %-15s %10lu %10lu %5.0f%%\n", p->name,
                               p->uncontended, p->contended,
                               100.0*p->contended /
                               (p->contended + p->uncontended));
                } else {
                        printf("S %-15s %10lu %10lu %5.0f%%\n", p->name,
                               p->uncontended, p->contended,
                               0.0);
                }
        }
        pthread_spin_unlock(&slist.lock);
}



int
lt_mutex_init(lt_mutex_t *mutex,
              const pthread_mutexattr_t *attr,
              const char *mutex_name)
{
        mutex->uncontended = 0;
        mutex->contended = 0;
        mutex->prev = 0;
        mutex->next = 0;

        int rc = pthread_mutex_init(&mutex->lock, attr);
        if (0 != rc) {
                goto done;
        }

        // Copy mutex name so we don't have to rely on caller's memory
        if (0 == (mutex->name = strdup(mutex_name))) {
                rc = ENOMEM;
                pthread_mutex_destroy(&mutex->lock);
                goto done;
        }

        // Put this on the main list of locks
        if (0 == pthread_mutex_lock(&mlist.lock)) {
                if (mlist.head) {
                        mlist.head->prev = mutex;
                } else {
                        mlist.tail = mutex;
                }
                mutex->next = mlist.head;
                mlist.head = mutex;
                pthread_mutex_unlock(&mlist.lock);
        }

done:
        return rc;
}

int
lt_mutex_destroy (lt_mutex_t *mutex)
{
        /// @todo: We should preserve the stats by copying the struct.

        // Remove from main list of locks, but make sure it's on there first
        pthread_mutex_lock(&mlist.lock);
        if (mlist.head == mutex || mutex->prev) {
                // mutex is on the list, so remove it
                if (mutex->next) {
                        mutex->next->prev = mutex->prev;
                } else {
                        mlist.tail = mutex->prev;
                }
                if (mutex->prev) {
                        mutex->prev->next = mutex->next;
                } else {
                        mlist.head = mutex->next;
                }
        }
        pthread_mutex_unlock(&mlist.lock);

        free(mutex->name);
        return pthread_mutex_destroy(&mutex->lock);
}

int
lt_mutex_lock(lt_mutex_t *mutex)
{
        int rc = pthread_mutex_trylock(&mutex->lock);
        if (0 == rc) {
                ++mutex->uncontended;
        } else if (EBUSY == rc) {
                rc = pthread_mutex_lock(&mutex->lock);
                if (0 == rc) {
                        ++mutex->contended;
                }
        }
        return rc;
}

int
lt_mutex_trylock(lt_mutex_t *mutex)
{
        return pthread_mutex_trylock(&mutex->lock);
}

int
lt_mutex_timedlock(lt_mutex_t *mutex,
                   const struct timespec *abstime)
{
        int rc = pthread_mutex_trylock(&mutex->lock);
        if (0 == rc) {
                ++mutex->uncontended;
        } else if (EBUSY == rc) {
                rc = pthread_mutex_timedlock(&mutex->lock, abstime);
                if (0 == rc) {
                        ++mutex->contended;
                }
        }
        return rc;
}

int
lt_mutex_unlock(lt_mutex_t *mutex)
{
        return pthread_mutex_unlock(&mutex->lock);
}

int
lt_spin_init(lt_spinlock_t *lock, int pshared, const char *name)
{
        if (!slist_initialized) {
                // Don't know what to do if this fails!
                assert(0 == pthread_spin_init(&slist.lock, 1));
                slist.head = 0;
                slist.tail = 0;
                slist_initialized = true;
        }

        lock->uncontended = 0;
        lock->contended = 0;
        lock->prev = 0;
        lock->next = 0;

        int rc = pthread_spin_init(&lock->lock, pshared);
        if (0 != rc) {
                goto done;
        }

        // Copy name so we don't have to rely on caller's memory
        if (0 == (lock->name = strdup(name))) {
                rc = ENOMEM;
                pthread_spin_destroy(&lock->lock);
                goto done;
        }

        // Put this on the main list of locks
        if (0 == pthread_spin_lock(&slist.lock)) {
                if (slist.head) {
                        slist.head->prev = lock;
                } else {
                        slist.tail = lock;
                }
                lock->next = slist.head;
                slist.head = lock;
                pthread_spin_unlock(&slist.lock);
        }

done:
        return rc;
}

int
lt_spin_destroy(lt_spinlock_t *lock)
{
        /// @todo: We should preserve the stats by copying the struct.

        // Remove from main list of locks, but make sure it's on there first
        pthread_spin_lock(&slist.lock);
        if (slist.head == lock || lock->prev) {
                // lock is on the list, so remove it
                if (lock->next) {
                        lock->next->prev = lock->prev;
                } else {
                        slist.tail = lock->prev;
                }
                if (lock->prev) {
                        lock->prev->next = lock->next;
                } else {
                        slist.head = lock->next;
                }
        }
        pthread_spin_unlock(&slist.lock);

        free(lock->name);
        return pthread_spin_destroy(&lock->lock);
}

int
lt_spin_lock(lt_spinlock_t *lock)
{
        int rc = pthread_spin_trylock(&lock->lock);
        if (0 == rc) {
                ++lock->uncontended;
        } else if (EBUSY == rc) {
                rc = pthread_spin_lock(&lock->lock);
                if (0 == rc) {
                        ++lock->contended;
                }
        }
        return rc;
}

int
lt_spin_trylock(lt_spinlock_t *lock)
{
        return pthread_spin_trylock(&lock->lock);
}

int
lt_spin_unlock(lt_spinlock_t *lock)
{
        return pthread_spin_unlock(&lock->lock);
}
