#include "locktrack.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

struct {
        pthread_mutex_t lock;
        lt_mutex_t *head;
        lt_mutex_t *tail;
} llist = {PTHREAD_MUTEX_INITIALIZER, 0, 0};

int
lt_mutex_init(lt_mutex_t *mutex,
              const pthread_mutexattr_t *attr,
              const char *mutex_name)
{
        mutex->uncontended = 0;
        mutex->contended = 0;
        mutex->prev = 0;
        mutex->next = 0;

        int rc = pthread_mutex_init(&mutex->mutex, attr);
        if (0 != rc) {
                goto done;
        }

        // Copy mutex name so we don't have to rely on caller's memory
        if (0 == (mutex->name = strdup(mutex_name))) {
                rc = ENOMEM;
                pthread_mutex_destroy(&mutex->mutex);
                goto done;
        }

        // Put this on the main list of locks
        if (0 == pthread_mutex_lock(&llist.lock)) {
                if (llist.head) {
                        llist.head->prev = mutex;
                } else {
                        llist.tail = mutex;
                }
                mutex->next = llist.head;
                llist.head = mutex;
                pthread_mutex_unlock(&llist.lock);
        }

done:
        return rc;
}

int
lt_mutex_destroy (lt_mutex_t *mutex)
{
        // XXX: We should probably preserve the info by copying the struct.

        // Remove from main list of locks.
        pthread_mutex_lock(&llist.lock);
        if (llist.head == mutex || mutex->prev) {
                // mutex is on the list, so remove it
                if (mutex->next) {
                        mutex->next->prev = mutex->prev;
                } else {
                        llist.tail = mutex->prev;
                }
                if (mutex->prev) {
                        mutex->prev->next = mutex->next;
                } else {
                        llist.head = mutex->next;
                }
        }
        pthread_mutex_unlock(&llist.lock);

        free(mutex->name);
        return pthread_mutex_destroy(&mutex->mutex);
}

int
lt_mutex_lock(lt_mutex_t *mutex)
{
        int rc = pthread_mutex_trylock(&mutex->mutex);
        if (0 == rc) {
                ++mutex->uncontended;
        } else if (EBUSY == rc) {
                rc = pthread_mutex_lock(&mutex->mutex);
                if (0 == rc) {
                        ++mutex->contended;
                }
        }
        return rc;
}

int
lt_mutex_trylock(lt_mutex_t *mutex)
{
        return pthread_mutex_trylock(&mutex->mutex);
}

int
lt_mutex_unlock(lt_mutex_t *mutex)
{
        return pthread_mutex_unlock(&mutex->mutex);
}
