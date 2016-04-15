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
// Filename: sched.cpp
//
// Implementation of the Scheduler class

#include <iostream>
#include <fstream>

using namespace std;

#include "sched.h"
#include "player.h"
#include "viewer.h"
#include "oslink.h"
#include "creature.h"
#include "parser.h"
#include "dodgame.h"
#include "dungeon.h"
#include "object.h"

extern Object	object;
extern RNG		rng;
extern Dungeon	dungeon;
extern dodGame	game;
extern Parser	parser;
extern Creature	creature;
extern Player	player;
extern Viewer	viewer;
extern OS_Link	oslink;

// Constructor
Scheduler::Scheduler()
{
	Reset();
}

void Scheduler::Reset()
{
	curTime=0;
	elapsedTime=0;
	TCBPTR = 0;
	KBDHDR = 0;
	KBDTAL = 0;
	SLEEP = 0;
	NOISEF = 0;
	NOISEV = 0;
	ZFLAG = 0;
	hrtChannel=0;

	for (int ctr = 0; ctr < 38; ++ctr)
		TCBLND[ctr].clear();
}

// Public Interface
void Scheduler::LoadSounds()
{
	hrtSound[0] = Utils::LoadSound("17_heart.wav");
	hrtSound[1] = Utils::LoadSound("18_heart.wav");
}

// Creates initial Task Blocks
void Scheduler::SYSTCB()
{
	int	ctr;
	int	TCBindex;

	for (ctr = 0; ctr < 38; ++ctr)
	{
		TCBLND[ctr].clear();
	}
	
	TCBPTR = 0;

	TCBLND[0].type = TID_CLOCK;
	TCBLND[0].frequency = 17;		// One JIFFY
	TCBindex = GETTCB();

	TCBLND[1].type = TID_PLAYER;
	TCBLND[1].frequency = 17;		// One JIFFY
	TCBindex = GETTCB();

	TCBLND[2].type = TID_REFRESH_DISP;
	TCBLND[2].frequency = 300;		// Three TENTHs
	TCBindex = GETTCB();
	
	TCBLND[3].type = TID_HRTSLOW;
	TCBindex = GETTCB();
	
	TCBLND[4].type = TID_TORCHBURN;
	TCBLND[4].frequency = 5000;		// Five Seconds
	TCBindex = GETTCB();
	
	TCBLND[5].type = TID_CRTREGEN;
	TCBLND[5].frequency = 300000;	// Five minutes
	TCBindex = GETTCB();
}

// This is the Main Loop of the game.  Originally it was
// a port of the scheduling algorithm used in the source
// code, but it has been entirely replaced with a simpler
// algorithm that works better on modern platforms.
//
// It uses milliseconds instead of JIFFYs.  And it does
// not have any queues, but a simple array of Task objects.
void Scheduler::SCHED()
{
	// Initialization
	int	result = 0;  // not currently being used
	int	ctr = 0;

	// Main game execution loop
	do
	{
		curTime = SDL_GetTicks();

		if (curTime >= TCBLND[ctr].next_time)
		{
			switch (TCBLND[ctr].type)
			{
			case TID_CLOCK:
				CLOCK();
				break;
			case TID_PLAYER:
				result = player.PLAYER();
				break;
			case TID_REFRESH_DISP:
				result = viewer.LUKNEW();
				break;
			case TID_HRTSLOW:
				result = player.HSLOW();
				break;
			case TID_TORCHBURN:
				result = player.BURNER();
				break;
			case TID_CRTREGEN:
				result = creature.CREGEN();
				break;
			case TID_CRTMOVE:
				result = creature.CMOVE(ctr, TCBLND[ctr].data);
				break;
			default:
				// error
				break;
			}
		}

		(ctr < TCBPTR) ? ++ctr : ctr = 0;

		if (ZFLAG != 0) // Saving or Loading
		{
			if (ZFLAG == 0xFF)
			{
				return; // Load game abandons current game
			}
			else
			{
				SAVE();
				ZFLAG = 0;
			}
		}

		if (player.PLRBLK.P_ATPOW < player.PLRBLK.P_ATDAM)
		{
			return; // Death
		}

		if (game.hasWon)
		{
			return; // Victory
		}

	} while (result == 0);
}

// This is the heart of the game, literally.  It manages
// the heartbeat, calls for the screen to be redrawn, and
// polls the OS for key strokes.
//
// This routine is called every 17 milliseconds from the
// scheduler, and also from the blocking loops after each
// sound, which allows the heartbeat to mix in with the
// other sounds.
void Scheduler::CLOCK()
{
	// Update Task's next_time
	TCBLND[0].next_time = curTime +
		TCBLND[0].frequency;

	// Update elaplsed time
	elapsedTime = curTime - TCBLND[0].prev_time;
	
	// Reality check
	if (elapsedTime > 126 * 17)
	{
		elapsedTime = 126 * 17;
	}

	if (elapsedTime >= 17)
	{
		// Update Task's prev_time
		TCBLND[0].prev_time = curTime;	
		if (player.HBEATF != 0)
		{
			player.HEARTC -= (elapsedTime / 17);
			if ((player.HEARTC & 0x80) != 0)
			{
				player.HEARTC = 0;
			}
			if (player.HEARTC == 0)
			{
				player.HEARTC = player.HEARTR;
				
				// make sound
				Mix_PlayChannel(hrtChannel, hrtSound[(dodBYTE) (player.HEARTS + 1)], 0);
				while (Mix_Playing(hrtChannel) == 1) ; // !!!

				if (player.HEARTF != 0)
				{
					if ((player.HEARTS & 0x80) != 0)
					{
						// small
						viewer.statArea[15] = '<';
						viewer.statArea[16] = '>';
						player.HEARTS = 0;
					}
					else
					{
						// large
						viewer.statArea[15] = '{';
						viewer.statArea[16] = '}';
						player.HEARTS = -1;
					}
					if (!player.turning)
					{
						--viewer.UPDATE;
						viewer.draw_game();
					}
				}
			}
		}
	}

	if (player.FAINT == 0)
	{
		if (game.AUTFLG)
		{
			// Abort demo on keypress
			if (keyCheck())
			{
				game.hasWon = true;
				game.demoRestart = false;
			}
		}
		else
		{
			// Perform Keyboard Input
			oslink.process_events();
		}
	}
}

// Gets next available Task Block and updates the index
int Scheduler::GETTCB()
{
	++TCBPTR;
	return (TCBPTR - 1);
}

// All the following methods should really be moved to the
// OS_Link class.

// Used by wizard fade in/out function
bool Scheduler::fadeLoop()
{
	SDL_Event event;
	viewer.displayCopyright();
	viewer.displayWelcomeMessage();

	// Start buzz
	Mix_Volume(viewer.fadChannel, 0);
	Mix_PlayChannel(viewer.fadChannel, creature.buzz, -1);
	
	while(true)
	{
		if ( keyCheck() )
		{
			viewer.clearArea(&viewer.TXTPRI);
			while(SDL_PollEvent(&event))
			{
				; // clear event buffer
			}

			// Stop buzz
			Mix_HaltChannel(viewer.fadChannel);

			return false;	// auto-play mode off == start demo game
		}
		if ( viewer.draw_fade() )
		{
			// Stop buzz
			Mix_HaltChannel(viewer.fadChannel);

			return true;	// auto-play mode on == start regular game
		}
	}
}

void Scheduler::deathFadeLoop()
{
	SDL_Event event;
	viewer.displayDeath();
	viewer.fadeVal = -2;
	viewer.VXSCAL = 0x80;
	viewer.VYSCAL = 0x80;
	viewer.VXSCALf = 128.0f;
	viewer.VYSCALf = 128.0f;
	viewer.delay = 0;
	viewer.VCTFAD = 32;
	viewer.done = false;

	while(SDL_PollEvent(&event))
	{
		; // clear event buffer
	}

	// Start buzz
	Mix_Volume(viewer.fadChannel, 0);
	Mix_PlayChannel(viewer.fadChannel, creature.buzz, -1);
	
	while (!viewer.done)
	{
		viewer.death_fade(viewer.W1_VLA);
		EscCheck();
	}

	// Stop buzz
	Mix_HaltChannel(viewer.fadChannel);

	while(SDL_PollEvent(&event))
	{
		; // clear event buffer
	}

	while(true)
	{
		viewer.death_fade(viewer.W1_VLA);
		if ( keyCheck() )
		{
			viewer.clearArea(&viewer.TXTPRI);
			while(SDL_PollEvent(&event))
			{
				; // clear event buffer
			}
			return;
		}
	}
}

void Scheduler::winFadeLoop()
{
	SDL_Event event;
	bool loopDone = false;
	viewer.displayWinner();
	viewer.fadeVal = -2;
	viewer.VXSCAL = 0x80;
	viewer.VYSCAL = 0x80;
	viewer.VXSCALf = 128.0f;
	viewer.VYSCALf = 128.0f;
	viewer.delay = 0;
	viewer.VCTFAD = 32;
	viewer.done = false;

	while(SDL_PollEvent(&event))
	{
		; // clear event buffer
	}

	// Start buzz
	Mix_Volume(viewer.fadChannel, 0);
	Mix_PlayChannel(viewer.fadChannel, creature.buzz, -1);
	
	while (!viewer.done)
	{
		viewer.death_fade(viewer.W2_VLA);
		EscCheck();
	}

	// Stop buzz
	Mix_HaltChannel(viewer.fadChannel);

	while(true)
	{
		viewer.death_fade(viewer.W2_VLA);
		if ( keyCheck() )
		{
			viewer.clearArea(&viewer.TXTPRI);
			while(SDL_PollEvent(&event))
			{
				; // clear event buffer
			}
			return;
		}
	}

	while(SDL_PollEvent(&event))
	{
		; // clear event buffer
	}
}

// Used by wizard fade in/out function
bool Scheduler::keyCheck()
{
	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
		case SDL_KEYDOWN:
			return ( keyHandler(&event.key.keysym) );
			break;
		case SDL_QUIT:
			oslink.quitSDL(0); // eventually change to meta-menu
			break;
		case SDL_VIDEOEXPOSE:
			SDL_GL_SwapBuffers();
			break;
		}
	}
	return false;
}

// Used by wizard fade in/out function
bool Scheduler::keyHandler(SDL_keysym * keysym)
{
	bool rc;

	switch(keysym->sym)
	{
	case SDLK_ESCAPE:
		Mix_HaltChannel(viewer.fadChannel);

		rc = oslink.main_menu();  // calls the meta-menu

		Mix_Volume(viewer.fadChannel, 0);
		Mix_PlayChannel(viewer.fadChannel, creature.buzz, -1);
		return rc;
	default:
		return true;
	}
}

// Used during fainting and intermission
bool Scheduler::EscCheck()
{
	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
		case SDL_KEYDOWN:
			return ( keyHandler(&event.key.keysym) );
			break;
		case SDL_QUIT:
			oslink.quitSDL(0); // eventually change to meta-menu
			break;
		case SDL_VIDEOEXPOSE:
			SDL_GL_SwapBuffers();
			break;
		}
	}
	return false;
}

// Used by wizard fade in/out function
bool Scheduler::EscHandler(SDL_keysym * keysym)
{
	bool rc;

	switch(keysym->sym)
	{
	case SDLK_ESCAPE:
		Mix_HaltChannel(viewer.fadChannel);

		rc = oslink.main_menu();  // Calls the meta-menu

		Mix_Volume(viewer.fadChannel, 0);
		Mix_PlayChannel(viewer.fadChannel, creature.buzz, -1);
		return (!rc);
	default:
		return false;
	}
}

void Scheduler::SAVE()
{
	ofstream	fout;
	int			ctr;
	char 		outstr[64];

	fout.open(oslink.gamefile, ios::trunc);
	if (!fout)
	{
		// DISK ERROR
		viewer.OUTSTR(DERR);
		viewer.PROMPT();
		return;
	}

	sprintf(outstr, "%d", game.LEVEL); fout << outstr << endl;
	sprintf(outstr, "%d", dungeon.VFTPTR); fout << outstr << endl;

	for (ctr = 0; ctr < 1024; ++ctr)
	{
		sprintf(outstr, "%d", dungeon.MAZLND[ctr]); fout << outstr << endl;
	}

	sprintf(outstr, "%d", player.PROW); fout << outstr << endl;
	sprintf(outstr, "%d", player.PCOL); fout << outstr << endl;
	sprintf(outstr, "%d", player.POBJWT); fout << outstr << endl;
	sprintf(outstr, "%d", player.PPOW); fout << outstr << endl;
	sprintf(outstr, "%d", player.PLHAND); fout << outstr << endl;
	sprintf(outstr, "%d", player.PRHAND); fout << outstr << endl;
	sprintf(outstr, "%d", player.PDAM); fout << outstr << endl;
	sprintf(outstr, "%d", player.PDIR); fout << outstr << endl;
	sprintf(outstr, "%d", player.PTORCH); fout << outstr << endl;
	sprintf(outstr, "%d", player.PRLITE); fout << outstr << endl;
	sprintf(outstr, "%d", player.PMLITE); fout << outstr << endl;
	sprintf(outstr, "%d", player.FAINT); fout << outstr << endl;
	sprintf(outstr, "%d", player.BAGPTR); fout << outstr << endl;
	sprintf(outstr, "%d", player.HEARTF); fout << outstr << endl;
	sprintf(outstr, "%d", player.HEARTC); fout << outstr << endl;
	sprintf(outstr, "%d", player.HEARTR); fout << outstr << endl;
	sprintf(outstr, "%d", player.HEARTS); fout << outstr << endl;
	sprintf(outstr, "%d", player.HBEATF); fout << outstr << endl;
	sprintf(outstr, "%d", rng.SEED[0]); fout << outstr << endl;
	sprintf(outstr, "%d", rng.SEED[1]); fout << outstr << endl;
	sprintf(outstr, "%d", rng.SEED[2]); fout << outstr << endl;
	sprintf(outstr, "%d", rng.carry); fout << outstr << endl;
	sprintf(outstr, "%d", creature.FRZFLG); fout << outstr << endl;
	sprintf(outstr, "%d", creature.CMXPTR); fout << outstr << endl;

	for (ctr = 0; ctr < 60; ++ctr)
	{
		sprintf(outstr, "%d", creature.CMXLND[ctr]); fout << outstr << endl;
	}

	for (ctr = 0; ctr < 32; ++ctr)
	{
		sprintf(outstr, "%d", creature.CCBLND[ctr].P_CCPOW); fout << outstr << endl;
		sprintf(outstr, "%d", creature.CCBLND[ctr].P_CCMGO); fout << outstr << endl;
		sprintf(outstr, "%d", creature.CCBLND[ctr].P_CCMGD); fout << outstr << endl;
		sprintf(outstr, "%d", creature.CCBLND[ctr].P_CCPHO); fout << outstr << endl;
		sprintf(outstr, "%d", creature.CCBLND[ctr].P_CCPHD); fout << outstr << endl;
		sprintf(outstr, "%d", creature.CCBLND[ctr].P_CCTMV); fout << outstr << endl;
		sprintf(outstr, "%d", creature.CCBLND[ctr].P_CCTAT); fout << outstr << endl;
		sprintf(outstr, "%d", creature.CCBLND[ctr].P_CCOBJ); fout << outstr << endl;
		sprintf(outstr, "%d", creature.CCBLND[ctr].P_CCDAM); fout << outstr << endl;
		sprintf(outstr, "%d", creature.CCBLND[ctr].P_CCUSE); fout << outstr << endl;
		sprintf(outstr, "%d", creature.CCBLND[ctr].creature_id); fout << outstr << endl;
		sprintf(outstr, "%d", creature.CCBLND[ctr].P_CCDIR); fout << outstr << endl;
		sprintf(outstr, "%d", creature.CCBLND[ctr].P_CCROW); fout << outstr << endl;
		sprintf(outstr, "%d", creature.CCBLND[ctr].P_CCCOL); fout << outstr << endl;
	}

	sprintf(outstr, "%d", object.OFINDF); fout << outstr << endl;
	sprintf(outstr, "%d", object.OCBPTR); fout << outstr << endl;
	sprintf(outstr, "%d", object.OFINDP); fout << outstr << endl;

	for (ctr = 0; ctr < 72; ++ctr)
	{
		sprintf(outstr, "%d", object.OCBLND[ctr].P_OCPTR); fout << outstr << endl;
		sprintf(outstr, "%d", object.OCBLND[ctr].P_OCROW); fout << outstr << endl;
		sprintf(outstr, "%d", object.OCBLND[ctr].P_OCCOL); fout << outstr << endl;
		sprintf(outstr, "%d", object.OCBLND[ctr].P_OCLVL); fout << outstr << endl;
		sprintf(outstr, "%d", object.OCBLND[ctr].P_OCOWN); fout << outstr << endl;
		sprintf(outstr, "%d", object.OCBLND[ctr].P_OCXX0); fout << outstr << endl;
		sprintf(outstr, "%d", object.OCBLND[ctr].P_OCXX1); fout << outstr << endl;
		sprintf(outstr, "%d", object.OCBLND[ctr].P_OCXX2); fout << outstr << endl;
		sprintf(outstr, "%d", object.OCBLND[ctr].obj_id); fout << outstr << endl;
		sprintf(outstr, "%d", object.OCBLND[ctr].obj_type); fout << outstr << endl;
		sprintf(outstr, "%d", object.OCBLND[ctr].obj_reveal_lvl); fout << outstr << endl;
		sprintf(outstr, "%d", object.OCBLND[ctr].P_OCMGO); fout << outstr << endl;
		sprintf(outstr, "%d", object.OCBLND[ctr].P_OCPHO); fout << outstr << endl;
	}
	
	sprintf(outstr, "%d", viewer.RLIGHT); fout << outstr << endl;
	sprintf(outstr, "%d", viewer.MLIGHT); fout << outstr << endl;
	sprintf(outstr, "%d", viewer.OLIGHT); fout << outstr << endl;
	sprintf(outstr, "%d", viewer.VXSCAL); fout << outstr << endl;
	sprintf(outstr, "%d", viewer.VYSCAL); fout << outstr << endl;
	sprintf(outstr, "%d", viewer.TXBFLG); fout << outstr << endl;
	sprintf(outstr, "%d", viewer.tcaret); fout << outstr << endl;
	sprintf(outstr, "%d", viewer.tlen); fout << outstr << endl;
	sprintf(outstr, "%d", viewer.NEWLIN); fout << outstr << endl;

	//Save current game levels' rnd seeds, as they are no longer hard-coded.
	for (ctr = 0; ctr <= 6; ++ctr)
	{
		sprintf(outstr, "%d", dungeon.LEVTAB[ctr]); fout << outstr << endl;
	}
	//Save vertical features table, as they are no longer hard-coded.

	for (ctr = 0; ctr <= 41; ++ctr)
	{
		sprintf(outstr, "%d", dungeon.VFTTAB[ctr]); fout << outstr << endl;
	}
	sprintf(outstr, "%d", game.RandomMaze); fout << outstr << endl;
	sprintf(outstr, "%d", game.ShieldFix); fout << outstr << endl;
	sprintf(outstr, "%d", game.VisionScroll); fout << outstr << endl;
	sprintf(outstr, "%d", game.CreaturesIgnoreObjects); fout << outstr << endl;
	sprintf(outstr, "%d", game.CreaturesInstaRegen); fout << outstr << endl;
	sprintf(outstr, "%d", game.MarkDoorsOnScrollMaps); fout << outstr << endl;
		
	fout.close();
}

void Scheduler::LOAD()
{
	ifstream	fin;
	int			ctr, in;
	char 		instr[64];

	fin.open(oslink.gamefile);
	if (!fin)
	{
		// DISK ERROR
		viewer.OUTSTR(DERR);
		viewer.PROMPT();
		return;
	}

	fin >> instr; if (1==sscanf(instr, "%d", &in)) game.LEVEL = in;
	fin >> instr; if (1==sscanf(instr, "%d", &in)) dungeon.VFTPTR = in;

	for (ctr = 0; ctr < 1024; ++ctr)
	{
		fin >> instr; if (1==sscanf(instr, "%d", &in)) dungeon.MAZLND[ctr] = in;
	}

	fin >> instr; if (1==sscanf(instr,"%d", &in)) player.PROW = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) player.PCOL = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) player.POBJWT = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) player.PPOW = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) player.PLHAND = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) player.PRHAND = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) player.PDAM = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) player.PDIR = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) player.PTORCH = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) player.PRLITE = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) player.PMLITE = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) player.FAINT = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) player.BAGPTR = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) player.HEARTF = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) player.HEARTC = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) player.HEARTR = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) player.HEARTS = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) player.HBEATF = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) rng.SEED[0] = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) rng.SEED[1] = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) rng.SEED[2] = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) rng.carry = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) creature.FRZFLG = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) creature.CMXPTR = in;

	for (ctr = 0; ctr < 60; ++ctr)
	{
		fin >> instr; if (1==sscanf(instr,"%d", &in)) creature.CMXLND[ctr] = in;
	}

	for (ctr = 0; ctr < 32; ++ctr)
	{
		fin >> instr; if (1==sscanf(instr,"%d", &in)) creature.CCBLND[ctr].P_CCPOW = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) creature.CCBLND[ctr].P_CCMGO = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) creature.CCBLND[ctr].P_CCMGD = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) creature.CCBLND[ctr].P_CCPHO = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) creature.CCBLND[ctr].P_CCPHD = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) creature.CCBLND[ctr].P_CCTMV = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) creature.CCBLND[ctr].P_CCTAT = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) creature.CCBLND[ctr].P_CCOBJ = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) creature.CCBLND[ctr].P_CCDAM = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) creature.CCBLND[ctr].P_CCUSE = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) creature.CCBLND[ctr].creature_id = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) creature.CCBLND[ctr].P_CCDIR = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) creature.CCBLND[ctr].P_CCROW = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) creature.CCBLND[ctr].P_CCCOL = in;
	}

	fin >> instr; if (1==sscanf(instr,"%d", &in)) object.OFINDF = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) object.OCBPTR = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) object.OFINDP = in;

	for (ctr = 0; ctr < 72; ++ctr)
	{
		fin >> instr; if (1==sscanf(instr,"%d", &in)) object.OCBLND[ctr].P_OCPTR = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) object.OCBLND[ctr].P_OCROW = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) object.OCBLND[ctr].P_OCCOL = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) object.OCBLND[ctr].P_OCLVL = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) object.OCBLND[ctr].P_OCOWN = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) object.OCBLND[ctr].P_OCXX0 = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) object.OCBLND[ctr].P_OCXX1 = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) object.OCBLND[ctr].P_OCXX2 = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) object.OCBLND[ctr].obj_id = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) object.OCBLND[ctr].obj_type = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) object.OCBLND[ctr].obj_reveal_lvl = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) object.OCBLND[ctr].P_OCMGO = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) object.OCBLND[ctr].P_OCPHO = in;
	}

	fin >> instr; if (1==sscanf(instr,"%d", &in)) viewer.RLIGHT = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) viewer.MLIGHT = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) viewer.OLIGHT = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) viewer.VXSCAL = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) viewer.VYSCAL = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) viewer.TXBFLG = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) viewer.tcaret = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) viewer.tlen = in;
	fin >> instr; if (1==sscanf(instr,"%d", &in)) viewer.NEWLIN = in;
	//Original save games ended here.

	fin >> instr; if (1==sscanf(instr,"%d", &in))  //Do we have more data to load?  Yes:
	{
		//Load current game levels' rnd seeds, as they are no longer hard-coded.
		dungeon.LEVTAB[0] = in;
		for (ctr = 1; ctr <= 6; ++ctr)
		{
			fin >> instr; if (1==sscanf(instr,"%d", &in)) dungeon.LEVTAB[ctr] = in;
		}

		//Load vertical features table, as they are no longer hard-coded.
		for (ctr = 0; ctr <= 41; ++ctr)
		{
			fin >> instr; if (1==sscanf(instr,"%d", &in)) dungeon.VFTTAB[ctr] = in;
		}
		fin >> instr; if (1==sscanf(instr,"%d", &in)) game.RandomMaze = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) game.ShieldFix = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) game.VisionScroll = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) game.CreaturesIgnoreObjects = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) game.CreaturesInstaRegen = in;
		fin >> instr; if (1==sscanf(instr,"%d", &in)) game.MarkDoorsOnScrollMaps = in;
	} else {  //Do we have more data to load?  No:
		//Old save game.  Must be old save with original map.
		//Put in original rnd seeds & vertical features table.
		dungeon.LEVTAB[0] = 0x73;
		dungeon.LEVTAB[1] = 0xC7;
		dungeon.LEVTAB[2] = 0x5D;
		dungeon.LEVTAB[3] = 0x97;
		dungeon.LEVTAB[4] = 0xF3;
		dungeon.LEVTAB[5] = 0x13;
		dungeon.LEVTAB[6] = 0x87;
		dungeon.VFTTAB[0] = -1;
		dungeon.VFTTAB[1] = 1;
		dungeon.VFTTAB[2] = 0;
		dungeon.VFTTAB[3] = 23;
		dungeon.VFTTAB[4] = 0;
		dungeon.VFTTAB[5] = 15;
		dungeon.VFTTAB[6] = 4;
		dungeon.VFTTAB[7] = 0;
		dungeon.VFTTAB[8] = 20;
		dungeon.VFTTAB[9] = 17;
		dungeon.VFTTAB[10] = 1;
		dungeon.VFTTAB[11] = 28;
		dungeon.VFTTAB[12] = 30;
		dungeon.VFTTAB[13] = -1;
		dungeon.VFTTAB[14] = 1;
		dungeon.VFTTAB[15] = 2;
		dungeon.VFTTAB[16] = 3;
		dungeon.VFTTAB[17] = 0;
		dungeon.VFTTAB[18] = 3;
		dungeon.VFTTAB[19] = 31;
		dungeon.VFTTAB[20] = 0;
		dungeon.VFTTAB[21] = 19;
		dungeon.VFTTAB[22] = 20;
		dungeon.VFTTAB[23] = 0;
		dungeon.VFTTAB[24] = 31;
		dungeon.VFTTAB[25] = 0;
		dungeon.VFTTAB[26] = -1;
		dungeon.VFTTAB[27] = -1;
		dungeon.VFTTAB[28] = 0;
		dungeon.VFTTAB[29] = 0;
		dungeon.VFTTAB[30] = 31;
		dungeon.VFTTAB[31] = 0;
		dungeon.VFTTAB[32] = 5;
		dungeon.VFTTAB[33] = 0;
		dungeon.VFTTAB[34] = 0;
		dungeon.VFTTAB[35] = 22;
		dungeon.VFTTAB[36] = 28;
		dungeon.VFTTAB[37] = 0;
		dungeon.VFTTAB[38] = 31;
		dungeon.VFTTAB[39] = 16;
		dungeon.VFTTAB[40] = -1;
		dungeon.VFTTAB[41] = -1;
		game.RandomMaze = false;
		game.ShieldFix = false;
		game.VisionScroll = false;
		game.CreaturesIgnoreObjects = false;
		game.CreaturesInstaRegen = false;
		game.MarkDoorsOnScrollMaps = false;
	}  //Do we have more data to load?

	fin.close();

}

/***********************************************************************
  Member:  pause

  Vars:    state - Used to toggle current pause state
                 - true - pause the game, false - unpause the game

  Note:    Only saves state of TCBs, no actual sidestepping of execution
***********************************************************************/
void Scheduler::pause(bool state)
{
  // The current pause state of the game
 static bool   curState = false;
 static Uint32 savedTime;

 if(!curState && state)
   {
   savedTime = curTime;
   }
 else if(curState && !state)
   {
   curTime = SDL_GetTicks();

   for(int i=0; i < TCBPTR; i++)
     {
     TCBLND[i].next_time += (curTime - savedTime);
     TCBLND[i].prev_time += (curTime - savedTime);
     }
   }
 return;
}

/***********************************************************************
  Member:  updateCreatureRegen

  Vars:    newTime - The new creature regen time (in minutes)

  Note:    This takes effect after the next creature regen
***********************************************************************/
void Scheduler::updateCreatureRegen(int newTime)
{
 TCBLND[5].frequency = 60000 * newTime;
}
