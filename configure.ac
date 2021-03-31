dnl Process this file with autoconf to produce a configure script.
AC_INIT
AC_CONFIG_SRCDIR([zmodem.c])

dnl Initialising output variables
EXTRA_DEPS=""
EXTRA_LIBS=""
EXTRA_CFLAGS=""
TERMCAP_LIB=""

dnl Check configure arguments
AC_MSG_CHECKING(whether to use libreadline for cmd-line editing)
fakereadline=""
AC_ARG_ENABLE(readline,
[
  --enable-readline        Use libreadline [default]
  --disable-readline       Link with a fake readline library
                           useful for testing if libreadline is not available
                           or if you don't want any command-line edition, 
		           history and completion !],
[ case "$enableval" in
  no)
       AC_MSG_RESULT(no)
       fakereadline="yes"
       EXTRA_DEPS="$EXTRA_DEPS fake_readline"
       EXTRA_CFLAGS="$EXTRA_CFLAGS -Ifake_readline"
       EXTRA_LIBS="$EXTRA_LIBS fake_readline/libreadline.a"
       ;;
  *)
       AC_MSG_RESULT(yes)
  esac ],
  AC_MSG_RESULT(yes)
)

dnl Checks for programs.
AC_PROG_INSTALL
AC_CHECK_PROG(MAKE, gmake, gmake, make)
AC_CHECK_PROG(TAR, gtar, gtar, tar)
AC_CHECK_PROG(AR, gar, gar, ar)
AC_CHECK_PROG(RANLIB, granlib, granlib, ranlib)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_PROG_GCC_TRADITIONAL

dnl Check if the C compiler understands -Wall
OLD_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Wall"
AC_CACHE_CHECK([that the C compiler understands -Wall],zssh_cv_wall, [
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[ ]], [[ ]])],[zssh_cv_wall=yes],[zssh_cv_wall=no])])
if test x"$zssh_cv_wall" != x"yes"; then
  CFLAGS="$OLD_CFLAGS"
fi


dnl Checks for header files.
AC_CONFIG_HEADERS([config.h])
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(fcntl.h paths.h sys/ioctl.h sys/time.h termios.h unistd.h \
	err.h sys/cdefs.h sys/param.h util.h libutil.h pty.h)



dnl Checks for libraries.
if test -z "$fakereadline"; then

  dnl libtermcap (or (n)curses) is only needed by libreadline
  BASH_CHECK_LIB_TERMCAP
  LIBS="$LIBS $TERMCAP_LIB"

  dnl check for readline headers
  AC_CHECK_HEADERS(readline/readline.h readline/history.h, , 
	AC_MSG_ERROR(
[
*** the readline library headers were not found. Check 'config.log' for more details.
*** You may use --disable-readline option of configure to build without libreadline
*** or make sure libreadline *and* its headers are properly installed]))

  AC_CHECK_LIB(readline, readline,,
	  AC_MSG_ERROR(
[
*** libreadline was not found. Check 'config.log' for more details.
*** You may use --disable-readline option of configure to build without libreadline
*** or make sure libreadline is properly installed]),
	  )

  dnl Check for completion function names. they are different in version 4.2 and < 4.2
  AC_CHECK_FUNC(rl_username_completion_function, AC_DEFINE(HAVE_READLINE_4_2_OR_GREATER,
    1, [Define this if you have libreadline v4.2 or greater]))
fi



dnl Checks for library functions.
AC_FUNC_GETPGRP
AC_CHECK_FUNCS(strdup tcsetpgrp)


dnl Check if openpty is available for tty/pty allocation
AC_CHECK_FUNCS(openpty,,
  [AC_CHECK_LIB(util,openpty,
    [AC_DEFINE(HAVE_OPENPTY)
    LIBS="$LIBS -lutil"],
    [AC_MSG_ERROR([
*** openpty() was not found.
    ])])]
)


dnl Setting output variables
AC_SUBST(EXTRA_DEPS)
AC_SUBST(EXTRA_CFLAGS)
AC_SUBST(EXTRA_LIBS)
AC_SUBST(TERMCAP_LIB)

AC_CONFIG_FILES([Makefile fake_readline/Makefile test/Makefile])
AC_OUTPUT
