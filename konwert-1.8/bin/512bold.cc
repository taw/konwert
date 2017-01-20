
#include <stdlib.h>
#include <iostream.h>

#define NPAR 16

int color, intensity, underline, reverse,
    s_color, s_intensity, s_underline, s_reverse,
    npar, par[NPAR];

int def_color = 7;
int ulcolor = 15;
int halfcolor = 8;

int nanowe[16] = {0, 1, 2, 3, 4, 5, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1};
int nastare[8] = {0, 1, 2, 3, 4, 5, 6, 7};
int poprzedni[9] = {8, 0, 1, 2, 3, 4, 5, 6, 7};
int nastepny[9] = {1, 2, 3, 4, 5, 6, 7, 8, 0};
int zmien[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int zmiencos = 0;
int odleglosc[16][16];

int kolory (int stary)
{
    int nowy = nanowe[stary];
    if (nowy < 0)
    {
	int najlepiej = 100;
	int i = nastepny[8];
	int ktory = 0;
	while (i != 8)
	{
	    int odl = odleglosc[stary][nastare[i]] + ktory;
	    if (odl < najlepiej)
	    {
		najlepiej = odl;
		nowy = i;
	    }
	    i = nastepny[i];
	    ktory++;
	}
	nanowe[nastare[nowy]] = -1;
	nanowe[stary] = nowy;
	nastare[nowy] = stary;
	zmien[nowy] = 1;
	zmiencos = 1;
    }
    if (nowy != poprzedni[8])
    {
	nastepny[poprzedni[nowy]] = nastepny[nowy];
	poprzedni[nastepny[nowy]] = poprzedni[nowy];
	nastepny[nowy] = 8;
	poprzedni[nowy] = poprzedni[8];
	nastepny[poprzedni[8]] = nowy;
	poprzedni[8] = nowy;
    }
    return nowy;
}

void update_pal()
{
    const char hex[] = "0123456789ABCDEF";
    const char *normalny[16] =
    {
	"000000",
	"AA0000",
	"00AA00",
	"AA5500",
	"0000AA",
	"AA00AA",
	"00AAAA",
	"AAAAAA",
	"555555",
	"FF5555",
	"55FF55",
	"FFFF55",
	"5555FF",
	"FF55FF",
	"55FFFF",
	"FFFFFF"
    };

    for (int i = 0; i < 8; i++)
    	if (zmien[i])
    	{
	    cout << "\33]P" << hex[i] << normalny[nastare[i]];
	    zmien[i] = 0;
    	}
    zmiencos = 0;
}

void update_attr()
{
    int attr = color;
    if (reverse) attr = (attr & 0x88) | (((attr >> 4) | (attr << 4)) & 0x77);
    cout << 30 + kolory
    (
	(underline ? ulcolor :
	intensity == 0 ? halfcolor :
	(attr & 0x0F)) ^ (intensity == 2) * 8
    ) << ';' << 40 + kolory (attr >> 4);
}

enum { ESnormal, ESesc, ESsquare, ESgetpars } state;

void default_attr()
{
    color = def_color;
    intensity = 1;
    underline = 0;
    reverse = 0;
}

void reset_terminal()
{
    state = ESnormal;
    default_attr();
}

void save_cur()
{
    s_color     = color;
    s_intensity = intensity;
    s_underline = underline;
    s_reverse   = reverse;
}

void restore_cur()
{
    color     = s_color;
    intensity = s_intensity;
    underline = s_underline;
    reverse   = s_reverse;
    update_attr();
}

void csi_m()
{
    for (int i = 0; i <= npar; i++)
    {
	switch (par[i])
	{
	    case 0:	/* all attributes off */
		cout << "0;";
		default_attr();
		update_attr();
		break;
	    case 1:
		intensity = 2;
		update_attr();
		break;
	    case 2:
		intensity = 0;
		update_attr();
		break;
	    case 4:
		underline = 1;
		update_attr();
		break;
	    case 7:
		reverse = 1;
		update_attr();
		break;
	    case 21:
	    case 22:
		intensity = 1;
		update_attr();
		break;
	    case 24:
		underline = 0;
		update_attr();
		break;
	    case 27:
		reverse = 0;
		update_attr();
		break;
	    case 38:
		color = (def_color & 0x0F) | (color & 0xF0);
                underline = 1;
		update_attr();
		break;
	    case 39:
		color = (def_color & 0x0F) | (color & 0xF0);
		underline = 0;
		update_attr();
		break;
	    case 49:
		color = (def_color & 0xF0) | (color & 0x0F);
		update_attr();
		break;
	    default:
		if (par[i] >= 30 && par[i] <= 37)
		{
		    color = (par[i] - 30) | color & 0xF0;
		    update_attr();
		}
		else if (par[i] >= 40 && par[i] <= 47)
		{
		    color = ((par[i] - 40) << 4) | color & 0x0F;
		    update_attr();
		}
		else
		    cout << par[i];
	}
	if (i != npar) cout << ';';
    }
    npar = -1;
}

void setterm_command()
{
    switch (par[0])
    {
	case 1: /* set color for underline mode */
	    if (par[1] < 16)
	    {
		ulcolor = par[1];
		if (underline)
		{
		    update_attr();
		    cout << "m\33[";
		}
	    }
	    break;
	case 2: /* set color for half intensity mode */
	    if (par[1] < 16)
	    {
		halfcolor = par[1];
		if (intensity == 0)
		{
		    update_attr();
		    cout << "m\33[";
		}
	    }
	    break;
	case 8: /* store colors as defaults */
	    def_color =
	    (
		(underline ? ulcolor :
		intensity == 0 ? halfcolor :
		(color & 0x0F)) ^ (intensity == 2) * 8
	    ) | (color & 0xF0);
	    cout << 30 + (def_color & 0x07) << ';' << 40 + (def_color >> 4);
	    if (def_color & 0x08) cout << ";1";
	    cout << "m\33[8]\33[";
	    if (def_color & 0x08) cout << "22;";
	    default_attr();
	    update_attr();
	    cout << 'm';
	    return;
    }
    for (int i = 0; i <= npar; i++)
    {
	cout << par[i];
	if (i < npar) cout << ';';
    }
    cout << ']';
}

void con_write (unsigned char c)
{
    switch (c)
    {
	case 24:
	case 26:
	    state = ESnormal;
	    break;
	case 27:
	    state = ESesc;
	    break;
	case 128+27:
	    state = ESsquare;
	    break;
    }
    if (c >= 0x20 && c <= 0x7E)
    {
	switch (state)
	{
	    case ESnormal:
		break;
	    case ESesc:
		state = ESnormal;
		switch (c)
		{
		    case '[':
			state = ESsquare;
			break;
		    case 'c':
			reset_terminal();
			break;
		    case '7':
			save_cur();
			break;
		    case '8':
		    	cout << "8\33[";
			restore_cur();
			c = 'm';
			break;
		}
		break;
	    case ESsquare:
		if (c != ';' && (c < '0' || c > '9'))
		{
		    state = ESnormal;
		    break;
		}
		for (npar = 0; npar < NPAR; npar++) par[npar] = 0;
		npar = -1;
		state = ESgetpars;
	    case ESgetpars:
		if (c == ';')
		{
		    if (npar == -1) npar = 0;
		    if (npar < NPAR - 1) npar++;
		    return;
		}
		else if (c >= '0' && c <= '9')
		{
		    if (npar == -1) npar = 0;
		    par[npar] *= 10;
		    par[npar] += c - '0';
		    return;
		}
		else
		{
		    state = ESnormal;
		    switch(c)
		    {
			case 'm':
			    csi_m();
			    break;
			case 's':
			    save_cur();
			    break;
			case 'u':
			    restore_cur();
			    break;
			case ']':
			    setterm_command();
			    return;
		    }
		    for (int i = 0; i <= npar; i++)
		    {
			cout << par[i];
			if (i < npar) cout << ';';
		    }
		}
        }
    }
    cout << c;
    if (zmiencos) update_pal();
}

void licz_odleglosci()
{
    const int normalny[16][3] =
    {
	{0, 0, 0},
	{2, 0, 0},
	{0, 2, 0},
	{2, 1, 0},
	{0, 0, 2},
	{2, 0, 2},
	{0, 2, 2},
	{2, 2, 2},
	{1, 1, 1},
	{3, 1, 1},
	{1, 3, 1},
	{3, 3, 1},
	{1, 1, 3},
	{3, 1, 3},
	{1, 3, 3},
	{3, 3, 3}
    };
    for (int i = 0; i < 16; i++)
	for (int j = 0; j < 16; j++)
	    odleglosc[i][j] = abs (normalny[i][0] - normalny[j][0]) +
	                      abs (normalny[i][1] - normalny[j][1]) +
	                      abs (normalny[i][2] - normalny[j][2]);
}

main()
{
    licz_odleglosci();
    reset_terminal();
    cout << "\30\33]R\33[0;"; update_attr(); cout << "m";
    update_pal();
    int c;
    while ((c = cin.get()) != -1) con_write (c);
    cout << "\30\33]R\33[0m";
}
