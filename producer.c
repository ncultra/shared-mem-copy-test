#include "prod-cons.h"

char in_file_path[PATH_MAX + 1];
int num_shared_bufs, in_path_len;

void check_and_get_args(int __argc, char **__argv)
{

	/* arrays are zero-indexed, others 1-indexed. */
	if (__argc - 1 != NARGS) { 
		exit(EXIT_FAILURE);
	}

	num_shared_bufs = strtol(__argv[1], NULL, 10);
	if (num_shared_bufs < 0 || num_shared_bufs > MAX_BUFS) {
		exit(EXIT_FAILURE);
	}
	
	in_path_len = strnlen(__argv[2], PATH_MAX);
	if  (in_path_len == 0x00 || in_path_len == PATH_MAX) {
		exit(EXIT_FAILURE);
	}
	strncpy(in_file_path, __argv[2], in_path_len);
	return;
}



/* sentence format is uint64_t sent_len char sentence[sent_len] */
/* write sentence into cursor * + sizeof(uint64_t), then go */
/* back and write the length of the sentence at cursor */
/* keep track of the total length written */
/* also: check input parameters */

int dump_file(FILE *fp, void *buf, int num)
{

	int written = 0, i = 0;
	void *cursor =  buf;

/* validate input here */

	fseek(fp, 0, SEEK_SET);
	
	while (num - written > 2 * sizeof(uint64_t) &&
		   fgets(cursor + sizeof(uint64_t), num - written, fp) != NULL)
	{
		uint64_t len  = strnlen(cursor + sizeof(uint64_t), num - written);
		void *old_cursor = cursor;
		*(uint64_t *)old_cursor = len; 
		written += (len + sizeof(len));
		cursor += (len + sizeof(len));
		printf ("Line %4d: %s", i++, old_cursor + sizeof(uint64_t));
	}
	return i;
}

int main(int argc, char **argv) 
{
	
	FILE *fp;
	int ccode = EXIT_FAILURE;
	int map_size;
	
	/* this next call initializes global vars for us */
	check_and_get_args(argc, argv);

	/* now in_file_path and num_shared_bufs are initialized */
	map_size = num_shared_bufs * BUFSIZE;

    /* map is always going to be rounded up to page size */
	if (map_size % PAGE_SIZE) {
		map_size += (PAGE_SIZE - (map_size % PAGE_SIZE));
	}
	
	fp = fopen(in_file_path, "r") ;
	if (fp == NULL) {
		perror("error opening file - bad path?");
		exit(EXIT_FAILURE);
	}

	/* all appears well, go ahead and map memory */
	/* read from file, copy to mapped memory */
	
 	int fd = shm_open("\\the_untrusted_one", O_CREAT | O_RDWR, 0666);
	if (fd <= 0) {
		perror("untrusted perducer: ");
		exit(EXIT_FAILURE);
	}

	ftruncate(fd, map_size);
	
	void *shared_buf = mmap(0, map_size,
							PROT_READ | PROT_WRITE, MAP_SHARED,
							fd, 0);
	
	if (shared_buf == MAP_FAILED) {
		perror("mapping shared buf - "
			   "is the mapping already created by another process?");
		ccode = EXIT_FAILURE;
		goto exit_input_file;
	}
	
/* sem values:
 * 0 empty, writeable - don't read the shared buffer
 * 1 buffer contents are readable
 */
	struct buf_head *h = shared_buf;
//	uint64_t *sem = shared_buf;
    /* warn readers to stay away  right now */
	__atomic_store_n(&h->sem, 0, __ATOMIC_SEQ_CST);

	ccode = dump_file(fp, shared_buf + sizeof(struct buf_head),
					  map_size - sizeof(struct buf_head));
     /*  kick the readers */
	__atomic_store_n(&h->sem, 1, __ATOMIC_SEQ_CST);
	
    /* now we want to wait for consumer to read, */
	while ( __atomic_load_n(&h->sem, __ATOMIC_SEQ_CST) == 1 ) {
		sched_yield();
	}
	printf ("i made it too\n");
	ccode = 0;
	
exit_input_file:
	fclose(fp);
	return ccode;
}
