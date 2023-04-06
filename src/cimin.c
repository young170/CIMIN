#include "cimin.h"

/*
Delta debugging information in progress is stored in global_handler.
Through this, when the program terminates due to a system call, 
program can print out shortest crashing input found so far.
*/
struct handler_args {
    int kill_pid;
    int length;
    char* output_string;
    char* output_filename;
} global_handler;

void end_program() {
    // end the program
    /*
    1. cimin must stop the running test if exists,
    2. prints out the size of the shortest crashing input found so far,
    3. produces the output with it,
    4. terminates the execution.
    */
    kill(global_handler.kill_pid, SIGTERM);
    wait(NULL);

	printf("\n============================================");
	printf("\n> Program ended. Printing result data.");
	printf("\n============================================");
    printf("\n> Size : %d", global_handler.length);
    printf("\n> Minimized Output : %s\n", global_handler.output_string);
    
	write_file(global_handler.output_filename);

	return;
}

void write_file(char * output_filename) {
	char *write_string = global_handler.output_string;
    FILE *fp;

    if (write_string == NULL || output_filename == NULL) {
        fprintf(stderr, "Error: Invalid input\n");
        return;
    }

    fp = fopen(output_filename, "w");
    if (fp == NULL) {
        fprintf(stderr, "Error: Failed to open file\n");
        return;
    }

    fwrite(write_string, sizeof(char), strlen(write_string), fp);
    fclose(fp);
}

void handle_signal(int sig) {
	// if alarm signal, end program
	if (sig == SIGALRM) {
		fprintf(stderr, "\nProgram quitting due to timer alarm..");
	}
	// if interrupt signal, clean up & end program
	else if (sig == SIGINT) {
        fprintf(stderr, "\nProgram quitting due to interrupt..");
    }

	end_program();
	exit(1);
}

char* program_exec(char* input, char* exec_file, char** target_options) {
	// pipe file descriptor, 0 for reading, 1 for writing
	int to_child_FD[2];
	int to_parent_FD[2];

	pid_t child_pid;
	int status;
	
	// set timer
	struct itimerval timer;
    timer.it_value.tv_sec = EXEC_TIME;	// 3 sec
	timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &timer, NULL);

	// signal
    signal(SIGALRM, handle_signal);
	signal(SIGINT, handle_signal);

	// initialize int arrays as pipe
	if (pipe(to_child_FD) != 0) {	// non-zero == error
		perror("Pipe Error");
		exit(1);
	}
	if (pipe(to_parent_FD) != 0) {
		perror("Pipe Error");
		exit(1);
	}

	child_pid = fork();
	global_handler.kill_pid = child_pid;
	if (child_pid < 0) {
		fprintf(stderr, "fork failed..\n");
		exit(1);
	} else if (child_pid == 0) {
		// use pipe() to redirect stdin input and stderr output
		close(to_child_FD[1]);	// close write
		close(to_parent_FD[0]);	// close read

		// dup2(source, target)
		// replaces target to the sources
		// ex) dup2(pipes[1], STDOUT_FILENO) - stdout is redirected to pipes[1]
        dup2(to_child_FD[0], STDIN_FILENO);		// macro : 0
		dup2(to_parent_FD[1], STDERR_FILENO);	// macro : 2

		// child process executes binary file
		// output of stderr will be redirected to to_parent_FD[1]
		// - by the dup2(to_parent_FD[1], STDERR_FILENO)
		if (execv(exec_file, target_options) == -1) {
			fprintf(stderr, "child process failed..\n");
			exit(1);
		}
	}

	// parent process 
	close(to_child_FD[0]);	// close read
	close(to_parent_FD[1]);	// close write

	// write to pipe_to_child - stdin
	write(to_child_FD[1], input, strlen(input));

	// need to close write pipe - or else it constantly waits for input
	close(to_child_FD[1]);

	waitpid(child_pid, &status, 0);

	char* buf = (char*) malloc(sizeof(char) * BUFSIZ);
	// recieve the output of child via stderr using unnamed pipe
	int num_of_bytes = read(to_parent_FD[0], buf, BUFSIZ);
	buf[num_of_bytes] = '\0';
	// printf("%s\n", buf);

	close(to_parent_FD[0]);

	// reset timer
	timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &timer, NULL);

	return buf;
}

char* minimize_input(char* input, char* condition, char* exec_file, char** target_options) {
	char* input_placeholder = (char*) malloc(sizeof(char) * strlen(input));

	strncpy(input_placeholder, input, strlen(input));	// variable input, t
	int input_length = strlen(input_placeholder);		// length of input, |t|
	int sub_length = input_length - 1;					// length of substring of input, s = t - 1

	while (sub_length > 0) {
		for (int i = 0; i <= input_length - sub_length; i++) {
			char * head = (char*) malloc(sizeof(char) * (i + 1));
			strncpy(head, input_placeholder, i);
			head[i] = '\0';

			char* tail = (char*) malloc(sizeof(char) * (input_length - i - sub_length + 1));
			strncpy(tail, input_placeholder + i + sub_length, input_length - i - sub_length);
			tail[input_length - i - sub_length] = '\0';

			char* test_input = (char*) malloc(sizeof(char) * (i + input_length - i - sub_length + 1));
			strcpy(test_input, head);
			strcat(test_input, tail);

			printf("head: %s, tail: %s, test_input: %s\n", head, tail, test_input);

			char* output = program_exec(test_input, exec_file, target_options);	// output = p(head + tail)
			if (strstr(output, condition) != NULL) {	// condition satisfied
				printf("condition satisfied, current input(head+tail): %s\n", test_input);
				// save global data, for SIGINT
				global_handler.length = strlen(test_input);
				global_handler.output_string = test_input;

				char* updated_input = minimize_input(test_input, condition, exec_file, target_options);	// minimize_input(head + tail)
				free(input_placeholder);
				input_placeholder = updated_input;

				free(test_input);
				free(head);
				free(tail);
				free(output);
				return input_placeholder;
			}

			free(output);
			free(test_input);
			free(head);
			free(tail);
		}

		for (int i = 0; i <= input_length - sub_length; i++) {
			char* mid = (char*) malloc(sizeof(char) * (sub_length + 1));
			strncpy(mid, input_placeholder + i, sub_length);//mid = tm[i..i+s-1]
			mid[sub_length] = '\0';
			printf("mid: %s\n", mid);
			char* output = program_exec(mid, exec_file, target_options);	// output = p(head + tail)
			if (strstr(output, condition) != NULL) {	// condition satisfied
				printf("condition satisfied, current input(mid): %s\n", mid);
				// save global data, for SIGINT
				global_handler.length = strlen(mid);
				global_handler.output_string = mid;

				char* updated_input = minimize_input(mid, condition, exec_file, target_options);	// minimize_input(head + tail)
				free(input_placeholder);
				input_placeholder = updated_input;

				free(mid);
				free(output);
				return input_placeholder;
			}

			free(output);
			free(mid);
		}

		sub_length = sub_length - 1;
	}

	return input_placeholder;
}

char* delta_debug(char* input, char* condition, char* exec_file, char** target_options) {
	return minimize_input(input, condition, exec_file, target_options);
}

char* file_data(char* filepath) {
    FILE* fp;
    char* buffer = NULL;
    long file_length;

    fp = fopen(filepath, "rb"); // use file path specified by -i option

    // check if file exists
    if (fp == NULL) {
        fprintf(stderr, "Error opening file..\n");

        return NULL;
    }

    // file length
    fseek(fp, 0L, SEEK_END);
    file_length = ftell(fp);
    rewind(fp);

    // malloc buffer
    buffer = malloc(sizeof(char) * file_length);
    if (buffer == NULL) {
        fclose(fp);
        fprintf(stderr, "Error allocating memory..\n");

        return NULL;
    }

	// read contents
    size_t bytes_read = fread(buffer, 1, file_length, fp);

	if (bytes_read != file_length) {
        fclose(fp);
        free(buffer);
        fprintf(stderr, "Error reading file..\n");
        return NULL;
    }
    fclose(fp);

    return buffer;
}

int main(int argc, char* argv[]) {
	// flags of input options
	// 0 : false, 1 : true
	int iflag = 0;
	int mflag = 0;
	int oflag = 0;
	int opt;

    // input
	char* crash_file;
	char* error_string;
	char* reduced_file;
	char** target_options;

	if (argc <= 7) {
		fprintf(stderr, "Input options count..\n");
		return 1;
	}

	while ((opt = getopt(7, argv, "i:m:o:")) != -1) {
		switch (opt) {
			case 'i':	// -i : input file path
				crash_file = (char*) malloc(sizeof(char) * (strlen(optarg) + 2));
				optarg[strlen(optarg)] = '\0';
				strcpy(crash_file, optarg);

				iflag = 1;
				break;
			case 'm':	// -m : keyword message
				error_string = (char*) malloc(sizeof(char) * (strlen(optarg) + 2));
				optarg[strlen(optarg)] = '\0';
				strcpy(error_string, optarg);

				mflag = 1;
				break;
			case 'o' :	// -o : output file path
				reduced_file = (char*) malloc(sizeof(char) * (strlen(optarg) + 2));
				optarg[strlen(optarg)] = '\0';
				strcpy(reduced_file, optarg);

				global_handler.output_filename = reduced_file;

				oflag = 1;
				break;
			case '?' :	// exception
          		fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
				return 1;
			default:
				fprintf(stderr, "Error option");
            	return 1;
		}
	}

	if (iflag == 0 | mflag == 0 | oflag == 0) {
		fprintf(stderr, "Error: missing option..\n");
		exit(1);
	}

	// length of options following the target binary exec file
	int length_of_target_arg = argc - optind;
	target_options = (char**) malloc(sizeof(char*) * (length_of_target_arg + 1));	// space for NULL

	for (int i = 0; i < length_of_target_arg; i++) {
		target_options[i] = (char*) malloc(sizeof(char) * strlen(argv[optind + i]));
		strcpy(target_options[i], argv[optind + i]);
	}
	target_options[length_of_target_arg] = NULL; // end of argument when running execv

	// main process
	char * crash_data = file_data(crash_file);	// read contents from crash input filepath

	global_handler.output_string = crash_data;
	global_handler.length = strlen(crash_data);

	char * output_exec = delta_debug(crash_data, error_string, target_options[0], target_options);

	global_handler.output_string = output_exec;
	global_handler.length = strlen(output_exec);
	write_file(global_handler.output_filename);

	// free char*
	free(crash_file);
	free(error_string);
	free(reduced_file);
	free(crash_data);
	free(output_exec);

	// free char**
	for (int i = 0; i < sizeof(target_options) / sizeof(char*); i++) {
		free(target_options[i]);
	}
	free(target_options);

    return 0;
}
