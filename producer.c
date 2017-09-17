#include "prod-cons.h"

char in_file_path[PATH_MAX + 1];
int num_shared_bufs, in_path_len;

/* really need messages and re-think method of exiting 
   from a function. */

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
		/* if tmp == num - written we have a potential overflow */
		void *old_cursor = cursor;
		*(uint64_t *)old_cursor = len; 
		written += (len + sizeof(len));
		cursor += (len + sizeof(len));
		printf ("Line %4d: %s", i++, old_cursor + sizeof(uint64_t));
//		printf("written %d, remaining %d\n", written, num - written);
	}
	return i;
}

// prefer throughput when a decision needs to be made
// can I simply map the input file to shared memory
//      (instead of streaming/copying)? NO, don't
// additional rules to consider:
//   buffer format - length, how many bits?, alignment?
// any rule about sentences? length?
// buffer = 1k
// map = 4 buffers
// header-footer issues: header should specify
//    the number of sentences, there should be an
//    end marker for the buf.
// first sentence will be a normal sentence but contain
// a header for the shared buff - e.g.:
// "349" "num lines, end marker, any other useful stuff

int main(int argc, char **argv) 
{
	
	FILE *fp;
	int ccode = EXIT_FAILURE;
	
	/* this next call initializes global vars for us */
	check_and_get_args(argc, argv);
	/* now in_file_path and num_shared_bufs are initialized */

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

	ftruncate(fd, MAP_SIZE);
	
	void *shared_buf = mmap(0, MAP_SIZE,
							PROT_READ | PROT_WRITE, MAP_SHARED,
							fd, 0);
	
	if (shared_buf == MAP_FAILED) {
		perror("mapping shared buf - "
			   "is the mapping already created by another process?");
		ccode = EXIT_FAILURE;
		goto exit_input_file;
	}

	ccode = dump_file(fp, shared_buf, BUFSIZE);
	ccode = 0;
	
exit_shm:
//	shm_unlink("the_untrusted_one");
exit_input_file:
	fclose(fp);
	return ccode;
}
