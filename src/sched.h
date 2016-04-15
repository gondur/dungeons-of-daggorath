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
// Filename: sched.h
//
// This class manages the scheduler.

#ifndef DOD_SCHEDULER_HEADER
#define DOD_SCHEDULER_HEADER

#include "dod.h"

class Scheduler
{
public:
	// Constructor
	Scheduler();

	// Scaffolding
	//void		printTrace(char * st, int rout);
	//void		printClock();

	// Public Interface
	void		SYSTCB();
	void		SCHED();
	void		CLOCK();
	int			GETTCB();
	bool		fadeLoop();
	void		deathFadeLoop();
	void		winFadeLoop();
	bool		keyCheck();
	bool		keyHandler(SDL_keysym * keysym);
	void		Reset();
	void		SAVE();
	void		LOAD();
	void		LoadSounds();
	bool		EscCheck();
	bool		EscHandler(SDL_keysym * keysym);
	void		pause(bool state);
	void		updateCreatureRegen(int newTime);
	
	// Public Data Fields
	Task	TCBLND[38];

	dodBYTE DERR[15];

	enum { // task IDs
		TID_CLOCK		= 0,
		TID_PLAYER		= 1,
		TID_REFRESH_DISP = 2,
		TID_HRTSLOW		= 3,
		TID_TORCHBURN	= 4,
		TID_CRTREGEN	= 5,
		TID_CRTMOVE		= 6,
	};


	Uint32		curTime;
	Uint32		elapsedTime;

	Mix_Chunk * hrtSound[2];
	int			hrtChannel;

	dodBYTE		ZFLAG;

private:
	// Data Fields
	int			TCBPTR;
	dodBYTE		KBDHDR;
	dodBYTE		KBDTAL;

	dodBYTE		SLEEP;
	dodBYTE		NOISEF;
	dodBYTE		NOISEV;
};

#endif // DOD_SCHEDULER_HEADER
