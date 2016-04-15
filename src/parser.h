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
// Filename: parser.h
//
// This class will manage the command parser

#ifndef DOD_PARSER_HEADER
#define DOD_PARSER_HEADER

#include "dod.h"

class Parser
{
public:
	// Constructor
	Parser();

	// Public Interface
	void	KBDPUT(dodBYTE c);
	dodBYTE	KBDGET();
	void	EXPAND(dodBYTE * X, int * Xup, dodBYTE * U);
	dodBYTE	GETFIV(dodBYTE * X, int * Xup, dodBYTE * zeroY);
	void	ASRD(dodBYTE & A, dodBYTE & B, int num);
	bool	GETTOK();
	int		PARSER(dodBYTE * X, dodBYTE &A, dodBYTE &B, bool norm);
	void	CMDERR();
	int		PARHND();
	void	Reset();

	// Public Data Member
	dodSHORT	LINPTR;
	dodBYTE		PARFLG;
	dodBYTE		PARCNT;
	dodBYTE		VERIFY;
	dodBYTE		FULFLG;
	dodBYTE		KBDHDR;
	dodBYTE		KBDTAL;
	dodBYTE		BUFFLG;
	dodBYTE		KBDBUF[33];
	dodBYTE		LINBUF[33];
	dodSHORT	LINEND;
	dodBYTE		TOKEN[33];
	dodBYTE		TOKEND;
	dodBYTE		STRING[35];
	dodBYTE		SWCHAR[11];
	dodBYTE		OBJSTR[33];
	dodBYTE		CMDTAB[69];
	dodBYTE		DIRTAB[26];

	enum {
		C_BS=0x08,
		C_CR=0x0D,
		C_SP=0x20,

		I_SP=0x00,
		I_BAR=0x1C,
		I_DOT=0x1E,
		I_CR=0x1F,
		I_EXCL=0x1B,
		I_QUES=0x1D,
		I_SHL=0x20,
		I_SHR=0x21,
		I_LHL=0x22,
		I_LHR=0x23,
		I_BS=0x24,
		I_NULL=0xff, // string terminator char

		CMD_ATTACK=0,
		CMD_CLIMB,
		CMD_DROP,
		CMD_EXAMINE,
		CMD_GET,
		CMD_INCANT,
		CMD_LOOK,
		CMD_MOVE,
		CMD_PULL,
		CMD_REVEAL,
		CMD_STOW,
		CMD_TURN,
		CMD_USE,
		CMD_ZLOAD,
		CMD_ZSAVE,

		DIR_LEFT=0,
		DIR_RIGHT,
		DIR_BACK,
		DIR_AROUND,
		DIR_UP,
		DIR_DOWN,
	};
	
	dodBYTE M_PROM1[5];
	dodBYTE M_CURS[3];
	dodBYTE M_ERAS[6];
	dodBYTE CERR[3];

private:
};

#endif // DOD_PARSER_HEADER
