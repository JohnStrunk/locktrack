#include "locktrack.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

int
lt_mutex_init(lt_mutex_t *mutex,
              const pthread_mutexattr_t *attr,
              const char *mutex_name)
{
        mutex->uncontended = 0;
        mutex->contended = 0;
        if ((mutex->name = strdup(mutex_name)) == 0) {
                free(mutex);
                mutex = 0;
                return ENOMEM;
        }

        return pthread_mutex_init(&mutex->mutex, attr);
}

int
lt_mutex_destroy (lt_mutex_t *mutex)
{
        int rc = pthread_mutex_destroy(&mutex->mutex);
        free(mutex->name);
        return rc;
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
