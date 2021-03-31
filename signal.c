/*
** signal.c for zssh
**
** Made by Matthieu Lucotte
** Login   <gounter@users.sourceforge.net>
**
** Started on  Thu Jun 29 19:10:55 2000 Matthieu Lucotte
** Last update Thu Oct 11 20:34:51 2001 Matthieu Lucotte
*/

#include "zssh.h"

/*
 * Note: all signal handlers should make sure they
 *	 reset themselves by calling signal() before
 *	 they exit.
 */

#ifdef DEBUG
void print_process_status(int pid, int s)
{
	if (WIFEXITED(s))
		printf("process %i: exit value: %i\n", pid, WEXITSTATUS(s));
	if (WIFSIGNALED(s))
		printf("process %i: received signal %i\n", pid, WTERMSIG(s));
	if (WIFSTOPPED(s))
		printf("process %i: stopped by signal %i\n", pid, WSTOPSIG(s));
}

#endif


void sigchld_handler(int sig)
{
	int old_errno;
	int pid;
	int s;
	int die;
	sigset_t sigchld_mask;

	old_errno = errno;
	signal(SIGCHLD, sigchld_handler);
	die = 0;

	/* unblock SIGCHLD, avoid missing new SIGCHLDs on return */
	sigemptyset(&sigchld_mask);
	sigaddset(&sigchld_mask, SIGCHLD);
	sigprocmask(SIG_UNBLOCK, &sigchld_mask, NULL);

	while (1) {
		errno = EINTR;
		pid = 0;
		while (pid <= 0 && errno == EINTR) {
			errno = 0;
			pid = waitpid(WAIT_ANY, &s, WNOHANG);
		}
		if (pid <= 0) {
			errno = old_errno;
			if (die)
				done(0);
			return;
		}
#ifdef DEBUG
		print_process_status(pid, s);
#endif
		if (pid == gl_child_shell)
			die = 1;
		if (pid == gl_child_rz)
			gl_child_rz = 0;
	}
}

void sigint_handler(int sig)
{
	signal(SIGINT, sigint_handler);
	gl_repeat = 0;
	if (gl_child_rz) {
		kill(gl_child_rz, SIGTERM);
		gl_interrupt = 1;
	}
}

void sigwinch_handler(int sig)
{
	signal(SIGWINCH, sigwinch_handler);
	ioctl(0, TIOCGWINSZ, &gl_win);
	ioctl(gl_slave, TIOCSWINSZ, &gl_win);
	kill(gl_child_shell, SIGWINCH);
}
