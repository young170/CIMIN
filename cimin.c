#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>

#define EXEC_TIME 3

void handle_signal(int sig) {
	if (sig == SIGALRM) {
		printf("3 sec..\n");
		exit(1);
	}
}

int main(int argc, char *argv[]) {
    int pid;
    int status;
    struct itimerval timer;

    timer.it_value.tv_sec = EXEC_TIME;
	timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    setitimer(ITIMER_REAL, &timer, NULL);

    signal(SIGALRM, handle_signal);

    pid = fork();
    if (pid < 0) {
        fprintf(stderr, "fork failed..\n");
        exit(1);
    } else if (pid == 0) {
        execl(argv[1], argv[1], NULL);					// child process executes binary file
        fprintf(stderr, "child process failed..\n");
        exit(1);
    } else {
        waitpid(pid, &status, 0);
        timer.it_value.tv_sec = 0;
        setitimer(ITIMER_REAL, &timer, NULL);
    }

    return 0;
}
