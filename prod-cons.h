#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <limits.h>
#include <sched.h>

#define NARGS 2
#define MAP_SIZE 4096
#define BUFSIZE 1024
#define MAX_BUFS 4

/**********************
 * v1 improvements
 *  1 intrinsics for synchronization
 *  2 formal header for shmem buf (msg box, state)
 *  3 make name of shared mem handle configurable
  * 
 * if this is ever scaled ...
 * Consider setting up mapped buffer pairs for "lockless" updating.
 *    2x a r/w pair. 4 total shared buffers.
 *    or a larger number of total shared buffers, specialized i/o ordering
 * RCU-like, with changes written to a background copy of memory 
 * and merged into the foreground (original) as soon as exclusive 
 * is achived. 
 */

/* v2 improvemements - atomic buffer sharing 
 * share more common constructs using header
 * 
 */

/*
 * version 3 - "rcu memory mappings" ?? 
 */


// buffer = 1k
// map = 4 buffers
// first sentence will be a normal sentence but contain
// a header for the shared buff - e.g.:
// "349" "num lines, end marker, any other useful stuff
