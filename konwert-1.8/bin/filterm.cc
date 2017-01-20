#include <iostream>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pty.h>

#define WERSJA "1.8"

char		*nazwaprogramu;
char		*shell;
int		master;
int		slave = -1;
struct termios	tt;
struct winsize	win;

void uzycie (int status)
{
    (status ? std::cerr : std::cout) << "\
Usage: " << nazwaprogramu << " INPUT OUTPUT [COMMAND [ARGS]]\n\
Execute the specified COMMAND (default is the shell), filtering terminal\n\
input and/or output.\n\
\n\
INPUT and OUTPUT are names of konwert's filters - they are passed as\n\
the first argument to the konwert program. `" << nazwaprogramu << " - OUTPUT' filters\n\
only output, and `" << nazwaprogramu << " INPUT -' only input.\n\
\n\
The command `-' executes the shell as a login shell.\n\
\n\
The input filter will have set the environment variable FILTERM=in,\n\
and the output one - FILTERM=out. This way some filters can slightly\n\
alter their behaviour when working for filterm.\n\
\n\
In addition, the following standard options are recognized:\n\
  --help      display this help and exit\n\
  --version   output version information and exit\n\
";
    exit (status);
}

void wersja()
{
    std::cout << "\
filterm, version " WERSJA "\n\
Copyright 1998 Marcin Kowalczyk <qrczak@knm.org.pl>\n\
";
    exit (0);
}

static char linedata[PATH_MAX];
char *line = linedata;

int getpty(void) {
    if (openpty(&master, &slave, line, NULL, NULL)) {
        return -1;
    } return master;
}


void getmasterslave()
{
    int r;
    r = getpty();
    if (r==-1) { 
        std::cerr << "Error opening pty\n";
        exit (1);
    }

    tcgetattr (0, &tt);
    ioctl (0, TIOCGWINSZ, (char *) &win);
    tcsetattr (slave, TCSAFLUSH, &tt);
    ioctl (slave, TIOCSWINSZ, (char *) &win);
    setsid();
    ioctl (slave, TIOCSCTTY, 0);
}

void terminalsurowy()
{
    struct termios rtt = tt;
    cfmakeraw (&rtt);
    rtt.c_lflag &= ~ECHO;
    tcsetattr (0, TCSAFLUSH, &rtt);
}

void przywrocterminal()
{
    tcsetattr (0, TCSAFLUSH, &tt);
}

void cat (int in, int out)
{
    char buf[BUFSIZ];
    int cc;
    while ((cc = read(in, buf, sizeof (buf))) > 0)
	write (out, buf, cc);
}

void konwert (char *filtr)
{
    execlp ("konwert", "konwert", filtr, 0);
    perror ("konwert");
    exit (1);
}

void komenda (int argc, char *argv[])
{
    dup2 (slave, 0);
    dup2 (slave, 1);
    dup2 (slave, 2);
    close (slave);
    if (!argc)
    {
	char *slash = strrchr (shell, '/');
	if (!slash) slash = shell - 1;
	execl (shell, slash + 1, 0);
    }
    else if (argc == 1 && !strcmp (argv[0], "-"))
    {
	char *slash = strrchr (shell, '/');
	if (!slash) slash = shell - 1;
	char *argv0 = new char[1 + strlen (slash + 1) + 1];
	argv0[0] = '-';
	strcpy (argv[0] + 1, slash + 1);
	execl (shell, argv0, 0);
    }
    else
	execvp (argv[0], argv);
    perror (argc ? argv[0] : shell);
    exit (1);
}

int glowny, input[2], output[2], pidout;

void koniec (int)
{
    if (input[1]) close (input[1]);
    if (glowny) przywrocterminal();
    if (pidout) kill (pidout, SIGTERM);
    if (glowny) while (wait (NULL) != -1);
    exit (0);
}

int
main (int argc, char *argv[])
{
    nazwaprogramu = argv[0];
    shell = getenv ("SHELL");
    if (!shell) shell = "/bin/sh";

    if (argc == 2)
    {
	if (!strcmp (argv[1], "--help")) uzycie (0);
	if (!strcmp (argv[1], "--version")) wersja ();
    }
    if (argc < 3) uzycie (0);

    getmasterslave();
    signal (SIGCHLD, koniec);

    if (strcmp (argv[2], "-"))
    {
	if (pipe (output) == -1)
	{
	    perror ("pipe");
	    exit (1);
	}
        switch (fork())
	{
	    case -1:
		perror ("fork");
		exit (1);
	    case 0:
		close (master);
		close (output[1]);
		dup2 (output[0], 0);
		close (output[0]);
		putenv ("FILTERM=out");
		konwert (argv[2]);
	}
	close (output[0]);
    }
    else output[1] = 1;

    switch (pidout = fork())
    {
	case -1:
	    perror ("fork");
	    exit (1);
	case 0:
	    close (0);
	    if (strcmp (argv[2], "-")) close (1);
	    close (2);
	    cat (master, output[1]);
	    exit (0);
    }
    if (strcmp (argv[2], "-")) close (output[1]);

    switch (fork())
    {
	case -1:
	    perror ("fork");
	    exit (1);
	case 0:
	    pidout = 0;
	    close (master);
	    komenda (argc - 3, argv + 3);
    }

    if (strcmp (argv[1], "-"))
    {
	if (pipe (input) == -1)
	{
	    perror ("pipe");
	    exit (1);
	}
	switch (fork())
	{
	    case -1:
		perror ("fork");
		exit (1);
	    case 0:
		pidout = 0;
		close (input[1]);
		dup2 (input[0], 0);
		close (input[0]);
		dup2 (master, 1);
		close (master);
		putenv ("FILTERM=in");
		konwert (argv[1]);
	}
	close (master);
	close (input[0]);
    }
    else input[1] = master;

    glowny = 1;
    terminalsurowy();
    cat (0, input[1]);
    koniec (0);

    return 0;
}
