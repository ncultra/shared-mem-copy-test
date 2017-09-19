#include "prod-cons.h"

char in_search_string[PATH_MAX + 1];
int num_shared_bufs, in_str_len;

/* need to make this inline'd in h file. */
void check_and_get_args(int __argc, char **__argv)
{

/* we will take one or two arguments */
	if (__argc - 1 != NARGS && __argc - 1 != NARGS - 1) { 
		exit(EXIT_FAILURE);
	}
	num_shared_bufs = strtol(__argv[1], NULL, 10);
	if (num_shared_bufs < 0 || num_shared_bufs > MAX_BUFS) {
		exit(EXIT_FAILURE);
	}

	/* if we have a second parameter */
	if (__argc - 1 == NARGS) {
		
		in_str_len = strnlen(__argv[2], PATH_MAX);
		if  (in_str_len == 0x00 || in_str_len == PATH_MAX) {
			exit(EXIT_FAILURE);
		}
		strncpy(in_search_string, __argv[2], in_str_len);
	} else {
		strcpy(in_search_string, "");
	}
	
	return;
}

/* read share buffer and unpack into text format, 
 * output to file handle.
 * empty substring "" matches all lines
 */
int buffer_to_file(FILE *fp, void *buf, int num, char *substring)
{
	int read = 0, i = 0, ccode = -1;
	void *cursor = buf;
	void *end = buf + num;
	
	while (read + (2 * sizeof(uint64_t)) < num &&
		   cursor + (2 * sizeof(uint64_t)) < end)

	{
		/* read the length, check for safety, write 'len' bytes to file
		 * advance cursor, repeat 
		 */

		uint64_t len = *(uint64_t *)cursor;
		
		if (len >= (num - read)) {
			/*	would overflow */
			printf("we read some data the was inconsistent with your reputation\n");
			printf("length of next sentence: %d, avail buffer space: %d\n",
				   len, num - read);
			ccode = -1;
			goto err_out;
		}

		/* if substring is empty, match everything */
		char *pos = strstr(cursor + sizeof(uint64_t), substring);
		if (pos) {
			fwrite(cursor + sizeof(uint64_t), sizeof(char), len, fp);			
		}
		cursor += (len + sizeof(len));
		read += (len + sizeof(len));
	}
	ccode = 0;
err_out:
	/* do any of the error paths need to sync using the 0/1 sem? */
	return ccode;
}


/* bare-bones synchronization 
 * first byte of the shared buffer is a semaphore.
 * can only write the the buffer if the first byte is zero,
 * can only read when the first byte is one.
 * shm header: 
 * uint64_t semaphore
 *  
 */

int main(int argc, char **argv)
{
	check_and_get_args(argc, argv);
	int fd = shm_open("\\the_untrusted_one", O_RDWR, 0666);
	if (fd <= 0) {
		perror("untrusted perducer-consumer: ");
		exit(EXIT_FAILURE);
	}
	void *buf = mmap(0, BUFSIZ, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (buf == MAP_FAILED) {
		perror("memory map: ");
		exit (EXIT_FAILURE);
	}
	/* synchronize with other users */
	uint64_t *sem = buf;

	/* wait until its ok to read from the shared buf */
	while ( __atomic_load_n(sem, __ATOMIC_SEQ_CST) != 1) {
		sched_yield();
	}
	
	int ccode = buffer_to_file(stdout, buf + sizeof(*sem),
							   BUFSIZE - sizeof(*sem),
							   in_search_string);

    __atomic_store_n(sem, 0, __ATOMIC_SEQ_CST);
	
	printf("i made it!\n");

	/* remove the shared memory segment */
	if (shm_unlink("\\the_untrusted_one") == -1) {
		printf("Error removing %s\n", "\\the_untrusted_one");
		exit(-1);
	}

	return 0;
}
