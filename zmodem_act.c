/*
** zmodem_act.c for zssh
**
** Made by Matthieu Lucotte
** Login   <gounter@users.sourceforge.net>
**
** Started on  Thu Jun 29 19:11:29 2000 Matthieu Lucotte
** Last update Tue Sep 23 01:20:22 2003
*/

#include "zssh.h"


void zact_shell(char **av, int master)
{
	if (!sfork(&gl_child_rz)) {
		signal(SIGINT, SIG_DFL);
		signal(SIGWINCH, SIG_DFL);
		my_tcsetpgrp(0, getpgrp());
		signal(SIGTSTP, SIG_IGN);
		signal(SIGINT, SIG_DFL);
		execvp(av[0], av);
		perror(av[0]);
		exit(1);
	}
}

void zact_help(char **av, int master)
{
	printf("\n");
	printf("Builtins :\n");
	printf("  ?                           : print this message\n");
	printf("  cd                          : change directory\n");
	printf("  disconnect                  : disconnect and exit\n");
	printf("  escape [^X]                 : change escape key to ^X \n");
	printf("                                without argument, print current escape key \n");
	printf("  exit                        : exit file transfer mode\n");
	printf("  help                        : print this message\n");
	printf("  hook prg                    : hook program 'prg' to the pty instead of sz or rz\n");
	printf("  quit                        : same as exit\n");
	printf("  repeat <cmd>                : repeats cmd forever (^C to interrupt)\n");
	printf("  rz                          : receive files\n");
	printf("  suspend                     : suspend zssh\n");
	printf("  sz <file> ...               : send files\n");
	printf("  version                     : print version information\n");
	printf("  <program_name> <params> ... : exec the prog\n");
	printf("\n");
	printf("Usage :\n");
	printf("  Download : run sz on the remote shell before switching to transfer mode\n");
	printf("             then type rz\n");
	printf("  Upload   : switch to transfer mode and type sz <files>\n");
	printf("             rz will be automatically run on the remote side\n");
	printf("\n");
	printf("Tips:\n");
	printf("  - If file transfer never completes, use the -e option of sz/rz\n");
	printf("  - Transfers can be interrupted with ^C\n");
	printf("  - If you get stuck in rz/sz (for example you've just ran rz, but you\n");
	printf("    then decided not to transmit anything), hit a dozen ^X to stop it\n");
	printf("  - Use sz -y <files> to overwrite remote files\n\n");
	printf("This shell supports line edition, history, completion, wildcards and escapes\n");
}

void zact_version(char **av, int master)
{
	version(0);
}


void zact_cd(char **av, int master)
{
	char *str;

	if (!(str = av[1]))
		str = getenv("HOME");
	if (chdir(str) < 0)
		perror(str);
}

void zact_suspend(char **av, int master)
{
	kill(getpid(), SIGTSTP);
}

void zact_disconnect(char **av, int master)
{
	done(0);
}


void zact_repeat(char **av, int master)
{
	gl_repeat = 1;
}

/* create the hook process and map its std{in,out} to master */
void zact_hook_sub(char **av, int master)
{
	tcsetattr(gl_slave, TCSAFLUSH, &gl_rtt);
	if (gl_copty) {
		tcsetattr(gl_hook_slave, TCSAFLUSH, &gl_rtt);
	}

	if (!sfork(&gl_child_rz)) {
		signal(SIGINT, SIG_DFL);
		signal(SIGWINCH, SIG_DFL);
		close(gl_slave);
		if (gl_copty) {
			close(gl_master);
			close(gl_hook_master);
			dup2(gl_hook_slave, 0);
			dup2(gl_hook_slave, 1);
			close(gl_hook_slave);
		} else {
			dup2(master, 0);
			dup2(master, 1);
			close(master);
		}

		execvp(av[0], av);
		error("error: execvp %s\n", av[0]);
		exit(1);
	}
#ifdef DEBUG
	printf("launching %s (pid=%i) ...\n", av[0], gl_child_rz);
#endif

	if (gl_copty) {
		if (!sfork(&gl_child_read)) {
			copy_rz_stream(gl_hook_master, master);
			exit(0);
		}
		copy_rz_stream(master, gl_hook_master);
	}
}

void zact_hook(char **av, int master)
{
	zact_hook_sub(av + 1, master);
}

void zact_escape(char **av, int master)
{
	if (!av[1])
		printf("Current escape key: ^%c (%s)\n", gl_escape, escape_help());
	else
		set_escape(av[1]);
}

void zact_exit(char **av, int master)
{
	write(master, "\n", 1);
}
