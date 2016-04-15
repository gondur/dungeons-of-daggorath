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
// Filename: creature.h
//
// This class manages the creature data and movement

#ifndef DOD_CREATURE_HEADER
#define DOD_CREATURE_HEADER

#include "dod.h"

class Creature
{
public:
	// Constructor
	Creature();

	// Public Interface
	void		NEWLVL();
	int			CREGEN();
	int			CMOVE(int task, int cidx);
	bool		CWALK(dodBYTE dir, CCB * cr);
	bool		CFIND(dodBYTE rw, dodBYTE cl);
	int			CFIND2(RowCol rc);
	void		Reset();
	void		LoadSounds();
	void		UpdateCreSpeed();
	
	// Public Data Fields
	CCB			CCBLND[32];
	dodBYTE		FRZFLG;
	int			CMXPTR;
	dodBYTE		CMXLND[60];
	dodBYTE		MOVTAB[7];
	Mix_Chunk * creSound[12];
	Mix_Chunk * clank;
	Mix_Chunk * kaboom;
	Mix_Chunk *	buzz;
	int			creChannel;
	int			creChannelv;
	int			creSpeedMul;

	enum { // creature ID#s
		CRT_SPIDER=0,
		CRT_VIPER=1,
		CRT_GIANT1=2,
		CRT_BLOB=3,
		CRT_KNIGHT1=4,
		CRT_GIANT2=5,
		CRT_SCORPION=6,
		CRT_KNIGHT2=7,
		CRT_WRAITH=8,
		CRT_GALDROG=9,
		CRT_WIZIMG=10,
		CRT_WIZARD=11,
	};

private:
	// Internal Implementation
	void CBIRTH(dodBYTE a);

	// Data Fields
	CDB			CDBTAB[12];
	
	// Constants
	enum {
		CTYPES=12,
	};
};

#endif // DOD_CREATURE_HEADER
