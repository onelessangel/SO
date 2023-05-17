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
	char *value = NULL;
	int err_code;

	/* Check if value word is not NULL (it exists) */
	if (cmd->next_part->next_part != NULL) {
		value = get_word(cmd->next_part->next_part);
	}

	// printf("var: %s\n", var);

	if (value == NULL) {
		// printf("valoarea e nula!!!\n", value);
		err_code = setenv(var, "", 1);
	} else {
		// printf("value: %s\n", value);
		err_code = setenv(var, value, 1);
	}

	free(value);

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

static void free_command_string(char ***argv, int argc)
{
	for (int i = 0; i < argc; i++) {
		free((*argv)[i]);
	}

	free(*argv);
	*argv = NULL;
}

static void redirect(int file_descriptor, const char *file_name, int flags, mode_t mode)
{
	int fd = open(file_name, flags, mode);
	DIE(fd < 0, "Error: failed open");

	int rc = dup2(fd, file_descriptor);
	DIE(rc < 0, "Error: failed dup2");

	rc = close(fd);
	DIE(rc < 0, "Error: failed close");
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
			int flags;
			
			if (s->out != NULL) {
				char *file_name = get_word(s->out);
				flags = O_WRONLY | O_CREAT;

				if (s->io_flags == IO_OUT_APPEND) {
					flags |= O_APPEND;
				} else {
					flags |= O_TRUNC;
				}

				int fd = open(file_name, flags, 0644);
				DIE(fd < 0, "Error: failed open");

				// int rc = dup2(fd, file_descriptor);
				// DIE(rc < 0, "Error: failed dup2");

				int rc = close(fd);
				DIE(rc < 0, "Error: failed close");

				// redirect(STDOUT_FILENO, s->out->string, flags, 0644);
			}

			if (s->err != NULL) {
				char *file_name = get_word(s->out);
				flags = O_WRONLY | O_CREAT;

				if (s->io_flags == IO_ERR_APPEND) {
					flags |= O_APPEND;
				} else {
					flags |= O_TRUNC;
				}

				int fd = open(file_name, flags, 0644);
				DIE(fd < 0, "Error: failed open");

				// int rc = dup2(fd, file_descriptor);
				// DIE(rc < 0, "Error: failed dup2");

				int rc = close(fd);
				DIE(rc < 0, "Error: failed close");

				// redirect(STDERR_FILENO, s->err->string, flags, 0644);
			}

			err_code = shell_cd(s->params);
			// printf("am intrat in comanda\n");
		}
		free_command_string(&argv, argc);
		return err_code;
	}

	/* TODO: If variable assignment, execute the assignment and return
	 * the exit status.
	 */
	// printf("sunt pe cazul asta\n");
	// printf("argc: %d\n", argc);
	// printf("argv[0]: %s\n", argv[0]);
	// printf("ELEMENT: %s\n", s->verb->next_part->string);
	if (argc == 1 && s->verb->next_part &&
		strcmp(s->verb->next_part->string, "=") == 0) {
		// printf("ENV VAR\n");

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
	int flags;

	int argc_child;
	char **argv_child;

	child_pid = fork();

	switch (child_pid)
	{
	case 0:
		/* Child process */
		
		argv_child = get_argv(s, &argc_child);

		if (s->in != NULL) {
			flags = O_RDONLY;
			redirect(STDIN_FILENO, s->in->string, flags, 0444);
		}

		if (s->out != NULL && s->err != NULL && strcmp(s->out->string, s->err->string) == 0) {
			char *file_name = get_word(s->out);

			int fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			DIE(fd < 0, "Error: failed open");

			int rc = dup2(fd, STDOUT_FILENO);
			DIE(rc < 0, "Error: failed dup2");

			rc = dup2(fd, STDERR_FILENO);
			DIE(rc < 0, "Error: failed dup2");

			rc = close(fd);
			DIE(rc < 0, "Error: failed close");

			free(file_name);
		} else {
			if (s->out != NULL) {
				char *file_name = get_word(s->out);
				flags = O_WRONLY | O_CREAT;

				if (s->io_flags == IO_OUT_APPEND) {
					flags |= O_APPEND;
				} else {
					flags |= O_TRUNC;
				}

				redirect(STDOUT_FILENO, file_name, flags, 0644);

				free(file_name);
			}

			if (s->err != NULL) {
				char *file_name = get_word(s->err);

				flags = O_WRONLY | O_CREAT;

				if (s->io_flags == IO_ERR_APPEND) {
					flags |= O_APPEND;
				} else {
					flags |= O_TRUNC;
				}

				redirect(STDERR_FILENO, file_name, flags, 0644);

				free(file_name);
			}
		}

		err_code = execvp(argv_child[0], argv_child);
		// DIE(1, "Error: failed execvp");

		if (err_code == -1) {
        	fprintf(stderr, "Execution failed for '%s'\n", argv_child[0]);
		}
		
		// !!!!!!!!!!!!!!!!!!!!!!
		exit(err_code);
		break;

	case -1:
		/* Error */
		// DIE(1, "Error; failed fork");
		err_code = 1;
		break;
	
	default:
		/* Parent process */
		wait_ret = waitpid(child_pid, &child_status, 0);
		DIE(wait_ret < 0, "Error: failed waitpid");
		// printf("io_flags: %d\n", s->io_flags);	

		// if (s->err != NULL) {
		// 	printf("err_file: %s\n", s->err->string);
		// }

		// if (s->out != NULL) {
		// 	printf("out_file: %s\n", s->out->string);
		// }

		// if (s->out != NULL && s->err != NULL && strcmp(s->out->string, s->err->string) == 0) {
		// 	printf("fac &>\n");
		// }
		// else if (s->out != NULL) {
		// 	printf("fac > sau >>\n");
		// } else if (s->err != NULL) {
		// 	printf("fac 2> sau 2>>\n");
		// }

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

		err_code = parse_command(c->cmd1, level + 1, c);
		err_code = parse_command(c->cmd2, level + 1, c);

		break;

	case OP_PARALLEL:
		/* TODO: Execute the commands simultaneously. */
		break;

	case OP_CONDITIONAL_NZERO:	// ||
		/* TODO: Execute the second command only if the first one
		 * returns non zero.
		 */

		err_code = parse_command(c->cmd1, level + 1, c);

		if (err_code != 0) {
			err_code = parse_command(c->cmd2, level + 1, c);
		}

		break;

	case OP_CONDITIONAL_ZERO:	// &&
		/* TODO: Execute the second command only if the first one
		 * returns zero.
		 */

		err_code = parse_command(c->cmd1, level + 1, c);

		if (err_code == 0) {
			err_code = parse_command(c->cmd2, level + 1, c);
		}
		
		break;

	case OP_PIPE:
		/* TODO: Redirect the output of the first command to the
		 * input of the second.
		 */
		break;

	default:
		return SHELL_EXIT;
	}

	return err_code; /* TODO: Replace with actual exit code of command. */
}
