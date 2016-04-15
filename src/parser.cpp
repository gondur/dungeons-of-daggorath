/****************************************
Daggorath PC-Port Version 0.2.1
Richard Hunerlach
November 13, 2002

The copyright for Dungeons of Daggorath
is held by Douglas J. Morgan.
(c) 1982, DynaMicro
*****************************************/

// Dungeons of Daggorath
// PC-Port
// Filename: parser.cpp
//
// Implementation of Parser class

#include "parser.h"
#include "viewer.h"

extern Viewer viewer;

// Constructor
Parser::Parser() : LINPTR(0),
				   PARFLG(0),
				   PARCNT(0),
				   VERIFY(0),
				   FULFLG(0),
				   KBDHDR(0),
				   KBDTAL(0),
				   BUFFLG(0),
				   LINEND(0),
				   TOKEND(0)
{
	int ctr;
	for (ctr = 0; ctr < 33; ++ctr)
	{
		KBDBUF[ctr] = 0;
		LINBUF[ctr] = 0;
		TOKEN[ctr] = 0;
		OBJSTR[ctr] = 0;
		STRING[ctr] = 0;
	}
	STRING[33] = 0;
	STRING[34] = 0;
	for (ctr = 0; ctr < 11; ++ctr)
	{
		SWCHAR[ctr] = 0;
	}

	M_PROM1[0] = I_CR;
	M_PROM1[1] = I_DOT;
	M_PROM1[2] = I_BAR;
	M_PROM1[3] = I_BS;
	M_PROM1[4] = I_NULL;

	M_CURS[0] = I_BAR;
	M_CURS[1] = I_BS;
	M_CURS[2] = I_NULL;

	M_ERAS[0] = I_SP;
	M_ERAS[1] = I_BS;
	M_ERAS[2] = I_BS;
	M_ERAS[3] = I_BAR;
	M_ERAS[4] = I_BS;
	M_ERAS[5] = I_NULL;

	Utils::LoadFromHex(CERR, "177BD0");
	Utils::LoadFromHex(CMDTAB, "0F30034A046B2806C4B440200927C0380B80B52E28180E5A003012E185D42018F7AC201AFB142021563030245B142C202747DC20295938182B32802834C78480283530D8A0");
	Utils::LoadFromHex(DIRTAB, "0620185350282493A280200411AC300327D5C4102B002008FBB8");

/*	Utils::LoadFromHex(CERR,"177BD0");

	Utils::LoadFromHex(CMDTAB, 
		"0F30034A046B2806C4B440200927C0380B80B52E28180E5A003012E185D42018F7AC20"
		"1AFB142021563030245B142C202747DC20295938182B32802834C78480283530D8A0"
		);

	Utils::LoadFromHex(DIRTAB,
		"0620185350282493A280200411AC300327D5C4102B002008FBB8");*/
}

void Parser::Reset()
{
	LINPTR = 0;
	PARFLG = 0;
	PARCNT = 0;
	VERIFY = 0;
	FULFLG = 0;
	KBDHDR = 0;
	KBDTAL = 0;
	BUFFLG = 0;
	LINEND = 0;
	TOKEND = 0;
	int ctr;
	for (ctr = 0; ctr < 33; ++ctr)
	{
		KBDBUF[ctr] = 0;
		LINBUF[ctr] = 0;
		TOKEN[ctr] = 0;
		OBJSTR[ctr] = 0;
		STRING[ctr] = 0;
	}
	STRING[33] = 0;
	STRING[34] = 0;
	for (ctr = 0; ctr < 11; ++ctr)
	{
		SWCHAR[ctr] = 0;
	}
}

// This method puts a character into the DoD buffer
void Parser::KBDPUT(dodBYTE c)
{
	KBDBUF[KBDTAL] = c;
	++KBDTAL;
	KBDTAL &= 31;
}

// This method gets a character from the DoD buffer
dodBYTE Parser::KBDGET()
{
	dodBYTE c = 0;
	if (KBDHDR == KBDTAL)
		return c;
	c = KBDBUF[KBDHDR];
	++KBDHDR;
	KBDHDR &= 31;
	return c;
}

// The rest of these methods are direct ports from the source,
// including all the GOTOs.  Someday, these should probably be
// updated to a more C/C++ programming style, but for the moment
// they work just fine.
//
int Parser::PARSER(dodBYTE * pTABLE, dodBYTE & A, dodBYTE & B, bool norm)
{
	bool	tok;
	int		U, Xup, Y;
	dodBYTE	retA, retB;

	if (norm)
	{
		A = 0;
		B = 0;
		tok = GETTOK();
		if (tok == false)
		{
			return 0;
		}
	}
	else
	{
		A = 0;
	}

	PARFLG = 0;
	FULFLG = 0;
	B = *pTABLE;
	++pTABLE;
	PARCNT = B;

PARS10:
	U = 0;
	EXPAND(pTABLE, &Xup, 0);
	pTABLE += Xup;
	Y = 2;
	
PARS12:
	B = TOKEN[U++];
	if (B == 0xFF)
	{
		goto PARS20;
	}
	if (B != STRING[Y++])
	{
		goto PARS30;
	}
	if (STRING[Y] != I_NULL && STRING[Y] != 0)
	{
		goto PARS12;
	}
	if (TOKEN[U] != 0xFF && TOKEN[U] != 0)
	{
		goto PARS30;
	}
	--FULFLG;

PARS20:
	if (PARFLG != 0)
	{
		goto PARS90;
	}
	++PARFLG;
	B = STRING[1];
	retA = A;
	retB = B;

PARS30:
	++A;
	--PARCNT;
	if (PARCNT != 0)
	{
		goto PARS10;
	}

	if (PARFLG != 0)
	{
		A = retA;
		B = retB;
		return 1;
	}

PARS90:
	A = 0xFF;
	B = 0xFF;
	return -1;
}

bool Parser::GETTOK()
{
	int		U = 0;
	int		X = LINPTR;
	dodBYTE	A;
	
	do
	{
		A = LINBUF[X++];
	} while (A == 0);
	goto GTOK22;

GTOK20:
	A = LINBUF[X++];

GTOK22:
	if (A == 0 || A == 0xFF)
	{
		goto GTOK30;
	}
	TOKEN[U++] = A;
	if (U < 32)
	{
		goto GTOK20;
	}

GTOK30:
	TOKEN[U++] = 0xFF;
	LINPTR = X;

	if (TOKEN[0] == 0xFF)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void Parser::EXPAND(dodBYTE * X, int * Xup, dodBYTE * U)
{
	dodBYTE * Y;
	dodBYTE	A, B;
	int Xup2;

	*Xup = 0;

	if (U != 0)
	{
		Y = (U - 1);
	}
	else
	{
		Y = &STRING[0];
		U = Y + 1;
	}
	*Y = 0;
	B = GETFIV(X, &Xup2, Y);
	X += Xup2;
	A = B;

EXPAN10:
	B = GETFIV(X, &Xup2, Y);
	X += Xup2;
	*Xup += Xup2;
	*U = B;
	++U;
	--A;
	if (A != 0xFF)
	{
		goto EXPAN10;
	}
	*U = A;

	if ( (*Y) != 0)
	{
		++X;
		++*Xup;
	}
}

dodBYTE Parser::GETFIV(dodBYTE * X, int * Xup, dodBYTE * zeroY)
{
	dodBYTE		A, B;
	
	*Xup = 0;

	A = *zeroY;

	switch (A)
	{
	case 0:
		B = *X;
		B = (B >> 3);
		break;
	case 1:
		A = *X;
		++X;
		++*Xup;
		B = *X;
		ASRD(A, B, 6);
		break;
	case 2:
		B = *X;
		B = (B >> 1);
		break;
	case 3:
		A = *X;
		++X;
		++*Xup;
		B = *X;
		ASRD(A, B, 4);
		break;
	case 4:
		A = *X;
		++X;
		++*Xup;
		B = *X;
		ASRD(A, B, 7);
		break;
	case 5:
		B = *X;
		B = (B >> 2);
		break;
	case 6:
		A = *X;
		++X;
		++*Xup;
		B = *X;
		ASRD(A, B, 5);
		break;
	case 7:
		B = *X;
		++X;
		++*Xup;
		break;
	}

	A = *zeroY;
	++A;
	A = (A & 7);
	*zeroY = A;

	return (B & 0x1F);
}

void Parser::ASRD(dodBYTE & A, dodBYTE & B, int num)
{
	signed short D = ((signed short)A<<8) + B;
	signed short sign = D & 0x8000;

	while (num--)
		D = (D>>1) |sign;

	A=D>>8;
	B=(dodBYTE)D;
}

void Parser::CMDERR()
{
	viewer.OUTSTI(CERR);
}

int Parser::PARHND()
{
	int		res;
	dodBYTE	A, B;
	
	res = PARSER(DIRTAB, A, B, true);
	if (res != 1)
	{
		CMDERR();
		return -1;
	}
	if (A == 0 || A == 1)
	{
		return A;
	}
	else
	{
		CMDERR();
		return -1;
	}
}

