#ifndef CIMIN_H
#define CIMIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <error.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>

#define EXEC_TIME 3

#ifdef DEBUG
    #define DPRINT(func) func;
#else
    #define DPRINT(func) ;
#endif

void end_program();
void handle_signal(int sig);
char * program_exec(char ** input, char * exec_file, char ** target_options);
char ** minimize_input(char ** input, char * condition, char * exec_file, char ** target_options);
char ** delta_debug(char ** input, char * condition, char * exec_file, char ** target_options);
char ** file_data(char * filepath);

#endif