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
// Filename: dodgame.h
//
// This class is intended to be a controller class, but
// so much of the functionality is closely associated
// with the primary objects that there is not much going
// on here.  As the game grows, this may change.

#ifndef DOD_GAME_HEADER
#define DOD_GAME_HEADER

#include "dod.h"

class dodGame
{
public:
	// Constructor
	dodGame();

	// Public Interface
	void COMINI();	// Common initialization
	void INIVU();	// View initialization
	void Restart();
	void LoadGame();
	void WAIT();

	// Public Data Fields
	dodBYTE	LEVEL;	// Dungeon level (0-4)
	bool	IsDemo;
	bool	RandomMaze;
	bool	ShieldFix;
	bool	VisionScroll;
	bool	CreaturesIgnoreObjects;
	bool	CreaturesInstaRegen;
	bool	MarkDoorsOnScrollMaps;
	bool	AUTFLG;	// Autoplay (demo) flag
	bool	hasWon;
	bool	demoRestart;
	int		DEMOPTR;
	dodBYTE DEMO_CMDS[256];

private:
	// Data Fields
};

#endif // DOD_GAME_HEADER
