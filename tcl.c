
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <stdbool.h>

	#include "tcl.h"

//---------------------------------------------------

const char const tcl_help_txt[] = {
	"A lightweight TCL interpreter for embedded systems\r\n"
	"Usage: tcl [-id] [-c \"command\"] [source] \r\n"
	"\r\n"
	"    -i           runs the interreter in interactive mode\r\n"
	"    -c           interpret and execute a single line of code\r\n"
	"    -d           Enables verbose debugging information\r\n"
};

extern struct tcl_cmd_builtin builtin_commands[];
bool interactive = false, doExit = false, debug = false;
int docmd = NULL;

//----------------------Main-------------------------

int main(int argc, char *argv[]){
	FILE *file = NULL;

	/* Default to starting in interactive mode if no args are passed */
	if (argc <= 1) interactive = true;
	else {
		for (int i = 1; i < argc; i++){
			if (argv[i][0] == '-'){
				switch (argv[i][1]){
					case 'h':
						puts(tcl_help_txt);
						return 0;
					default:
					case 'i':
						interactive = true;
						break;
					case 'c':
						if (i < argc - 1) {
							docmd = i;
							i++;
						} else {
							printf("[?] No command specified\n");
							return;
						}
						break;
					case 'd':
						debug = true;
						break;
				}
			} else {
				file = fopen(argv[i], "rb");
				if (file == NULL) {
					printf("[?] File could not be found\n");
					return -1;
				}
				fseek(file, 0, SEEK_SET);
			}
		}
	}

	struct tcl tcl;
	tcl_init(&tcl);

	if (interactive) {
		printf("TCL Command Shell v0.8\n");
		printf("NotArtyom 6/10/19\n");
	} else if (docmd != NULL) {
		if (tcl_eval(&tcl, argv[docmd+1], strlen(argv[docmd+1])+1) != FERROR) {
    		if (*(tcl.result) != '\0') printf("> %s\n", tcl_string(tcl.result));
		} else {
			printf("[?] Syntax\n");
		}
		tcl_destroy(&tcl);
		return;
	}

	int buffer_length = 1024;
	char *buff = malloc(buffer_length);
	int i = 0;
	while (!doExit) {
		int input;
		if (interactive) input = fgetc(stdin);
		else input = fgetc(file);

		if (i > buffer_length - 1) buff = realloc(buff, buffer_length += 1024);
		if (input == 0 || input == EOF) break;
		buff[i++] = input;

		tcl_each(buff, i, 1) {
			if (prse.token == TCL_ERROR && (prse.dest - buff) != i) {
				memset(buff, 0, buffer_length);
				i = 0;
				break;
			} else if (prse.token == TCL_CMD && *(prse.src) != '\0') {
				int r = tcl_eval(&tcl, buff, strlen(buff));
				if (r == FERROR) {
					printf("[?] Syntax\n");
				} else if (interactive && !doExit && *(tcl.result) != '\0') {
					printf("> %.*s\n", tcl_length(tcl.result), tcl_string(tcl.result));
				}
				memset(buff, 0, buffer_length);
		        i = 0;
		        break;
			}
		}
	}
	free(buff);
	if (file != NULL) fclose(file);
	return 0;
}

//---------------------------------------------------

void tcl_init(struct tcl *tcl) {
	tcl->env = tcl_env_alloc(NULL);
    tcl->result = tcl_alloc("", 0);
    tcl->cmds = NULL;

	/* Walk throuth the builtin command array and install each one */
	for (int i = 0; i < get_builtin_count(); i++)
		tcl_register(tcl, builtin_commands[i].name, builtin_commands[i].fn, builtin_commands[i].arity, NULL);

		char *math[] = {"+", "-", "*", "/", ">", ">=", "<", "<=", "==", "!="};
		for (unsigned int i = 0; i < (sizeof(math) / sizeof(math[0])); i++)
			tcl_register(tcl, math[i], tcl_cmd_math, 3, NULL);
}

void tcl_destroy(struct tcl *tcl) {
	while (tcl->env) tcl->env = tcl_env_free(tcl->env);
	while (tcl->cmds) {
		struct tcl_cmd *cmd = tcl->cmds;
		tcl->cmds = tcl->cmds->next;
		free(cmd->name);
		free(cmd->arg);
		free(cmd);
	}
	free(tcl->result);
}
