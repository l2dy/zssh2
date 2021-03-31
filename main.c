/*
** main.c for zssh
**
** Made by Matthieu Lucotte
** Login   <gounter@users.sourceforge.net>
**
** Started on  Thu Jun 29 19:10:13 2000 Matthieu Lucotte
** Last update Mon Sep  1 23:18:58 2003
*/

#include "zssh.h"


int main(int argc, char *argv[])
{
	init(&argc, &argv);
	printf("Press ^%c (%s) to enter file transfer mode, then ? for help\n",
	       gl_escape, escape_help());
	fflush(stdout);

	if (!sfork(&gl_child_output))
		dooutput();
	if (!sfork(&gl_child_shell))
		doshell(argc, argv, gl_shav); /* std{in,out} mapped to gl_slave in initslave() */

	doinput();
	return 0;
}

/* new one: test escape sequence
 * NB: customizing Ctrl-Something escape sequences is easy ...
 *                 Alt-Something is not.
 */
int escape_input(ssize_t *cc, unsigned char *ibuf)
{
	if (*cc == 1 && ibuf[0] == gl_escape - '@') /* escape key code */
		return 1;
	return 0;
}

void read_input(ssize_t *cc, unsigned char *ibuf)
{
	*cc = read(0, ibuf, BUFSIZ);
#ifdef DEBUG_CRAZY
	{
		int i;

		for (i = 0; i < *cc; i++)
			printf("%02i ", ibuf[i]);
		printf("\n");
	}
#endif
}

/* switch to local shell mode
 */
void rz_mode(void)
{
	char **av;
	int ac;
	int i, k;
	char *line;

	gl_local_shell_mode = 1;
	kill(gl_child_output, SIGSTOP); /* suspend output */
	printf("\r");
	printf("\n");
	tcgetattr(gl_slave, &gl_tt2);   /* save slave tty state */
	/* TCSAFLUSH causes problems on some systems */
	tcsetattr(0, TCSANOW, &gl_tt);  /* was in raw mode */
	i = 0;
	while (i < 100) {               /* action codes >= 100 exit */
		line = zprompt();
		if (zparse(&line, &av, &ac) < 0)
			continue;
		i = zrun(av);
		free(line);
		for (k = 0; k < ac; k++)
			if (av[k])
				free(av[k]);
		free(av);
	}
	tcsetattr(0, TCSANOW, &gl_rtt); /* restore raw term */
	kill(gl_child_output, SIGCONT); /* resume output */
	gl_local_shell_mode = 0;
}


void fail(void)
{
	done(1);
}

/* should be called only by main process */
void done(int ret)
{
	if (getpid() != gl_main_pid)
		error("done() should only be called by main process", "");
	if (gl_child_rz)
		kill(gl_child_rz, SIGTERM);
	if (gl_child_shell)
		kill(gl_child_shell, SIGTERM);
	if (gl_child_output) {
		usleep(500);
		kill(gl_child_output, SIGTERM);
		tcsetattr(0, TCSAFLUSH, &gl_tt);
	}
	exit(ret);
}


