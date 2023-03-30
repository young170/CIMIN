#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>

#define EXEC_TIME 3

void end_program();
void handle_signal(int sig);
char * minimize_input(char * input);
char * delta_debug(char * input);