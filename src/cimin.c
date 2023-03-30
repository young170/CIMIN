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


char * minimize_input(char * input, char * substring) {
	int length = strlen(input);
}

char * delta_debug(char * input, char * condition) {
	char * reduced_input = new char[strlen(input)];
	int length = strlen(reduced_input) - 1;

	while (length > 0) {

	}
}

int main(int argc, char *argv[]) {
    int pid;
    int status;
	int opt;
    struct itimerval timer;

    // signal
    timer.it_value.tv_sec = EXEC_TIME;
	timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    setitimer(ITIMER_REAL, &timer, NULL);

    signal(SIGALRM, handle_signal);

	// input
	char crashFile[16];
	char errorString[16];
	char reducedFile[16];
	char execFile[16];

	if (argc != 8){
			fprintf(stderr, "too many arguments..\n");
			exit(1);
	}

	while ((opt = getopt(argc,argv,"i:m:o:")) != -1) {
		switch (opt) {
		case 'i':
			memcpy(crashFile, optarg, 16);
			break;
		case 'm':
			memcpy(errorString, optarg, 16);
			break;
		case 'o' :
			memcpy(reducedFile, optarg, 16);
			break;
		case '?' :
			fprintf(stderr, "option doesn't exist..\n");
			exit(1);
		}
	}
	memcpy(execFile, argv[7], 16);

    pid = fork();
    if (pid < 0) {
        fprintf(stderr, "fork failed..\n");
        exit(1);
    } else if (pid == 0) {
        // child process executes binary file
        execl(execFile, execFile, NULL);
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
