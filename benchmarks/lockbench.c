#include "locktrack.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

lt_mutex_t mutex;
unsigned long shared_var = 0ul;

void *
worker(void *arg_iter)
{
        long iters = *(long *)arg_iter;
        //printf("iters: %ld\n", iters);

        while (iters--) {
                lt_mutex_lock(&mutex);
                ++shared_var;
                lt_mutex_unlock(&mutex);
        }

        return 0;
}

void
usage(char *pname)
{
        printf("%s -i <iterations> -t <threads>\n", pname);
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

        printf("pthread_mutex_t size: %lu\n", sizeof(pthread_mutex_t));
        printf("lt_mutex_t size: %lu\n", sizeof(lt_mutex_t));
        lt_mutex_init(&mutex, 0, "my mutex");

        pthread_t *thread = malloc(sizeof(pthread_t) * threads);
        for (int i = 0; i < threads; ++i) {
                pthread_create(&(thread[i]), 0, worker, &iters);
        }

        for (int i = 0; i < threads; ++i) {
                pthread_join(thread[i], 0);
        }
        free(thread);

        printf("contended=%lu  uncontended=%lu\n", mutex.contended,
               mutex.uncontended);

        lt_mutex_destroy(&mutex);

        assert(shared_var == iters * threads);

        return 0;
}
