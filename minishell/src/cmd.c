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
#define AVOID_DUP	0
#define MAKE_DUP	1

/**
 * Internal change-directory command.
 */
static bool shell_cd(word_t *dir)
{
	/* Execute cd. */
	DIE(dir == NULL, "Error: incorrect directory");

	char *target = get_word(dir);

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
	char *value = get_word(cmd->next_part->next_part);
	int err_code;

	if (value == NULL)
		err_code = setenv(var, "", 1);
	else
		err_code = setenv(var, value, 1);

	free(value);

	if (err_code == -1)
		return 1;

	return 0;
}

/**
 * Internal exit/quit command.
 */
static int shell_exit(void)
{
	/* Execute exit/quit. */
	return SHELL_EXIT;
}

static void free_command_string(char ***argv, int argc)
{
	for (int i = 0; i < argc; i++)
		free((*argv)[i]);

	free(*argv);
	*argv = NULL;
}

static void redirect(int file_descriptor, const char *file_name, int flags, mode_t mode, int dup_usage)
{
	int rc;
	int fd;

	fd = open(file_name, flags, mode);
	DIE(fd < 0, "Error: failed open");

	if (dup_usage == MAKE_DUP) {
		rc = dup2(fd, file_descriptor);
		DIE(rc < 0, "Error: failed dup2");
	}

	rc = close(fd);
	DIE(rc < 0, "Error: failed close");
}

static void redirect_stdin(simple_command_t *s, int dup_usage)
{
	char *file_name = get_word(s->in);
	int flags = O_RDONLY;

	redirect(STDIN_FILENO, file_name, flags, 0444, dup_usage);

	free(file_name);
}

static void redirect_stdout(simple_command_t *s, int dup_usage)
{
	char *file_name = get_word(s->out);
	int flags = O_WRONLY | O_CREAT;

	if (s->io_flags == IO_OUT_APPEND)
		flags |= O_APPEND;
	else
		flags |= O_TRUNC;

	redirect(STDOUT_FILENO, file_name, flags, 0644, dup_usage);

	free(file_name);
}

static void redirect_stderr(simple_command_t *s, int dup_usage)
{
	char *file_name = get_word(s->err);
	int flags = flags = O_WRONLY | O_CREAT;

	if (s->io_flags == IO_ERR_APPEND)
		flags |= O_APPEND;
	else
		flags |= O_TRUNC;

	redirect(STDERR_FILENO, file_name, flags, 0644, dup_usage);

	free(file_name);
}

static void redirect_stdout_and_stderr(simple_command_t *s)
{
	char *file_name = get_word(s->out);
	int flags = O_WRONLY | O_CREAT | O_TRUNC;
	int fd, rc;

	fd = open(file_name, flags, 0644);
	DIE(fd < 0, "Error: failed open");

	rc = dup2(fd, STDOUT_FILENO);
	DIE(rc < 0, "Error: failed dup2");

	rc = dup2(fd, STDERR_FILENO);
	DIE(rc < 0, "Error: failed dup2");

	rc = close(fd);
	DIE(rc < 0, "Error: failed close");

	free(file_name);
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

	int argc;
	char **argv = get_argv(s, &argc);
	int err_code = 0;

	/* If builtin command, execute the command. */

	if (strcmp(argv[0], "exit") == 0 || strcmp(argv[0], "quit") == 0) {
		err_code = shell_exit();
		free_command_string(&argv, argc);
		return err_code;
	}

	if (strcmp(argv[0], "cd") == 0) {
		if (argc == 2) {
			if (s->out != NULL)
				redirect_stdout(s, AVOID_DUP);

			if (s->err != NULL)
				redirect_stderr(s, AVOID_DUP);

			err_code = shell_cd(s->params);
		}
		free_command_string(&argv, argc);
		return err_code;
	}

	/* If variable assignment, execute the assignment and return
	 * the exit status.
	 */
	if (argc == 1 && s->verb->next_part &&
		strcmp(s->verb->next_part->string, "=") == 0) {
		err_code = shell_set_env_var(s->verb);
		free_command_string(&argv, argc);
		return err_code;
	}

	/* If external command:
	 *   1. Fork new process
	 *     2c. Perform redirections in child
	 *     3c. Load executable in child
	 *   2. Wait for child
	 *   3. Return exit status
	 */

	pid_t child_pid;
	pid_t wait_ret;
	int child_status;

	int argc_child;
	char **argv_child;

	child_pid = fork();

	switch (child_pid) {
	case 0:
		/* Child process */
		argv_child = get_argv(s, &argc_child);

		if (s->in != NULL)
			redirect_stdin(s, MAKE_DUP);

		if (s->out != NULL && s->err != NULL && strcmp(s->out->string, s->err->string) == 0) {
			redirect_stdout_and_stderr(s);
		} else {
			if (s->out != NULL)
				redirect_stdout(s, MAKE_DUP);

			if (s->err != NULL)
				redirect_stderr(s, MAKE_DUP);
		}

		err_code = execvp(argv_child[0], argv_child);

		if (err_code == -1)
			fprintf(stderr, "Execution failed for '%s'\n", argv_child[0]);

		exit(err_code);
		break;

	case -1:
		/* Error */
		err_code = 1;
		break;

	default:
		/* Parent process */
		wait_ret = waitpid(child_pid, &child_status, 0);
		DIE(wait_ret < 0, "Error: failed waitpid");

		if (WIFEXITED(child_status))
			err_code = WEXITSTATUS(child_status);

		break;
	}

	free_command_string(&argv, argc);

	return err_code;
}

/**
 * Process two commands in parallel, by creating two children.
 */
static bool run_in_parallel(command_t *cmd1, command_t *cmd2, int level,
		command_t *father)
{
	/* Execute cmd1 and cmd2 simultaneously. */
	int err_code = 0;
	pid_t child_pid;
	pid_t wait_ret;
	int child_status;

	child_pid = fork();

	switch (child_pid) {
	case 0:
		/* Child process */
		err_code = parse_command(cmd1, level, father);

		exit(err_code);
		break;

	case -1:
		/* Error */
		err_code = 1;
		break;

	default:
		/* Parent process */
		err_code = parse_command(cmd2, level, father);

		wait_ret = waitpid(child_pid, &child_status, 0);
		DIE(wait_ret < 0, "Error: failed waitpid");

		if (WIFEXITED(child_status))
			err_code = WEXITSTATUS(child_status);

		break;
	}

	return err_code;
}

/**
 * Run commands by creating an anonymous pipe (cmd1 | cmd2).
 */
static bool run_on_pipe(command_t *cmd1, command_t *cmd2, int level,
		command_t *father)
{
	/* Redirect the output of cmd1 to the input of cmd2. */
	int pipefd[2];
	pid_t child_pid;
	pid_t second_child_pid;
	int pipe_ret;
	int err_code = 0;
	int second_child_status;
	int wait_ret;
	int rc;

	pipe_ret = pipe(pipefd);
	DIE(pipe_ret == -1, "Error: incorrect pipe");

	child_pid = fork();

	switch (child_pid) {
	case 0:
		/* Child process */

		/* Use first child process for executing first command */
		rc = close(pipefd[READ]);
		DIE(rc < 0, "Error: failed close");

		/* Only needs the WRITE end of the pipe */
		rc = dup2(pipefd[WRITE], STDOUT_FILENO);
		DIE(rc < 0, "Error: failed dup2");

		rc = close(pipefd[WRITE]);
		DIE(rc < 0, "Error: failed close");

		err_code = parse_command(cmd1, level, father);

		exit(err_code);
		break;

	case -1:
		/* Error */
		rc = close(pipefd[READ]);
		DIE(rc < 0, "Error: failed close");

		rc = close(pipefd[WRITE]);
		DIE(rc < 0, "Error: failed close");

		err_code = 1;
		break;

	default:
		/* Parent process */
		second_child_pid = fork();

		switch (second_child_pid) {
		case 0:
			/* Child process */

			/* Use second child process for executing second command */
			rc = close(pipefd[WRITE]);
			DIE(rc < 0, "Error: failed close");

			/* Only needs the READ end of the pipe */
			rc = dup2(pipefd[READ], STDIN_FILENO);
			DIE(rc < 0, "Error: failed dup2");

			rc = close(pipefd[READ]);
			DIE(rc < 0, "Error: failed close");

			err_code = parse_command(cmd2, level, father);

			exit(err_code);
			break;

		case -1:
			/* Error */
			rc = close(pipefd[READ]);
			DIE(rc < 0, "Error: failed close");

			rc = close(pipefd[WRITE]);
			DIE(rc < 0, "Error: failed close");

			err_code = 1;
			break;

		default:
			/* Parent process */

			break;
		}

		rc = close(pipefd[READ]);
		DIE(rc < 0, "Error: failed close");

		rc = close(pipefd[WRITE]);
		DIE(rc < 0, "Error: failed close");

		/* Wait for second child process to finish */
		wait_ret = waitpid(second_child_pid, &second_child_status, 0);
		DIE(wait_ret < 0, "Error: failed waitpid");

		if (WIFEXITED(second_child_status))
			err_code = WEXITSTATUS(second_child_status);

		break;
	}

	return err_code;
}

/**
 * Parse and execute a command.
 */
int parse_command(command_t *c, int level, command_t *father)
{
	/* Sanity checks */
	DIE(c == NULL, "Error: command is NULL");
	DIE(level < 0, "Error: incorrect level value");

	int err_code = 0;

	if (c->op == OP_NONE) {
		/* Execute a simple command. */
		err_code = parse_simple(c->scmd, level, father);

		return err_code;
	}

	switch (c->op) {
	case OP_SEQUENTIAL:
		/* Execute the commands one after the other. */
		err_code = parse_command(c->cmd1, level + 1, c);
		err_code = parse_command(c->cmd2, level + 1, c);

		break;

	case OP_PARALLEL:
		/* Execute the commands simultaneously. */
		err_code = run_in_parallel(c->cmd1, c->cmd2, level + 1, c);
		break;

	case OP_CONDITIONAL_NZERO:	// ||
		/* Execute the second command only if the first one
		 * returns non zero.
		 */
		err_code = parse_command(c->cmd1, level + 1, c);

		if (err_code != 0)
			err_code = parse_command(c->cmd2, level + 1, c);

		break;

	case OP_CONDITIONAL_ZERO:	// &&
		/* Execute the second command only if the first one
		 * returns zero.
		 */
		err_code = parse_command(c->cmd1, level + 1, c);

		if (err_code == 0)
			err_code = parse_command(c->cmd2, level + 1, c);

		break;

	case OP_PIPE:
		/* Redirect the output of the first command to the
		 * input of the second.
		 */
		err_code = run_on_pipe(c->cmd1, c->cmd2, level + 1, c);

		break;

	default:
		return SHELL_EXIT;
	}

	return err_code;
}
