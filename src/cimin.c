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
        fprintf(stderr, "Program quitting due to interrupt..\n");
        end_program();
        exit(1);
    }
}

char * minimize_input(char * input, char * condition) {
	char * test_input = input;				// variable input
	int input_length = strlen(test_input);	// length of input
	int sub_length = input_length - 1;		// length of substring of input

	// pipe init
	int pipe_fd[2];
	if (pipe(pipe_fd) == -1) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	while (sub_length > 0) {
		for (int i = 0; i < input_length - sub_length - 1) {
			char * head, tail;

			strncpy(head, test_input, i);	// [0..i-1]
			int offset = i + sub_length;	// [i+s..
			strncpy(tail, test_input + offset, input_length);	// ..|t|-1]

			char * output;
			// char * output = program_exec(strcat(head, tail));

			/*
			/////////////////////////////////////////////////////
			use fork() and execl() to get output
			/////////////////////////////////////////////////////
			int pid = fork;
			if (pid < 0) {
				fprintf(stderr, "fork failed..\n");
				exit(1);
			} else if (pid == 0) {
				// child process executes binary file
				// use pipe() to redirect stdin input
				execl(exec_file, ???, NULL);
				fprintf(stderr, "child process failed..\n");
				exit(1);
			} else {
				// parent process waits for child to finish
				waitpid(pid, &status, 0);	// need time also as global var

				/////////////////////////////////////////////////////
				recieve the output of child via stderr using unnamed pipe
				/////////////////////////////////////////////////////
				close(pipe_fd[1]);
				char buf[BUFSIZ];
				char * output
				ssize_t num_bytes_read;
				while ((num_bytes_read = read(pipe_fd[0], buf, BUFSIZ)) > 0) {
					// Output the received data
					write(output, buf, num_bytes_read);
				}
				close(pipe_fd[0]);
				/////////////////////////////////////////////////////

				timer.it_value.tv_sec = 0;
				setitimer(ITIMER_REAL, &timer, NULL);
			}
			*/

			// if the output of stderr contains the error keyword condition
			if (strstr(output, condition) != NULL) {
				return minimize_input(strcat(head, tail));
			}
		}

		for (int i = 0; i < input_length - sub_length - 1) {
			char * mid;
			strncpy(mid, test_input + i, i + sub_length);	// [i..i+s-1]

			char * output;
			// char * output = program_exec(mid);
			if (strcmp(output, condition) == 0) {
				return minimize_input(mid);
			}
		}

		// decrement substring by 1
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

	// flags of input options
	// 0 : false, 1 : true
	int iflag = 0;
	int mflag = 0;
	int oflag = 0;

	int opt;

	//pipe file descriptor, 0 for reading, 1 for writing
	int pipeFD[2]; 

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
	memcpy(exec_file, argv[7], 16);	// ./a.out : target program

	//create pipe
	if(pipe(pipeFD)!=0){
		perror("Pipe Error");
		exit(1);
	}

	/////////////////////////////////////////////////////
	// move fork(), execl(), and pipe to minimize_input()
	/////////////////////////////////////////////////////
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
	/////////////////////////////////////////////////////

    return 0;
}
