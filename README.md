# Overview
This is a library for experimenting with tracking lock contention.

# Building
The sources are built using cmake, so:

    $ mkdir build
    $ cd build
    $ cmake ..
    $ make

This will build the library, a simple benchmark program, and the documentation
(Doxygen).

# Usage
Include the `locktrack.h` file in your source, then use `lt_*` functions and
types as you would `pthread_*` functions & types. Link your code against
`liblocktrack.a` and pthreads.

## Code example
To use the mutex versions:
```c
// Declare it
lt_mutex_t my_mutex;

// initialize like you would a pthread mutex, but adding a name
int rc = lt_mutex_init(&my_mutex, 0, "My mutex");

// Lock & unlock
int rc = lt_mutex_lock(&my_mutex);
int rc = lt_mutex_unlock(&my_mutex);

// Dump the stats before you destroy it
lt_stats_print();

// Clean up after yourself :)
int lt_mutex_destroy(&my_mutex);
```

## Example output
To get the contention stats, call `lt_stats_print()`. Here's an example of the
output:
```
=== Tracking stats ===
T name                   un-  contended  c-pct
M mutex4                   0          0     0%
M mutex3                   0          0     0%
M mutex2                   0          0     0%
M mutex1                   0          0     0%
M mutex0                   0          0     0%
M my mutex         270529967  129470033    32%
S spin4                    0          0     0%
S spin3                    0          0     0%
S spin2                    0          0     0%
S spin1                    0          0     0%
S spin0                    0          0     0%
S my spin         1997572146    2427854     0%
```
From the above, we see that only "my mutex" was regularly contended (32% of the
time).

# Overheads
The mutex and lock structures are larger because of the tracking information. We
are currently tracking counters for contended and uncontended access as well as
a name for each lock.

The following is the performance on my 4-core laptop. The test program is just
incrementing a counter as fast as possible, so this should show nearly the
maximum amount of locking overhead.

The first run is with the number of threads equal to the number of cores. The
tracked locks shouldn't actually be faster and is likely due to cache line
effects.
```
$ ./benchmarks/lockbench -i 100000000 -t 4
Test parameters: threads=4 iterations=100000000
mutex size increase: 40 ==> 80: 100%
mutex slowdown: 46.728 ==> 42.133: -10%
spinlock size increase: 4 ==> 48: 1100%
spinlock slowdown: 43.724 ==> 39.600: -9%
```

This run is with a single thread (so no contention).
```
$ ./benchmarks/lockbench -i 1000000000 -t 1
Test parameters: threads=1 iterations=1000000000
mutex size increase: 40 ==> 80: 100%
mutex slowdown: 18.386 ==> 17.318: -6%
spinlock size increase: 4 ==> 48: 1100%
spinlock slowdown: 43.937 ==> 52.350: 19%
```
