// SPDX-License-Identifier: BSD-3-Clause

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <unistd.h>

#include <string.h>

#include "cmd.h"
#include "utils.h"

#define READ		0
#define WRITE		1
#define PATH_LEN	40
#define CMD_LEN		20

/**
 * Internal change-directory command.
 */
static bool shell_cd(word_t *dir)
{
	/* Execute cd. */
	DIE(dir == NULL, "Error: incorrect directory");

	char *target = get_word(dir);

	printf("director: %s\n", target);

	int err_code = chdir(target);

	free(target);

	if (err_code == -1) {
		fprintf(stderr, "no such file or directory\n");
		return 1;
	}

	return 0;
}

static int shell_set_env_var(word_t *cmd)
{
	DIE(cmd == NULL, "Error: incorrect environment variable command");

	const char *var = cmd->string;
	const char *value = NULL;
	int err_code;

	/* Check if value word is not NULL (it exists) */
	if (cmd->next_part->next_part != NULL) {
		value = cmd->next_part->next_part->string;
	}

	// printf("var: %s\n", var);

	if (value == NULL) {
		// printf("valoarea e nula!!!\n", value);
		err_code = setenv(var, "", 1);
	} else {
		// printf("value: %s\n", value);
		err_code = setenv(var, value, 1);
	}

	// printf("value: %s\n", getenv(var));

	if (err_code == -1) {
		return 1;
	}

	return 0;
}

/**
 * Internal exit/quit command.
 */
static int shell_exit(void)
{
	/* Execute exit/quit. */

	// char path[PATH_LEN] = "/usr/bin/";
	// char command[CMD_LEN] = "quit";
    // char *args[2] = { "quit", NULL };

	// strcat(path, command);

	// printf("%s \n", path);

	// execv(path, args);

	return SHELL_EXIT;
}

static bool shell_pwd()
{

	return true;
}

static void free_command_string(char ***argv, int argc)
{
	for (int i = 0; i < argc; i++) {
		free((*argv)[i]);
	}

	free(*argv);
	*argv = NULL;
}

/**
 * Parse a simple command (internal, environment variable assignment,
 * external command).
 */
static int parse_simple(simple_command_t *s, int level, command_t *father)
{
	/* Sanity checks. */
	DIE(s == NULL, "Error: simple command is NULL");
	DIE(level < 0, "Error: incorrect level value");

	int err_code = 0;

	/* If builtin command, execute the command. */
	int argc;
	char **argv = get_argv(s, &argc);

	if (strcmp(argv[0], "exit") == 0 || strcmp(argv[0], "quit") == 0) {
		err_code = shell_exit();
		free_command_string(&argv, argc);
		return err_code;
	}
	
	if (strcmp(argv[0], "cd") == 0) {
		// printf("dir: %s\n", s->params->string);
		printf("argc: %d\n", argc);
		if (argc == 2) {
			err_code = shell_cd(s->params);
			// printf("am intrat in comanda\n");
		}
		free_command_string(&argv, argc);
		return err_code;
	}

	// printf("command: %s\n", argv[0]);

	// printf("command size: %d\n", argc);

	/* TODO: If variable assignment, execute the assignment and return
	 * the exit status.
	 */
	// printf("sunt pe cazul asta\n");
	// printf("argc: %d\n", argc);
	// printf("argv[0]: %s\n", argv[0]);
	// printf("ELEMENT: %s\n", s->verb->next_part->string);
	if (argc == 1 && s->verb->next_part &&
		strcmp(s->verb->next_part->string, "=") == 0) {
		printf("ENV VAR\n");

		err_code = shell_set_env_var(s->verb);
		free_command_string(&argv, argc);
		return err_code;
	}

	/* TODO: If external command:
	 *   1. Fork new process
	 *     2c. Perform redirections in child
	 *     3c. Load executable in child
	 *   2. Wait for child
	 *   3. Return exit status
	 */

	pid_t child_pid;
	pid_t wait_ret;
	int child_status;

	child_pid = fork();

	switch (child_pid)
	{
	case 0:
		/* Child process */
		// stdout file
		err_code = execvp(argv[0], argv);
		// DIE(1, "Error: failed execvp");
		break;

	case -1:
		/* Error */
		// DIE(1, "Error; failed fork");
		err_code = 1;
		break;
	
	default:
		/* Parent process */
		wait_ret = waitpid(child_pid, &child_status, 0);
		// DIE(wait_ret < 0, "Error: failed waitpid");

		if (WIFEXITED(child_status)) {
			err_code = WEXITSTATUS(child_status);
		}
		break;
	}

	free_command_string(&argv, argc);

	return err_code; /* TODO: Replace with actual exit status. */
}

/**
 * Process two commands in parallel, by creating two children.
 */
static bool run_in_parallel(command_t *cmd1, command_t *cmd2, int level,
		command_t *father)
{
	/* TODO: Execute cmd1 and cmd2 simultaneously. */

	return true; /* TODO: Replace with actual exit status. */
}

/**
 * Run commands by creating an anonymous pipe (cmd1 | cmd2).
 */
static bool run_on_pipe(command_t *cmd1, command_t *cmd2, int level,
		command_t *father)
{
	/* TODO: Redirect the output of cmd1 to the input of cmd2. */

	return true; /* TODO: Replace with actual exit status. */
}

/**
 * Parse and execute a command.
 */
int parse_command(command_t *c, int level, command_t *father)
{
	/* sanity checks */
	DIE(c == NULL, "Error: command is NULL");
	DIE(level < 0, "Error: incorrect level value");

	int err_code = 0;

	if (c->op == OP_NONE) {
		/* Execute a simple command. */
		err_code = parse_simple(c->scmd, level, father);

		// printf("sunt aici\n");

		// exit(err_code);

		return(err_code) ; /* TODO: Replace with actual exit code of command. */
	}

	switch (c->op) {
	case OP_SEQUENTIAL:
		/* TODO: Execute the commands one after the other. */
		break;

	case OP_PARALLEL:
		/* TODO: Execute the commands simultaneously. */
		break;

	case OP_CONDITIONAL_NZERO:
		/* TODO: Execute the second command only if the first one
		 * returns non zero.
		 */
		break;

	case OP_CONDITIONAL_ZERO:
		/* TODO: Execute the second command only if the first one
		 * returns zero.
		 */
		break;

	case OP_PIPE:
		/* TODO: Redirect the output of the first command to the
		 * input of the second.
		 */
		break;

	default:
		return SHELL_EXIT;
	}

	return 0; /* TODO: Replace with actual exit code of command. */
}
