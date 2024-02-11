#ifndef CIMIN_H
#define CIMIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mach/error.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>

#define EXEC_TIME 3

void end_program();
void write_file(char *output_filename);
void handle_signal(int sig);
char* run_target_program(char *crashing_input, char *exe_file, char **target_options);
char* minimize_crashing_input(char *input, char *condition, char *exe_file, char **target_options);
char* read_file_content(char *filepath);
void get_arguments(int argc, char *argv[], char **crashing_input_file, char **error_string, char **output_dir, char ***target_options);

#endif
