
#include <iostream.h>

int kolory[2][8] = { { 0, 1, 2, 1, 4, 1, 2, 7 }, { 0, 5, 6, 6, 5, 5, 6, 3 } };

#define def_color 0x07
#define ulcolor 3
#define halfcolor 4
#define NPAR 16

enum { ESnormal, ESesc, ESsquare, ESgetpars } state;

int color, intensity, underline, s_color, s_intensity, s_underline,
    npar, par[NPAR];

void update_attr()
{
    cout << 30 + (underline ? ulcolor : intensity == 0 ? halfcolor :
	kolory[intensity == 2][color & 0x0F])
	<< ';' << 40 + kolory[0][color >> 4];
}

void default_attr()
{
    color = def_color;
    intensity = 1;
    underline = 0;
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
}

void restore_cur()
{
    color     = s_color;
    intensity = s_intensity;
    underline = s_underline;
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
	    case 21:
	    case 22:
		intensity = 1;
		update_attr();
		break;
	    case 24:
		underline = 0;
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
			restore_cur();
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
}

main()
{
    reset_terminal();
    cout << "\33]P0000000\33]P1AA0000\33]P200AA00\33]P3FFFFFF"
	    "\33]P40000AA\33]P5FF55FF\33]P655FFFF\33]P7AAAAAA"
	    "\33]P8000000\33]P9AA0000\33]PA00AA00\33]PBFFFFFF"
	    "\33]PC0000AA\33]PDFF55FF\33]PE55FFFF\33]PFAAAAAA"
	    "\33[0;";
    update_attr(); cout << "m";
    int c;
    while ((c = cin.get()) != -1) con_write (c);
    cout << "\33]R\33[0m";
}
