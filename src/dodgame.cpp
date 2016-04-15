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
// Filename: dodgame.cpp
//
// Implementation of dodGame class.

//#include <stdio.h>

#include "dodgame.h"
#include "player.h"
#include "object.h"
#include "viewer.h"
#include "sched.h"
#include "creature.h"
#include "parser.h"
#include "dungeon.h"
#include "oslink.h"

extern OS_Link		oslink;
extern Dungeon		dungeon;
extern Parser		parser;
extern Creature		creature;
extern Object		object;
extern Player		player;
extern Viewer		viewer;
extern Scheduler	scheduler;

#define D_EXAMINE		"010f"
#define D_PULL_RIGHT	"032605"
#define D_TORCH			"19"
#define D_USE_RIGHT		"023705"
#define D_LOOK			"011e"
#define D_MOVE			"0122"
#define D_PULL_LEFT		"032601"
#define D_SHIELD		"0f"
#define D_SWORD			"14"
#define D_ATTACK_RIGHT	"020105"
#define D_TURN_RIGHT	"023305"
#define D_END			"00"

// Constructor
dodGame::dodGame() : LEVEL(2), AUTFLG(true), hasWon(false),
					 DEMOPTR(0), demoRestart(true)
{
	Utils::LoadFromHex(DEMO_CMDS,
//		D_LOOK
		D_EXAMINE
		D_PULL_RIGHT D_TORCH
		D_USE_RIGHT
		D_LOOK
		D_MOVE
		D_PULL_LEFT  D_SHIELD
		D_PULL_RIGHT D_SWORD
		D_MOVE D_MOVE
		D_ATTACK_RIGHT
		D_TURN_RIGHT D_MOVE D_MOVE D_MOVE
		D_TURN_RIGHT D_MOVE D_MOVE
		D_END
		);

/*	DEMO_CMDS[0] = 1;
	DEMO_CMDS[1] = 15;		// EXAMINE
	DEMO_CMDS[2] = 3;
	DEMO_CMDS[3] = 38;		// PULL
	DEMO_CMDS[4] = 5;		// RIGHT
	DEMO_CMDS[5] = 25;		// TORCH
	DEMO_CMDS[6] = 2;
	DEMO_CMDS[7] = 55;		// USE
	DEMO_CMDS[8] = 5;		// RIGHT
	DEMO_CMDS[9] = 1;
	DEMO_CMDS[10] = 30;	// LOOK
	DEMO_CMDS[11] = 1;
	DEMO_CMDS[12] = 34;	// MOVE
	DEMO_CMDS[13] = 3;
	DEMO_CMDS[14] = 38;	// PULL
	DEMO_CMDS[15] = 1;		// LEFT
	DEMO_CMDS[16] = 15;	// SHIELD

	DEMO_CMDS[17] = 3;
	DEMO_CMDS[18] = 38;	// PULL
	DEMO_CMDS[19] = 5;		// RIGHT
	DEMO_CMDS[20] = 20;	// SWORD
	DEMO_CMDS[21] = 1;
	DEMO_CMDS[22] = 34;	// MOVE
	DEMO_CMDS[23] = 1;
	DEMO_CMDS[24] = 34;	// MOVE
	DEMO_CMDS[25] = 2;
	DEMO_CMDS[26] = 1;		// ATTACK
	DEMO_CMDS[27] = 5;		// RIGHT

	DEMO_CMDS[28] = 2;
	DEMO_CMDS[29] = 51;	// TURN
	DEMO_CMDS[30] = 5;		// RIGHT
	DEMO_CMDS[31] = 1;
	DEMO_CMDS[32] = 34;	// MOVE
	DEMO_CMDS[33] = 1;
	DEMO_CMDS[34] = 34;	// MOVE
	DEMO_CMDS[35] = 1;
	DEMO_CMDS[36] = 34;	// MOVE
	DEMO_CMDS[37] = 2;
	DEMO_CMDS[38] = 51;	// TURN
	DEMO_CMDS[39] = 5;		// RIGHT
	DEMO_CMDS[40] = 1;
	DEMO_CMDS[41] = 34;	// MOVE
	DEMO_CMDS[42] = 1;
	DEMO_CMDS[43] = 34;	// MOVE
	DEMO_CMDS[44] = -1;*/
}

// Game initialization
void dodGame::COMINI()
{
	Uint32 ticks1, ticks2;

	scheduler.SYSTCB();
	object.CreateAll();
	player.HBEATF = 0;
	viewer.clearArea(&viewer.TXTSTS);
	viewer.clearArea(&viewer.TXTPRI);
	viewer.VXSCAL = 0x80;
	viewer.VYSCAL = 0x80;
	viewer.VXSCALf = 128.0f;
	viewer.VYSCALf = 128.0f;
	AUTFLG = viewer.ShowFade(Viewer::FADE_BEGIN);
	//AUTFLG = scheduler.fadeLoop();
	//AUTFLG = false; // TAKE THIS LINE OUT !!!!!!!!!! [Prevents demo from starting]
	player.setInitialObjects(AUTFLG);
	viewer.displayPrepare();
	viewer.display_mode = Viewer::MODE_TITLE;
	viewer.draw_game();

	// Delay with "PREPARE!" on screen
	ticks1 = SDL_GetTicks();
	do
	{
		oslink.process_events();
		ticks2 = SDL_GetTicks();
	} while (ticks2 < ticks1 + viewer.prepPause);
	
	creature.NEWLVL();
	if (AUTFLG)
	{
		// do map
		viewer.display_mode = Viewer::MODE_TITLE;
		viewer.showSeerMap = true;
		--viewer.UPDATE;
		viewer.draw_game();
		// wait 3 seconds
		ticks1 = SDL_GetTicks();
		do
		{
			oslink.process_events();
			ticks2 = SDL_GetTicks();
		} while (ticks2 < ticks1 + 3000);
	}
	INIVU();
	viewer.PROMPT();
}

void dodGame::Restart()
{
	Uint32 ticks1, ticks2;

	object.Reset();
	creature.Reset();
	parser.Reset();
	player.Reset();
	scheduler.Reset();
	viewer.Reset();
	hasWon = false;

	dungeon.VFTPTR = 0;
	scheduler.SYSTCB();
	object.CreateAll();
	player.HBEATF = 0;
	player.setInitialObjects(false);
	viewer.displayPrepare();
	viewer.displayCopyright();
	viewer.display_mode = Viewer::MODE_TITLE;
	viewer.draw_game();

	// Delay with "PREPARE!" on screen
	ticks1 = SDL_GetTicks();
	do
	{
		oslink.process_events();
		ticks2 = SDL_GetTicks();
	} while (ticks2 < ticks1 + 2500);
	
	creature.NEWLVL();
	INIVU();
	viewer.PROMPT();
}

void dodGame::LoadGame()
{
	scheduler.LOAD();
	viewer.setVidInv((game.LEVEL % 2) ?true: false);
	--viewer.UPDATE;
	viewer.draw_game();
	INIVU();
	viewer.PROMPT();
}

// Initializes 3D viewer
void dodGame::INIVU()
{
	viewer.clearArea(&viewer.TXTSTS);
	viewer.clearArea(&viewer.TXTPRI);
	player.HUPDAT();
	++player.HEARTC;
	--player.HEARTF;
	--player.HBEATF;
	viewer.STATUS();
	player.PLOOK();
}

// Pause 1.5 seconds
void dodGame::WAIT()
{
	Uint32 ticks1;
	ticks1 = SDL_GetTicks();
	scheduler.curTime = ticks1;

	do
	{
		if (scheduler.curTime >= scheduler.TCBLND[0].next_time)
		{
			scheduler.CLOCK();
			if (game.AUTFLG && game.demoRestart == false)
			{
				return;
			}
			scheduler.EscCheck();
		}
		scheduler.curTime = SDL_GetTicks();
	} while (scheduler.curTime < ticks1 + 1500);
}
