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
#define PAGE_SIZE 4096
#define BUFSIZE 1024
#define MAX_BUFS 400

struct buf_head {
	uint64_t sem;
	void *mm;
};

