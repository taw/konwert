
// TRS - filtr zamieniaj±cy podane ci±gi znaków na inne
//
// -- 
//                                  QRCZAK
//     __("<              Marcin Kowalczyk
//     \__/              qrczak@knm.org.pl
//      ^^      http://qrczak.home.ml.org/

#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#define _(String) (String)

using namespace std;

#define WERSJA "1.8"
#define WLKSLOWA 64*1024	// Maksymalna d³ugo¶æ s³owa do zamiany
				// albo s³owa, na które zamieniamy

char *nazwaprogramu;

void uzycie (int status)
{
	(status ? cerr : cout) << _("\
Usage: ") << nazwaprogramu << _(" [-[r]e] 'REPLACE_THIS WITH_THAT [AND_THIS WITH_THAT]...'\n\
       ") << nazwaprogramu << _(" [-[r]f] FILE\n\
Copy stdin to stdout replacing every occurence of given strings with\n\
other ones. This is similar to tr, but replaces strings, not only\n\
single chars.\n\
\n\
Rules (separated by whitespace) can be given directly after -e option,\n\
or can be read from FILE. Argument not preceded by -e or -f is guessed\n\
to be a script when it contains some whitespace, or a filename otherwise.\n\
\n\
Comments are allowed from # until the end of line. The character `#' in\n\
strings must be specified as `\\#'.\n\
\n\
Standard C-like escapes \\a \\b \\e \\f \\n \\r \\t \\v \\\\ \\nnn are recognized.\n\
In addition, \\s means a space character and \\! means an empty string.\n\
\n\
Sets of acceptable characters at a given position can be specified\n\
between \\[ and \\]. ASCII ranges in sets can be shortly written as\n\
FIRST\\-LAST. When a set consists of only a single range, \\[ and \\]\n\
can be omitted.\n\
\n\
When a part of the string to translate is enclosed in \\{...\\}, only that\n\
part is replaced. Any text outside \\{...\\} serve as an assertion:\n\
a string is translated only if it is preceded by the given text and\n\
followed by another one. \\{ at the beginning or \\} at the end of the\n\
string can be omitted.\n\
\n\
A fragment of the form \?x=N, where x is a letter A-Za-z and N is a\n\
digit 0-9, contained in the target text sets the variable x to the value\n\
N when that rule succeeds. Similar fragment in the source text causes\n\
the given rule to be considered only if that variable has such value.\n\
Initially all variables have the value of 0. Several assignments or\n\
conditions can be present in one rule - they are ANDed together.\n\
\n\
Multiple -e or -f options are allowed. All rules are loaded together\n\
then, and earlier ones have precedence.\n\
\n\
Option -r means to reverse every rule and affects only the next -e or -f option.\n\
Of course this doesn't have to give the reverse translation! Any rule\n\
containing any of \\{\\}\\[\\]\\- is taken in only one direction.\n\
\n\
In addition, the following standard options are recognized:\n\
  --help      display this help and exit\n\
  --version   output version information and exit\n\
\n\
Example:\n") << "\
$ echo Leeloo |" << nazwaprogramu << " -e 'el n e i i aqq o\\)\\n x o u'\n\
Linux\n\
";
	exit (status);
}

void wersja ()
{
	cout << _("\
trs, version " WERSJA "\n\
Copyright 1998 Marcin Kowalczyk <qrczak@knm.org.pl>\n\
");
	exit (0);
}

void blad (char *s)
{
	cerr << nazwaprogramu << ": " << s << endl;
	exit (1);
}

/******** BUFOR **************************************************************/

// Pozwala czytaæ plik pó³-sekwencyjnie, pó³-swobodnie. Mo¿liwe s± powroty
// najwy¿ej o z góry ustalon± odleg³o¶æ.

class bufor
{
private:
	istream &str;
	char *buf;
	int wlk, poz;
public:
	bufor (int w, istream &s = cin) :
		str (s), buf (new char[w]), wlk (w), poz (0) {}
	char operator[] (int i);
	int niekoniec (int i);
};

char bufor::operator[] (int i)
{
	while (i >= poz)
	{
		if (str.eof ()) return '\n';
		int c = str.get ();
		if (str.eof ()) return '\n';
		buf[poz++ % wlk] = c;
	}
	return i >= 0 ? buf[i % wlk] : '\n';
}

int bufor::niekoniec (int i)
{
	(*this)[i];
	return i < poz;
}

/******** REGU£Y *************************************************************/

#define ZMIENNYCH (26+26)

struct zmienna
{
	int zmienna2, wartosc;
	zmienna *nast;
};

struct regula
{
	zmienna *warunki;
	char *co;
	int dco;
	int ileprzed, ilezast;
	char *naco;
	int dnaco;
	zmienna *zmienne;
	struct regula *nast;
};

regula *reguly = 0, *r;

int rozrzut = 1;        /* Maksymalna odleg³o¶æ, o jak± mo¿emy siê cofn±æ przy
						   czytaniu wej¶cia, = wielko¶æ bufora wej¶ciowego */
int odwrotnie = 0;

/* Tworzymy nastêpn± regu³ê - w zmiennej r */
void nastregula ()
{
	regula *nowa = new regula;
	if (reguly) r->nast = nowa; else reguly = nowa;
	r = nowa;
	r->nast = 0;
}

/* Anulujemy ostatnio stworzon± regu³ê */
void anulujregule ()
{
	if (!reguly->nast)
	{
		delete reguly;
		reguly = 0;
	}
	else
	{
		for (r = reguly; r->nast->nast; r = r->nast);
		delete r->nast;
		r->nast = 0;
	}
}

/* Rozwija \! \nnn \a \b \e \f \n \r \s \t \v \-, pozostawia \[ \] \\.
   Je¶li to jest "co" (a nie "na co"), to w ileprzed i ilezast zapamiêtuje
   pozycjê \{ i d³ugo¶æ tekstu w \{\}.
   Zwraca 1 gdy regu³a ma byæ anulowana z powodu \{\}\[\]\- w "na co"
   (co oznacza, ¿e ta regu³a mia³aby sens tylko w przeciwn± stronê) */
int rozwineskejpy (char *napis1, char *&napis2, int &dlug2, zmienna **zmienne,
		int co, int *ileprzed, int *ilezast)
{
	char buf[WLKSLOWA];
	char *s1 = napis1, *s2 = buf;
	int poz = 0, niewzbiorze = 1;
	int c = -1,		/* Ostatnio zapisany znak. Je¶li c == -1,
				   to teraz \- nie bêdzie mia³o sensu */
	przedzial = -1;		/* Pocz±tek przedzia³u, je¶li w³a¶nie by³o \-,
				   albo -1 je¶li nie */
	if (co) *ileprzed = -1, *ilezast = -1;

	while (*s1)
	{
		if (s2 - buf > WLKSLOWA - 2) blad (_("Rule too long"));
		if (*s1 == '\\')
		{
			s1++;
			switch (*s1++)
			{
				case '!':
					goto pomin;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
					c = s1[-1] - '0';
					if (*s1 >= '0' && *s1 <= '7')
					{
						c *= 8;
						c += *s1++ - '0';
						if (*s1 >= '0' && *s1 <= '7')
						{
							c *= 8;
							c += *s1++ - '0';
						}
					}
					if (co && c == '\\') *s2++ = '\\';
					break;
				case 'x':
					if (isxdigit (*s1))
					{
						c = *s1 < 0x40 ? *s1 - '0' : (*s1 & ~0x20) - 'A' + 10;
						s1++;
						if (isxdigit (*s1))
						{
							c *= 16;
							c += *s1 < 0x40 ? *s1 - '0' : (*s1 & ~0x20) - 'A' + 10;
							s1++;
						}
						if (co && c == '\\') *s2++ = '\\';
					}
					else
						c = 'x';
					break;
				case 'a':
					c = '\a';
					break;
				case 'b':
					c = '\b';
					break;
				case 'e':
					c = '\33';
					break;
				case 'f':
					c = '\f';
					break;
				case 'n':
					c = '\n';
					break;
				case 'r':
					c = '\r';
					break;
				case 's':
					c = ' ';
					break;
				case 't':
					c = '\t';
					break;
				case 'v':
					c = '\v';
					break;
				case '{':
					if (!co) return 1;
					if (*ileprzed != -1) blad (_("Multiple \\{'s"));
					if (*ilezast != -1) blad (_("\\{ after \\}"));
					*ileprzed = poz;
					goto pomin;
				case '}':
					if (!co) return 1;
					if (*ilezast != -1) blad (_("Multiple \\}'s"));
					*ilezast = poz - (*ileprzed != -1 ? *ileprzed : 0);
					goto pomin;
				case '[':
					if (!co) return 1;
					if (!niewzbiorze) blad (_("Extra \\[ or missing \\]"));
					niewzbiorze = 0;
					*s2++ = '\\', *s2++ = '[';
					przedzial = -1, c = -1;
					goto pomin;
				case ']':
					if (niewzbiorze) blad (_("Extra \\] or missing \\["));
					niewzbiorze = 1;
					*s2++ = '\\', *s2++ = ']';
					poz++;
					przedzial = -1, c = -1;
					goto pomin;
				case '-':
					if (!co) return 1;
					/* Tutaj niby powinno byæ if (przedzial == -1 && c != -1),
					   ale je¶li c == -1, to i tak przedzial == -1,
					   a je¶li przedzial != -1, to i tak c == przedzial. */
					przedzial = c;
					goto pomin;
				case '?':
					if
					(
						(
							s1[0] >= 'A' && s1[0] <= 'Z' ||
							s1[0] >= 'a' && s1[0] <= 'z'
						) &&
						s1[1] == '=' &&
						s1[2] >= '0' && s1[2] <= '9'
					)
					{
						*zmienne = new zmienna;
						(*zmienne)->zmienna2 =
							s1[0] >= 'A' && s1[0] <= 'Z' ?
								s1[0] - 'A'
							:
								26 + (s1[0] - 'a');
						(*zmienne)->wartosc = s1[2] - '0';
						zmienne = &(*zmienne)->nast;
						s1 += 3;
						goto pomin;
					}
					else
						blad (_("Syntax error after \\?"));
				case '\\':
					if (co) *s2++ = '\\';
				default:
					c = (unsigned char) s1[-1];
			}
		}
		else
			c = (unsigned char) *s1++;
		if (przedzial != -1)
		{
			s2--;
			if (przedzial == '\\') s2--;
			if (niewzbiorze) *s2++ = '\\', *s2++ = '[';
			for (; przedzial <= c; przedzial++)
			{
				if (s2 - buf > WLKSLOWA - 4) blad (_("Rule too long!"));
				if (przedzial == '\\') *s2++ = '\\';
				*s2++ = przedzial;
			}
			if (niewzbiorze) *s2++ = '\\', *s2++ = ']';
			przedzial = -1, c = -1;
		}
		else
		{
			*s2++ = c;
			poz += niewzbiorze;
		}
	  pomin:;
	}
	if (!niewzbiorze) blad (_("Extra \\[ or missing \\]"));

	memcpy (napis2 = new char[s2 - buf], buf, s2 - buf);
	dlug2 = poz;
	(*zmienne) = NULL;
	if (co)
	{
		if (*ileprzed == -1) *ileprzed = 0;
		if (*ilezast == -1) *ilezast = poz - *ileprzed;
	}
	return 0;
}

/******** NIECIEKAWE ZNAKI MO¯NA PRZEPU¦CIÆ BEZ ZMIAN ************************/

char ciekawe[256];

void jakieciekawe (char *s, int ile, int ileprzed)
{
	if (ileprzed >= ile)
	{
		for (int c = 0; c < 0x100; c++) ciekawe[c] = 1;
		return;
	}

	for (; ileprzed; s++, ileprzed--)
		if (*s == '\\' && *++s == '[')
		{
			s++;
			while (!(*s == '\\' && *++s == ']')) s++;
		}

	if (*s == '\\' && *++s == '[')
	{
		s++;
		while (!(*s == '\\' && *++s == ']'))
			ciekawe[(unsigned char) *s++] = 1;
	}
	else
		ciekawe[(unsigned char) *s] = 1;
}

/******** CZYTAMY REGU£Y *****************************************************/

int czytajslowo (istream &f, char *s, int n)
{
	int c;
	for(;;)
	{
		for(;;)
		{
			if ((c = f.get ()) == -1) return 1;
			if (c != '#') break;
			do
				if ((c = f.get ()) == -1) return 1;
			while (c != '\n');
		}
		if (!isspace (c)) break;
	}

	char *s1 = s;
	for (;;)
	{
		if (c == '\\')
		{
			if ((c = f.get ()) == -1) break;
			*s1++ = '\\';
		}
		*s1++ = c;
		if ((c = f.get ()) == -1 || isspace (c)) break;
		if (s1 - s > n - 2) blad (_("Rule too long"));
	}
	*s1 = '\0';
	return 0;
}

void czytajreguly (istream &f)
{
	int maxwstecz = 0, maxwprzod = 1;
	for (;;)
	{
		char s[WLKSLOWA];
		if (czytajslowo (f, s, WLKSLOWA)) break;
		nastregula ();
		if (!odwrotnie)
		{
			rozwineskejpy (s, r->co, r->dco, &r->warunki, 1, &r->ileprzed, &r->ilezast);
			if (czytajslowo (f, s, WLKSLOWA)) blad (_("Incomplete rule"));
			if (rozwineskejpy (s, r->naco, r->dnaco, &r->zmienne, 0, 0, 0))
			{
				anulujregule ();
				continue;
			}
		}
		else
		{
			int beztej = rozwineskejpy (s, r->naco, r->dnaco, &r->zmienne, 0, 0, 0);
			if (czytajslowo (f, s, WLKSLOWA)) blad (_("Incomplete rule"));
			if (beztej)
			{
				anulujregule ();
				continue;
			}
			rozwineskejpy (s, r->co, r->dco, &r->warunki, 1, &r->ileprzed, &r->ilezast);
		}
		if (!r->dco) blad (_("Empty string to translate"));
		/* ¦ci¶le bior±c to !ilezast jest tym przypadkiem, ale kombinacja
		   !ilezast && dco jest specjalnie obs³u¿ona i dzia³a */
		if (r->ileprzed > maxwstecz) maxwstecz = r->ileprzed;
		if (r->dco - r->ileprzed > maxwprzod)
			maxwprzod = r->dco - r->ileprzed;
		jakieciekawe (r->co, r->dco, r->ileprzed);
	}
	if (maxwstecz + maxwprzod > rozrzut) rozrzut = maxwstecz + maxwprzod;
}

void regulyzarg (char *s)
{
	istringstream f (s);
	czytajreguly (f);
}

/*
void regulyzarg1 (char **argv, int &i, int argc)
{
	for (; i < argc && *argv[i];)
	{
		nastregula ();
		if (!odwrotnie)
		{
			r->co = argv[i++];
			r->dco = strlen (r->co);
			r->ileprzed = 0;
			r->ilezast = r->dco;
			if (!(i < argc && *argv[i])) blad (_("Incomplete rule"));
			r->naco = argv[i++];
			r->dnaco = strlen (r->naco);
		}
		else
		{
			r->naco = argv[i++];
			r->dnaco = strlen (r->naco);
			if (!(i < argc && *argv[i])) blad (_("Incomplete rule"));
			r->co = argv[i++];
			r->dco = strlen (r->co);
			r->ileprzed = 0;
			r->ilezast = r->dco;
		}
		if (r->dco > rozrzut) rozrzut = r->dco;
		jakieciekawe (r->co, r->dco, r->ileprzed);
	}
}
*/

void regulyzpliku (char *nazwa)
{
	ifstream f (nazwa);
	if (!f)
	{
		cerr << nazwaprogramu << ": " << _("Couldn't open file ") << nazwa << endl;
		exit (1);
	}
	czytajreguly (f);
}

/******** W£A¦CIWA KONWERSJA *************************************************/

void trs ()
{
	int zmienne[ZMIENNYCH];
	for (int z = 0; z < ZMIENNYCH; z++)
		zmienne[z] = 0;
	int mozebycpuste = 1;
	bufor buf (rozrzut);
	int poz = 0;
	while (buf.niekoniec (poz))
	{
		if (ciekawe[(unsigned char) buf[poz]])
		{
			for (regula *r = reguly; r; r = r->nast)
			{
				if (r->ilezast || mozebycpuste)
				{
					zmienna *z = r->warunki;
					while (z)
					{
						if (zmienne[z->zmienna2] != z->wartosc)
							goto niepasuje;
						z = z->nast;
					}
					{
						int j = poz - r->ileprzed;
						char *s;
						int ls;
						for (s = r->co, ls = r->dco; ls; j++, s++, ls--)
						{
							char c = buf[j];
							if (*s == '\\' && *++s == '[')
							{
								s++;
								while (!(*s == '\\' && *++s == ']'))
									if (c == *s++) goto wzbiorze;
								goto niepasuje;
							  wzbiorze:
								while (!(*s == '\\' && *++s == ']')) s++;
							}
							else if (c != *s) goto niepasuje;
						}
						for (s = r->naco, ls = r->dnaco; ls; s++, ls--)
								cout.put (*s);
					}
					if ((mozebycpuste = r->ilezast))
						poz += r->ilezast;
					z = r->zmienne;
					while (z)
					{
						zmienne[z->zmienna2] = z->wartosc;
						z = z->nast;
					}
					goto zamienione;
				  niepasuje:;
				}
			}
		}
		cout.put (buf[poz++]);
		mozebycpuste = 1;
	  zamienione:;
	}
}

/******** PROGRAM G£ÓWNY *****************************************************/

int
main (int argc, char **argv)
{
/*
	setlocale (LC_ALL, "");
	bindtextdomain (PACKAGE, LOCALEDIR);
	textdomain (PACKAGE);
*/
	nazwaprogramu = argv[0];
	if (argc == 2)
	{
		if (!strcmp (argv[1], "--help")) uzycie (0);
		if (!strcmp (argv[1], "--version")) wersja ();
	}
	if (argc == 1) uzycie (0);

	memset (ciekawe, 0, sizeof (ciekawe));
	for (;;)
	{
		switch (getopt (argc, argv, "e:f:r"))
		{
			case -1:
				goto koniecopcji;
			case 'e':
				regulyzarg (optarg);
				odwrotnie = 0;
				break;
			case 'f':
				regulyzpliku (optarg);
				odwrotnie = 0;
				break;
			case 'r':
				odwrotnie = !odwrotnie;
				break;
			case '?':
				exit (1);
		}
	}
  koniecopcji:
	for (; optind < argc; optind++)
		if (!argv[optind][0] || strpbrk (argv[optind], " \f\n\r\t\v"))
			regulyzarg (argv[optind]);
		else
			regulyzpliku (argv[optind]);

	trs ();

	return 0;
}
