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

int main(int argc, char *argv[]) {
    int pid;
    int status;
    struct itimerval timer;

    // signal
    timer.it_value.tv_sec = EXEC_TIME;
	timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    setitimer(ITIMER_REAL, &timer, NULL);

    signal(SIGALRM, handle_signal);

    // options
    	// i : input
    char * test_input;
		// c : condition
	char * condition;
		// ./a.out : target program
	char * target_program = argv[1];

    pid = fork();
    if (pid < 0) {
        fprintf(stderr, "fork failed..\n");
        exit(1);
    } else if (pid == 0) {
        // child process executes binary file
        execl(target_program, target_program, NULL);
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
