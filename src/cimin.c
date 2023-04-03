#include "cimin.h"

void end_program() {
    // end the program
    /*
    cimin must stop the running test if exists,
    and prints out the size of the shortest crashing input found so far,
    produces the output with it, and terminates the execution.
    */
	return;
}

void handle_signal(int sig) {
	// if alarm signal, end program
	if (sig == SIGALRM) {
		fprintf(stderr, "Execution time passed 3 sec..\n");
		exit(1);
	}
	// if interrupt signal, clean up & end program
	else if (sig == SIGINT) {
        fprintf(stderr, "\nProgram quitting due to interrupt..\n");
        end_program();
        exit(1);
    }
}

char * program_exec(char ** input, char * exec_file, char ** target_options) {
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
	int j = 0;
    while (input[j] != NULL) {
        write(to_child_FD[1], input[j], strlen(input[j]));
        j++;
    }

	// need to close write pipe - or else it constantly waits for input
	close(to_child_FD[1]);

	waitpid(child_pid, &status, 0);

	char buf[BUFSIZ];
	// recieve the output of child via stderr using unnamed pipe
	read(to_parent_FD[0], buf, BUFSIZ);
	printf("%s\n", buf);	// intentional - print stderr to console
	close(to_parent_FD[0]);

	return buf;
}

// char ** minimize_input(char ** input, char * condition, char * exec_file, char * target_options[]) {
// 	char ** test_input = input;				// variable input
// 	int input_length = strlen(test_input);	// length of input
// 	int sub_length = input_length - 1;		// length of substring of input

// 	while (sub_length > 0) {
// 		for (int i = 0; i < input_length - sub_length; i++) { 
// 			char * head, tail;

// 			strncpy(head, test_input, i);	// [0..i-1]
// 			int offset = i + sub_length;	// [i+s..
// 			strncpy(tail, test_input + offset, input_length);	// ..|t|-1]

// 			char * output = program_exec(strcat(head, tail), condition, exec_file, target_option);

// 			// if the output of stderr contains the error keyword condition
// 			if (strstr(output, condition) != NULL) {
// 				return minimize_input(strcat(head, tail));
// 			}
// 		}

// 		for (int i = 0; i < input_length - sub_length - 1) {
// 			char * mid;
// 			strncpy(mid, test_input + i, i + sub_length);	// [i..i+s-1]

// 			char * output = program_exec(mid, condition, exec_file, target_option);
// 			if (strstr(output, condition) != NULL) {
// 				return minimize_input(mid);
// 			}
// 		}

// 		// decrement substring by 1
// 		sub_length = sub_length - 1;
// 	}
	
// 	return test_input;
// }

// char ** delta_debug(char ** input, char * condition, char * exec_file, char ** target_options) {
// 	return minimize_input(input, condition, exec_file, target_options);
// }

char ** file_data(char * filepath) {
    FILE *fp;
    char ** buffer = NULL;
    long file_length;

    fp = fopen(filepath, "r"); // use file path specified by -i option

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
    buffer = malloc(sizeof(char *));
    if (buffer == NULL) {
        fclose(fp);
        fprintf(stderr, "Error allocating memory..\n");

        return NULL;
    }

    // read contents
    int i = 0;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, fp)) != -1) {
        buffer = realloc(buffer, sizeof(char *) * (i + 1)); // allocate memory for a new pointer to char

        if (buffer == NULL) {
            fclose(fp);
            printf("Error allocating memory\n");

            return NULL;
        }

        buffer[i] = malloc(sizeof(char) * (read + 1)); // allocate memory for line

        if (buffer[i] == NULL) {
            fclose(fp);
            fprintf(stderr, "Error allocating memory\n");

            return NULL;
        }

        strcpy(buffer[i], line); // copy line to buffer
        i++;
    }
    fclose(fp);

    if (line)
		free(line); // free memory allocated by getline()

	buffer[i - 1][strlen(buffer[i - 1]) - 1] = '\0';

    return buffer;
}

int main(int argc, char *argv[]) {
	// flags of input options
	// 0 : false, 1 : true
	int iflag = 0;
	int mflag = 0;
	int oflag = 0;
	int opt;

    // input
	char * crash_file;
	char * error_string;
	char * reduced_file;
	char ** target_options;

	if (argc <= 7) {
		fprintf(stderr, "Input options count..\n");
		return 1;
	}

	while ((opt = getopt(7, argv, "i:m:o:")) != -1) {
		switch (opt) {
			case 'i':	// -i : input file path
				crash_file = (char *) malloc(sizeof(char) * strlen(optarg));
				strcpy(crash_file, optarg);
				crash_file[strlen(crash_file)] = '\0';

				iflag = 1;
				break;
			case 'm':	// -m : keyword message
				error_string = (char *) malloc(sizeof(char) * strlen(optarg));
				strcpy(error_string, optarg);

				mflag = 1;
				break;
			case 'o' :	// -o : output file path
				reduced_file = (char *) malloc(sizeof(char) * strlen(optarg));
				strcpy(reduced_file, optarg);

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

	// length of options following the target binary exec file
	int length_of_target_arg = argc - optind;
	target_options = (char **) malloc(sizeof(char) * (length_of_target_arg+1));

	for (int i = 0; i < length_of_target_arg; i++) {
		target_options[i] = (char *) malloc(sizeof(char) * strlen(argv[optind + i]));
		strcpy(target_options[i], argv[optind + i]);
	}
	target_options[length_of_target_arg] = NULL; // end of argument when running execv

	// main process
	char ** crash_data = file_data(crash_file);	// read contents from crash input filepath
	char* output_exec = program_exec(crash_data, target_options[0], target_options);
	// char * output_result = delta_debug(crash_data, error_string, exec_file, target_options);



	// free char*
	free(crash_file);
	free(error_string);
	free(reduced_file);

	// free char**
	for (int i = 0; i < sizeof(target_options) / sizeof(char *); i++) {
		free(target_options[i]);
	}
	free(target_options);

	// free char**
	for (int i = 0; i < sizeof(crash_data) / sizeof(char *); i++) {
		free(crash_data[i]);
	}
	free(crash_data);

    return 0;
}
