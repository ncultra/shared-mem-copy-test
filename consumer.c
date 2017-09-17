#include "prod-cons.h"

char in_search_string[PATH_MAX + 1];
int num_shared_bufs, in_str_len;

/* need to make this inline'd in h file. */
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
	
	in_str_len = strnlen(__argv[2], PATH_MAX);
	if  (in_str_len == 0x00 || in_str_len == PATH_MAX) {
		exit(EXIT_FAILURE);
	}
	strncpy(in_search_string, __argv[2], in_str_len);
	return;
}

/* read share buffer and unpack into text format, 
 * output to file handle.
 */
int buffer_to_file(FILE *fp, void *buf, int num)
{
	int read = 0, i = 0;
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
			goto bad_out;
		}
		
		fwrite(cursor + sizeof(uint64_t), sizeof(char), len, fp);
		cursor += (len + sizeof(len));
		read += (len + sizeof(len));
	}

	return 0;
	
bad_out:

	return -1;
	
}





int main(int argc, char **argv)
{
	check_and_get_args(argc, argv);
	int fd = shm_open("\\the_untrusted_one", O_RDONLY, 0444);
	if (fd <= 0) {
		perror("untrusted perducer-consumer: ");
		exit(EXIT_FAILURE);
	}
	void *buf = mmap(0, BUFSIZ, PROT_READ, MAP_SHARED, fd, 0);
	if (buf == MAP_FAILED) {
		perror("memory map: ");
		exit (EXIT_FAILURE);
	}
	int ccode = buffer_to_file(stdout, buf,  BUFSIZE);
	
	printf("we made it!\n");

	/* remove the shared memory segment */
	if (shm_unlink("\\the_untrusted_one") == -1) {
		printf("Error removing %s\n", "\\the_untrusted_one");
		exit(-1);
	}

	return 0;
}
