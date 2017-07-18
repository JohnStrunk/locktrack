#include "locktrack.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

const long SPIN_ITER_INCREASE = 5l;

unsigned long shared_var = 0ul;
lt_mutex_t ltmutex;
pthread_mutex_t pthmutex;
lt_spinlock_t ltspin;
pthread_spinlock_t pthspin;

void *
ltmworker(void *arg_iter)
{
        long iters = *(long *)arg_iter;
        while (iters--) {
                lt_mutex_lock(&ltmutex);
                ++shared_var;
                lt_mutex_unlock(&ltmutex);
        }
        return 0;
}

void *
pthmworker(void *arg_iter)
{
        long iters = *(long *)arg_iter;
        while (iters--) {
                pthread_mutex_lock(&pthmutex);
                ++shared_var;
                pthread_mutex_unlock(&pthmutex);
        }
        return 0;
}

void *
ltsworker(void *arg_iter)
{
        long iters = *(long *)arg_iter;
        while (iters--) {
                lt_spin_lock(&ltspin);
                ++shared_var;
                lt_spin_unlock(&ltspin);
        }
        return 0;
}

void *
pthsworker(void *arg_iter)
{
        long iters = *(long *)arg_iter;
        while (iters--) {
                pthread_spin_lock(&pthspin);
                ++shared_var;
                pthread_spin_unlock(&pthspin);
        }
        return 0;
}

double
time_get()
{
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return ts.tv_sec + (double)ts.tv_nsec/1e9;
}

void
usage(char *pname)
{
        printf("%s -i <iterations> -t <threads>\n", pname);
}

double
timed_benchmark(void *(*worker)(void*), int threads, long iters)
{
        pthread_t *thread = malloc(sizeof(pthread_t) * threads);
        double start = time_get();
        for (int i = 0; i < threads; ++i) {
                pthread_create(&(thread[i]), 0, worker, &iters);
        }
        for (int i = 0; i < threads; ++i) {
                pthread_join(thread[i], 0);
        }
        double end = time_get();
        free(thread);
        return end - start;
}

int
main(int argc, char *argv[])
{
        long iters = 0;
        int threads = 0;
        char opt;
        while ((opt = getopt(argc, argv, "i:t:")) != -1) {
                switch (opt) {
                        case 'i':
                                iters = atol(optarg);
                                break;
                        case 't':
                                threads = atoi(optarg);
                                break;
                        default:
                                usage(argv[0]);
                                return 1;
                }
        }

        if (0 >= iters || 0 >= threads) {
                usage(argv[0]);
                return 1;
        }
        printf("Test parameters: threads=%d iterations=%ld\n", threads,
               iters);

        double pelapsed;
        double telapsed;


        // Test mutexes
        printf("mutex size increase: %lu ==> %lu: %.0f%%\n",
               sizeof(pthread_mutex_t), sizeof(lt_mutex_t),
               100.0 * (sizeof(lt_mutex_t) - sizeof(pthread_mutex_t)) /
               sizeof(pthread_mutex_t));

        shared_var = 0ul;
        pthread_mutex_init(&pthmutex, 0);
        pelapsed = timed_benchmark(pthmworker, threads, iters);
        pthread_mutex_destroy(&pthmutex);
        assert(shared_var == iters * threads);

        shared_var = 0ul;
        lt_mutex_init(&ltmutex, 0, "my mutex");
        telapsed = timed_benchmark(ltmworker, threads, iters);
        assert(shared_var == iters * threads);

        printf("mutex slowdown: %.3f ==> %.3f: %.0f%%\n", pelapsed, telapsed,
               100.0*(telapsed - pelapsed)/pelapsed);


        // Test spinlocks
        printf("spinlock size increase: %lu ==> %lu: %.0f%%\n",
               sizeof(pthread_spinlock_t), sizeof(lt_spinlock_t),
               100.0 * (sizeof(lt_spinlock_t) - sizeof(pthread_spinlock_t)) /
               sizeof(pthread_spinlock_t));

        shared_var = 0ul;
        pthread_spin_init(&pthspin, 0);
        pelapsed = timed_benchmark(pthsworker, threads,
                                   SPIN_ITER_INCREASE * iters);
        pthread_spin_destroy(&pthspin);
        assert(shared_var == iters * threads * SPIN_ITER_INCREASE);

        shared_var = 0ul;
        lt_spin_init(&ltspin, 0, "my spin");
        telapsed = timed_benchmark(ltsworker, threads,
                                   SPIN_ITER_INCREASE*iters);
        assert(shared_var == iters * threads * SPIN_ITER_INCREASE);

        printf("spinlock slowdown: %.3f ==> %.3f: %.0f%%\n", pelapsed,
               telapsed, 100.0*(telapsed - pelapsed)/pelapsed);


        // Create some dummy locks just for show
        lt_mutex_t dm[5];
        lt_spinlock_t ds[5];
        for (unsigned i = 0; i < 5; ++i) {
                char buf[10];
                sprintf(buf, "mutex%u", i);
                lt_mutex_init(&dm[i], 0, buf);
                sprintf(buf, "spin%u", i);
                lt_spin_init(&ds[i], 0, buf);
        }

        // print stats about all the locks
        lt_stats_print();

        for (unsigned i = 0; i < 5; ++i) {
                lt_mutex_destroy(&dm[i]);
                lt_spin_destroy(&ds[i]);
        }

        lt_mutex_destroy(&ltmutex);
        lt_spin_destroy(&ltspin);
        return 0;
}
