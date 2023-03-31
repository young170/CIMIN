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
	if (sig == SIGALRM) {
		printf("3 sec..\n");
		exit(1);
	} else if (sig == SIGINT) {
        printf("quit..\n");
        end_program();
        exit(1);
    }
}

char * minimize_input(char * input, char * condition) {
	char * test_input = input;
	int input_length = strlen(test_input);
	int sub_length = input_length - 1;

	while (sub_length > 0) {
		for (int i = 0; i < input_length - sub_length - 1) {
			char * head, tail;

			strncpy(head, test_input, i);
			int offset = i + sub_length + 1;
			strncpy(tail, test_input + offset, input_length - 1);

			char * output;
			// char * output = program_exec(strcat(head, tail));
			if (strcmp(output, condition) == 0) {
				return minimize_input(strcat(head, tail));
			}
		}

		for (int i = 0; i < input_length - sub_length - 1) {
			char * mid;
			strncpy(mid, test_input + i, i + sub_length);

			char * output;
			// char * output = program_exec(mid);
			if (strcmp(output, condition) == 0) {
				return minimize_input(mid);
			}
		}

		sub_length = sub_length - 1;
	}

	return test_input;
}

char * delta_debug(char * input, char * condition) {
	return minimize_input(input, condition);
}

int main(int argc, char *argv[]) {
    int pid;
    int status;

    struct itimerval timer;

	int iflag = 0;
	int mflag = 0;
	int oflag = 0;
	int opt;

    // signal
    timer.it_value.tv_sec = EXEC_TIME;
	timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    setitimer(ITIMER_REAL, &timer, NULL);

    signal(SIGALRM, handle_signal);

	// input
	char crash_file[16];
	char error_string[16];
	char reduced_file[16];
	char exec_file[16];

	if (argc != 8){
			fprintf(stderr, "wrong number of arguments..\n");
			exit(1);
	}

	while ((opt = getopt(argc, argv, "imo:")) != -1) {
		switch (opt) {
			case 'i':	// -i : input file path
				iflag = 1;
				memcpy(crash_file, optarg, 16);
				break;
			case 'm':	// -m : keyword message
				mflag = 1;
				memcpy(error_string, optarg, 16);
				break;
			case 'o' :	// -o : output file path
				oflag = 1;
				memcpy(reduced_file, optarg, 16);
				break;
			case '?' :	// exception
				if (isprint (optopt))
          			fprintf (stderr, "Unknown option '-%c'.\n", optopt);
        		else
          			fprintf (stderr, "Unknown option character '\\x%x'.\n", optopt);
				return 1;
		}
	}
	memcpy(exec_file, argv[7], 16);		// ./a.out : target program

    pid = fork();
    if (pid < 0) {
        fprintf(stderr, "fork failed..\n");
        exit(1);
    } else if (pid == 0) {
        // child process executes binary file
        execl(exec_file, exec_file, NULL);
        fprintf(stderr, "child process failed..\n");
        exit(1);
    } else {
        // parent process waits for child to finish
        waitpid(pid, &status, 0);

        timer.it_value.tv_sec = 0;
        setitimer(ITIMER_REAL, &timer, NULL);
    }

    return 0;
}
