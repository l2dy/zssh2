/*
** openpty.c for zssh
**
** Made by Matthieu Lucotte
** Login   <gounter@users.sourceforge.net>
**
** Started on  Thu Jun 29 19:10:25 2000 Matthieu Lucotte
** Last update Wed Sep 24 00:05:06 2003
*/

#include "zssh.h"

#define GL_SLAVENAMELEN 50
static char gl_slavename[GL_SLAVENAMELEN + 1] = { 0 };

#ifdef HAVE_UTIL_H
#include <util.h>
#endif

#ifdef HAVE_LIBUTIL_H
#include <libutil.h>
#endif

#ifdef HAVE_PTY_H
#include <pty.h>
#endif

void getmaster(void)
{
#ifdef DEBUG
	printf("Using openpty() for tty allocation\n");
#endif
	if (openpty(&gl_master, &gl_slave, gl_slavename, &gl_tt, &gl_win) < 0)
		error(0, "openpty");
}

void getslave(void)
{
	testslave(gl_slavename);
}

void my_tcsetpgrp(int fd, int pgrpid)
{
	int ret;

#ifdef HAVE_TCSETPGRP
	ret = tcsetpgrp(fd, pgrpid);
#else
	ret = ioctl(fd, TIOCSPGRP, &pgrpid);
#endif /* HAVE_TCSETPGRP */

	if (ret < 0)
		error(0, "my_tcsetpgrp");
}

/* set raw mode */
void my_cfmakeraw(struct termios *pt)
{
	/* beginning of 'official' cfmakeraw function */
	pt->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
	                 | INLCR | IGNCR | ICRNL | IXON);
	pt->c_oflag &= ~OPOST;
	pt->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	pt->c_cflag &= ~(CSIZE | PARENB);
	pt->c_cflag |= CS8;
	/* end of 'official' cfmakeraw function */

	pt->c_cc[VMIN] = 1;
	pt->c_cc[VTIME] = 0;
	/*   pt->c_oflag |= OPOST; */
	/*   pt->c_lflag &= ~ECHO; */
}


/* called by getslave()
 * test tty permissions and warn user if insecure
 */
void testslave(char *ttyname)
{
	struct stat st;
	struct passwd        *pwd;
	int ask = 0;

	if (fstat(gl_slave, &st) < 0)
		error(0, "fstat tty");
	if (st.st_uid != getuid()) { /* tty is not owned by the user, this can be a security issue so prompt the user */
		if ( (pwd = getpwuid(st.st_uid)) )
			printf("*** %s: This tty is owned by someone else (%s) !\n", ttyname, pwd->pw_name);
		else
			printf("*** %s: This tty is owned by someone else (uid %lu) !\n", ttyname, (long)st.st_uid);
		ask = 1;
	}
	if (st.st_mode & S_IWOTH)
		/* tty is world writeable: this can be abused but there is no serious security issue here
		 * so just print a warning.   */
		printf("*** %s: this tty is world writeable !\n", ttyname);
	if (st.st_mode & S_IROTH) { /* tty is world readable: this is very insecure so prompt the user */
		printf("*** %s: this tty is world readable !\n", ttyname);
		ask = 1;
	}
	if (ask) {
		printf("*** This is a security issue\n");
		if (!ask_user("Do you want to continue anyway ?", 0, 1))
			error("aborting\n", "");
	}
}


/* init slave after call to getslave
 * make slave the controlling tty for current process
 */
void initslave(void)
{
	close(gl_master);
	setsid();

	/* by now we should have dropped the controlling tty
	 * make sure it is indeed the case
	 */
	if (open("/dev/tty", O_RDWR) >= 0)
		error("Couldn't drop controlling tty\n", "");

#ifdef TIOCSCTTY
	if (ioctl(gl_slave, TIOCSCTTY, 0) < 0)
		perror("ioctl(slave, TIOCSCTTY, 0)");
#else    /* re-open the tty so that it becomes the controlling tty */
	close(gl_slave);
	if ( (gl_slave = open(gl_slavename, O_RDWR)) < 0 )
		error(0, gl_slavename);
#endif /* TIOCSCTTY */

	if (dup2(gl_slave, 0) < 0)
		error(0, "dup2(slave, 0)");
	dup2(gl_slave, 1);
	close(gl_slave);
}

