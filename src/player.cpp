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
// Filename: player.cpp
//
// Implementation of Player class

#include "player.h"
#include "dodgame.h"
#include "viewer.h"
#include "sched.h"
#include "parser.h"
#include "object.h"
#include "dungeon.h"
#include "creature.h"
#include "oslink.h"
#include "enhanced.h"


extern OS_Link		oslink;
extern Creature		creature;
extern Object		object;
extern Viewer		viewer;
extern dodGame		game;
extern Scheduler	scheduler;
extern Parser		parser;
extern Dungeon		dungeon;

// Constructor
Player::Player() : PPOW(PLRBLK.P_ATPOW),
				   PMGO(PLRBLK.P_ATMGO),
				   PMGD(PLRBLK.P_ATMGD),
				   PPHO(PLRBLK.P_ATPHO),
				   PPHD(PLRBLK.P_ATPHD),
				   PDAM(PLRBLK.P_ATDAM),
				   turnDelay(20),
				   moveDelay(25),
				   wizDelay(500)
{
	Reset();
}

void Player::Reset()
{
	PROW = 12;
	PCOL = 22;
	PPOW = ((0x17 << 8) | 160);
	POBJWT = 35;
	FAINT = 0;
	PRLITE = 0;
	PMLITE = 0;
	PLHAND = -1;
	PRHAND = -1;
	PTORCH = -1;
	BAGPTR = -1;
	EMPHND.obj_type = Object::OBJT_WEAPON;
	EMPHND.obj_reveal_lvl = 0;
	EMPHND.P_OCMGO = 0;
	EMPHND.P_OCPHO = 5;
	PDAM = 0;
	PDIR = 0;
	HEARTF = 0;
	HEARTC = 0;
	HEARTR = 0;
	HEARTS = 0;
	HBEATF = 0;
	turning = false;
}

// Public Interface

void Player::LoadSounds()
{
	klink = Utils::LoadSound("12_klink.wav");
	thud = Utils::LoadSound("14_thud.wav");
	bang = Utils::LoadSound("15_bang.wav");
}

// This method gets called from the scheduler as often
// as possible.  It retrieves keyboard input, or commands
// from the demo data.
int Player::PLAYER()
{
	int tokCnt, tokCtr;
	dodBYTE objstr[10];
	dodBYTE * X, * U;
	int Xup;
	
	// Update Task's next_time
	scheduler.TCBLND[Scheduler::TID_PLAYER].next_time = scheduler.curTime +
		scheduler.TCBLND[Scheduler::TID_PLAYER].frequency;
	
	dodBYTE c;
	if (game.AUTFLG == 0)
	{
		// Process Keyboard Buffer
		do
		{
			c = parser.KBDGET();
			if (c == 0)
			{
				return 0;
			}
			if (FAINT != 0)
			{
				while (parser.KBDGET() != 0)
					;	// loop !!!
				return 0;
			}

			// Convert from ASCII to Internal Codes
			if (c >= 'A' && c <= 'Z')
			{
				c &= 0x1F;
			}
			else if (c == parser.C_BS)
			{
				c = parser.I_BS;
			}
			else if (c == parser.C_CR)
			{
				c = parser.I_CR;
			}
			else
			{
				c = parser.I_SP;
			}

			if(!HUMAN(c))
				return -1;
		} while (true);
	}
	else
	{
		// Process Autoplay Commands
		tokCnt = game.DEMO_CMDS[game.DEMOPTR++];
		if (tokCnt == 0)
		{
			game.WAIT();
			game.WAIT();
			game.hasWon = true;
			game.demoRestart = true;
			return 0;
		}
		
		// Feed next autoplay command to HUMAN
		tokCtr = 1;
		
		do
		{
			if (tokCtr == 1)
			{
				X = &parser.CMDTAB[game.DEMO_CMDS[game.DEMOPTR]];
			}
			else if (tokCtr == 2)
			{
				X = &parser.DIRTAB[game.DEMO_CMDS[game.DEMOPTR]];
			}
			else
			{
				X = &object.GENTAB[game.DEMO_CMDS[game.DEMOPTR]];
			}
			++game.DEMOPTR;
			U = &objstr[1];
			parser.EXPAND(X, &Xup, U);
			++U;
			game.WAIT();
			do
			{
				HUMAN(*U);
				++U;
			} while (*U != 0xFF);
			HUMAN(parser.I_SP);
			++tokCtr;
		} while (tokCtr <= tokCnt);
		--viewer.UPDATE;
		viewer.draw_game();
		HUMAN(parser.I_CR);
	}

	return 0;
}

bool Player::HUMAN(dodBYTE c)
{
	int		res;
	dodBYTE	A, B;
	
	// Check if we are displaying the map
	if (HEARTF == 0)
	{
		game.INIVU();
		viewer.PROMPT();
	}
	if (c == parser.I_CR)
	{
carriage_return:
		viewer.OUTCHR(parser.I_SP);
		parser.LINBUF[parser.LINPTR] = Parser::I_NULL;
		parser.LINBUF[parser.LINPTR+1] = Parser::I_NULL;
		parser.LINPTR = 0;

		if (!PreTranslateCommand(&parser.LINBUF[0]))
		{
			game.AUTFLG = true;
			game.demoRestart = true;
			return false;
		}

		// dispatch to proper routine
		res = parser.PARSER(&parser.CMDTAB[0], A, B, true);
		if (res == 1)
		{
			// dispatch
			switch (A)
			{
			case Parser::CMD_ATTACK:
				PATTK();
				break;
			case Parser::CMD_CLIMB:
				PCLIMB();
				break;
			case Parser::CMD_DROP:
				PDROP();
				break;
			case Parser::CMD_EXAMINE:
				PEXAM();
				break;
			case Parser::CMD_GET:
				PGET();
				break;
			case Parser::CMD_INCANT:
				PINCAN();
				break;
			case Parser::CMD_LOOK:
				PLOOK();
				break;
			case Parser::CMD_MOVE:
				PMOVE();
				break;
			case Parser::CMD_PULL:
				PPULL();
				break;
			case Parser::CMD_REVEAL:
				PREVEA();
				break;
			case Parser::CMD_STOW:
				PSTOW();
				break;
			case Parser::CMD_TURN:
				PTURN();
				break;
			case Parser::CMD_USE:
				PUSE();
				break;
			case Parser::CMD_ZLOAD:
				PZLOAD();
				break;
			case Parser::CMD_ZSAVE:
				PZSAVE();
				break;
			}
		}
		if (res == -1)
		{
			parser.CMDERR();
		}

		if ((HEARTF != 0) && (FAINT == 0))
		{
			viewer.PROMPT();
		}

		parser.LINPTR = 0;
		return true;
	}
	if (c == parser.I_BS)
	{
		if (parser.LINPTR == 0)
		{
			return true;
		}
		--parser.LINPTR;
		viewer.OUTSTR(parser.M_ERAS);
		return true;
	}
	// Buffer normal characters
	viewer.OUTCHR(c);
	parser.LINBUF[parser.LINPTR] = c;
	++parser.LINPTR;
	viewer.OUTSTR(parser.M_CURS);
	if (parser.LINPTR >= 32)
	{
		goto carriage_return;
	}
	return true;
}

// This method gets called from the scheduler with a
// frequency equal to the current heart rate.  It performs
// damage recovery, indicated by slowing the heartbeat.
int Player::HSLOW()
{
	PLRBLK.P_ATDAM -= (PLRBLK.P_ATDAM >> 6);
	if ((PLRBLK.P_ATDAM & 0x8000) != 0)
	{
		PLRBLK.P_ATDAM = 0;
	}
	HUPDAT();

	// Update Task's next_time
	scheduler.TCBLND[Scheduler::TID_HRTSLOW].next_time = scheduler.curTime +
		(HEARTR * 17);
	
	return 0;
}

// Calculates heartbeat based on power level and damage.
// Also processes fainting and death.
void Player::HUPDAT()
{
	Uint32 ticks1;
	SDL_Event event;

	// Heartrate in source:
	//
	//               PPOW * 64
	// HEARTR = ------------------- - 19
	//           PPOW + (PDAM * 2)
	//
	// The original division routine added one to the integer quotient
	// so that [(1/5) == 1], [(5/5) == 2], [(10/5) == 3], etc.
	// The formula below reflects that peculiarity by only subtracting 18.
	//
	HEARTR = (((PLRBLK.P_ATPOW) * 64) /
			  ((PLRBLK.P_ATPOW) + ((PLRBLK.P_ATDAM) * 2))) - 18;

	if (FAINT == 0)
	{
		// not in a faint
		if (HEARTR <= 3 || (HEARTR & 128) != 0)
		{
			// do faint
			FAINT = -1;
			viewer.clearArea(&viewer.TXTPRI);
			viewer.OLIGHT = viewer.RLIGHT;
			do
			{
				--viewer.MLIGHT;
				--viewer.UPDATE;
				viewer.draw_game();
				--viewer.RLIGHT;
				ticks1 = SDL_GetTicks();
				scheduler.curTime = ticks1;
				do
				{
					if (scheduler.curTime >= scheduler.TCBLND[0].next_time)
					{
						scheduler.CLOCK();
						scheduler.EscCheck();
					}
					scheduler.curTime = SDL_GetTicks();
				} while (scheduler.curTime < ticks1 + 750);
			} while (viewer.RLIGHT != 248);	// not equal to -8
			--viewer.UPDATE;
			parser.KBDHDR = 0;
			parser.KBDTAL = 0;
		}
	}
	else
	{
		// in a faint
		if (HEARTR >= 4 && (HEARTR & 128) == 0)
		{
			// do recover from faint
			do
			{
				--viewer.UPDATE;
				viewer.draw_game();
				++viewer.MLIGHT;
				++viewer.RLIGHT;
				ticks1 = SDL_GetTicks();
				scheduler.curTime = ticks1;
				do
				{
					if (scheduler.curTime >= scheduler.TCBLND[0].next_time)
					{
						scheduler.CLOCK();
						scheduler.EscCheck();
					}
					scheduler.curTime = SDL_GetTicks();
				} while (scheduler.curTime < ticks1 + 750);
			} while (viewer.RLIGHT != viewer.OLIGHT);
			FAINT = 0;
			viewer.PROMPT();
			--viewer.UPDATE;
		}
	}
	if (PLRBLK.P_ATPOW < PLRBLK.P_ATDAM)
	{
		// Do death
		while(SDL_PollEvent(&event))
		{
			; // clear event buffer
		}
		viewer.clearArea(&viewer.TXTSTS);
		viewer.clearArea(&viewer.TXTPRI);
		viewer.ShowFade(Viewer::FADE_DEATH);
		//scheduler.deathFadeLoop();
	}
}

// This method is called every five seconds.  It will
// manage the lit torch's timers.
//
// The full burn time is stored in XX0 in 5-second units,
// so 15 minutes equals 180 5-second units.
//
// The magical light values and physical light values are
// stored in minutes.  Hence the conversion half way thorugh.
//
// This is a hack to give the main timer more granularity,
// so that it doesn't lose a whole minute on each level change,
// but I had to leave the other values as minutes because they
// are used elsewhere in a complicated formula to determine
// the ligthing (line pixelation/fade) values for the 3D viewer.
//
int Player::BURNER()
{
	// Update Task's next_time
	scheduler.TCBLND[Scheduler::TID_TORCHBURN].next_time = 
		scheduler.curTime +
		scheduler.TCBLND[Scheduler::TID_TORCHBURN].frequency;

	dodSHORT A;
	if (PTORCH == -1)
	{
		--viewer.NEWLUK;
		return 0;
	}
	A = object.OCBLND[PTORCH].P_OCXX0;
	if (A == 0)
	{
		--viewer.NEWLUK;
		return 0;
	}
	--A;
	object.OCBLND[PTORCH].P_OCXX0 = A;

	// Convert A to minutes
	(A % 12 == 0) ? A /= 12 : A = (A / 12) + 1;

	if (!(g_cheats&CHEAT_TORCH)) {
		if (A <= 5)
		{
			object.OCBLND[PTORCH].obj_id = Object::OBJ_TORCH_DEAD;
			object.OCBLND[PTORCH].obj_reveal_lvl = 0;
		}
		if (A < object.OCBLND[PTORCH].P_OCXX1)
		{
			object.OCBLND[PTORCH].P_OCXX1 = A;
		}
		if (A < object.OCBLND[PTORCH].P_OCXX2)
		{
			object.OCBLND[PTORCH].P_OCXX2 = A;
		}
	}

	--viewer.NEWLUK;
	return 0;
}

// Used during initialization to set data for either
// the built-in demo or starting a live game
void Player::setInitialObjects(bool isDemo)
{
	int x, y;
	
	if (isDemo)
	{
		game.IsDemo = true;
		game.LEVEL = 2;
		dungeon.SetLEVTABOrig();  //Make sure the original seeds aren't overwritten from pervious new game.
		// demo: iron sword, pine torch, leather shield

		x = object.OBIRTH(Object::OBJ_SWORD_IRON, 0);
		++object.OCBLND[x].P_OCOWN;
		object.OCBFIL(Object::OBJ_SWORD_IRON, x);
		object.OCBLND[x].obj_reveal_lvl = 0;
		BAGPTR = x;
		y = x;

		x = object.OBIRTH(Object::OBJ_TORCH_PINE, 0);
		++object.OCBLND[x].P_OCOWN;
		object.OCBFIL(Object::OBJ_TORCH_PINE, x);
		object.OCBLND[x].obj_reveal_lvl = 0;
		object.OCBLND[y].P_OCPTR = x;
		y = x;

		x = object.OBIRTH(Object::OBJ_SHIELD_LEATHER, 0);
		++object.OCBLND[x].P_OCOWN;
		object.OCBFIL(Object::OBJ_SHIELD_LEATHER, x);
		object.OCBLND[x].obj_reveal_lvl = 0;
		object.OCBLND[y].P_OCPTR = x;
	}
	else
	{
		game.IsDemo = false;
		game.LEVEL = 0;
		if (!game.RandomMaze)  //Do we have an original maze?  Yes:
			dungeon.SetLEVTABOrig();  //Make sure the original seeds aren't overwritten from pervious new game.
		else  //Do we have an original maze?  No:
			dungeon.SetLEVTABRandomMap();  //Sets random seeds for maze.
		PROW = 0x10;
		PCOL = 0x0B;
		PLRBLK.P_ATPOW = 160;

		if (g_cheats&CHEAT_ITEMS)
		{
			x = object.OBIRTH(Object::OBJ_SWORD_IRON, 0);
			++object.OCBLND[x].P_OCOWN;
			object.OCBFIL(Object::OBJ_SWORD_IRON, x);
			object.OCBLND[x].obj_reveal_lvl = 0;
			BAGPTR = x;
			y = x;

			x = object.OBIRTH(Object::OBJ_SHIELD_MITHRIL, 0);
			++object.OCBLND[x].P_OCOWN;
			object.OCBFIL(Object::OBJ_SHIELD_MITHRIL, x);
			object.OCBLND[x].obj_reveal_lvl = 0;
			object.OCBLND[y].P_OCPTR = x;
			y = x;

			x = object.OBIRTH(Object::OBJ_SCROLL_SEER, 0);
			++object.OCBLND[x].P_OCOWN;
			object.OCBFIL(Object::OBJ_SCROLL_VISION, x);
			object.OCBLND[x].obj_reveal_lvl = 0;
			object.OCBLND[y].P_OCPTR = x;
			y = x;

			x = object.OBIRTH(Object::OBJ_TORCH_LUNAR, 0);
			++object.OCBLND[x].P_OCOWN;
			object.OCBFIL(Object::OBJ_TORCH_LUNAR, x);
			object.OCBLND[x].obj_reveal_lvl = 0;
			object.OCBLND[y].P_OCPTR = x;
		}
		else
		{
			x = object.OBIRTH(Object::OBJ_TORCH_PINE, 0);
			++object.OCBLND[x].P_OCOWN;
			object.OCBFIL(Object::OBJ_TORCH_PINE, x);
			object.OCBLND[x].obj_reveal_lvl = 0;
			BAGPTR = x;
			y = x;

			x = object.OBIRTH(Object::OBJ_SWORD_WOOD, 0);
			++object.OCBLND[x].P_OCOWN;
			object.OCBFIL(Object::OBJ_SWORD_WOOD, x);
			object.OCBLND[x].obj_reveal_lvl = 0;
			object.OCBLND[y].P_OCPTR = x;
		}
	}
}

// Processes ATTACK command
void Player::PATTK()
{
	int res, idx, cidx, optr, val;
	OCB *U, *Y;
	dodBYTE r,c;
	SDL_Event event;
	Uint32 ticks1, ticks2;

	res = parser.PARHND();
	if (res == -1)
	{
		return;
	}

	if (res == 0)
	{
		idx = PLHAND;
		if (idx == -1)
		{
			U = &EMPHND;
		}
		else
		{
			U = &object.OCBLND[idx];
		}
	}
	else
	{
		idx = PRHAND;
		if (idx == -1)
		{
			U = &EMPHND;
		}
		else
		{
			U = &object.OCBLND[idx];
		}
	}

	Y = U;
	PMGO = U->P_OCMGO;
	PPHO = U->P_OCPHO;
	PDAM += ((PPOW * (((int) PMGO + (int) PPHO) / 8)) >> 7);

	// make sound for appropriate object
	Mix_PlayChannel(object.objChannel,
		object.objSound[U->obj_type], 0);
	while (Mix_Playing(object.objChannel) == 1)
	{
		if (scheduler.curTime >= scheduler.TCBLND[0].next_time)
		{
			scheduler.CLOCK();
			if (game.AUTFLG && game.demoRestart == false)
			{
				return;
			}
		}
		scheduler.curTime = SDL_GetTicks();
	}

	if (U->obj_id >= Object::OBJ_RING_ENERGY && U->obj_id <= Object::OBJ_RING_FIRE)
	{
		if (!(g_cheats&CHEAT_RING)) {
			--U->P_OCXX0;
			if (U->P_OCXX0 == 0)
			{
				U->obj_id = Object::OBJ_RING_GOLD;
				object.OCBFIL(Object::OBJ_RING_GOLD, idx);
				U->obj_reveal_lvl = 0;
				viewer.STATUS();
			}
		}
	}

	cidx = creature.CFIND2(RowCol(PROW, PCOL));
	if (cidx == -1)
	{
		HUPDAT();
		return;
	}

	if (!ATTACK(PLRBLK.P_ATPOW, creature.CCBLND[cidx].P_CCPOW,
		creature.CCBLND[cidx].P_CCDAM))
	{
		HUPDAT();
		return;
	}

	if (PTORCH == -1 || object.OCBLND[PTORCH].obj_id == Object::OBJ_TORCH_DEAD)
	{
		if ((rng.RANDOM() & 3) != 0)
		{
			HUPDAT();
			return;
		}
	}

	// make KLINK sound
	Mix_PlayChannel(object.objChannel, klink, 0);
	while (Mix_Playing(object.objChannel) == 1)
	{
		if (scheduler.curTime >= scheduler.TCBLND[0].next_time)
		{
			scheduler.CLOCK();
			if (game.AUTFLG && game.demoRestart == false)
			{
				return;
			}
		}
		scheduler.curTime = SDL_GetTicks();
	}

	viewer.OUTSTI(viewer.exps);

	// do damage
	if (DAMAGE(PLRBLK.P_ATPOW, PLRBLK.P_ATMGO, PLRBLK.P_ATPHO,
			   creature.CCBLND[cidx].P_CCPOW,
			   creature.CCBLND[cidx].P_CCMGD,
			   creature.CCBLND[cidx].P_CCPHD,
			   &creature.CCBLND[cidx].P_CCDAM) == true)
	{
		// Creature still alive
		HUPDAT();
		return;
	}

	optr = creature.CCBLND[cidx].P_CCOBJ;
	while (optr != -1)
	{
		object.OCBLND[optr].P_OCOWN = 0;
		object.OCBLND[optr].P_OCROW = creature.CCBLND[cidx].P_CCROW;
		object.OCBLND[optr].P_OCCOL = creature.CCBLND[cidx].P_CCCOL;
		optr = object.OCBLND[optr].P_OCPTR;
	}

	--creature.CMXLND[creature.CMXPTR + creature.CCBLND[cidx].creature_id];
	creature.CCBLND[cidx].P_CCUSE = 0;
	viewer.PUPDAT();

	// do loud explosion sound
	Mix_PlayChannel(object.objChannel, bang, 0);
	while (Mix_Playing(object.objChannel) == 1)
	{
		if (scheduler.curTime >= scheduler.TCBLND[0].next_time)
		{
			scheduler.CLOCK();
			if (game.AUTFLG && game.demoRestart == false)
			{
				return;
			}
		}
		scheduler.curTime = SDL_GetTicks();
	}

	PPOW += (creature.CCBLND[cidx].P_CCPOW >> 3);
	if ((PPOW & 0x8000) != 0)
	{
		PPOW = 0x7FFF;
	}

	if (creature.CCBLND[cidx].creature_id == Creature::CRT_WIZIMG)
	{
		// Wizard's Image Killed
		// transport to 4th level

		// do fade in with message
		
		// Pause so player can see scroll
		ticks1 = SDL_GetTicks();
		do
		{
			ticks2 = SDL_GetTicks();
		} while (ticks2 < ticks1 + wizDelay);
		
		while(SDL_PollEvent(&event))
		{
			; // clear event buffer
		}
		viewer.clearArea(&viewer.TXTSTS);
		viewer.clearArea(&viewer.TXTPRI);
		viewer.done = false;
		viewer.fadeVal = -2;
		viewer.VCTFAD = 32;
		viewer.VXSCAL = 0x80;
		viewer.VYSCAL = 0x80;
		viewer.VXSCALf = 128.0f;
		viewer.VYSCALf = 128.0f;
/*
		// Start buzz
		Mix_Volume(viewer.fadChannel, 0);
		Mix_PlayChannel(viewer.fadChannel, creature.buzz, -1);

		while (!viewer.done)
		{
			viewer.enough_fade();
			scheduler.EscCheck();
		}

		// Stop buzz
		Mix_HaltChannel(viewer.fadChannel);
*/
		viewer.ShowFade(Viewer::FADE_MIDDLE);

		BAGPTR = PTORCH;
		if (PTORCH != -1)
		{
			object.OCBLND[PTORCH].P_OCPTR = -1;
		}

		POBJWT = 200;
		game.LEVEL = 3;
		creature.NEWLVL();

		do
		{
			c = (rng.RANDOM() & 31);
			r = (rng.RANDOM() & 31);
			val = dungeon.MAZLND[dungeon.RC2IDX(r, c)];
		} while (val == 0xFF);

		PROW = r;
		PCOL = c;

		game.INIVU();
	}

	if (creature.CCBLND[cidx].creature_id != Creature::CRT_WIZARD)
	{
		HUPDAT();
		if (game.CreaturesInstaRegen) creature.CREGEN();  //Regen creature intantly after death if option selected.
		return;
	}

	// Killed the real Wizard
	--creature.FRZFLG;
	PRLITE = 0x07;
	PMLITE = 0x13;
	object.OCBPTR = 1;
	BAGPTR = -1;
	PTORCH = -1;
	PRHAND = -1;
	PLHAND = -1;
	game.INIVU();
	HUPDAT();
	return;
}

// Determines if an attack strikes its target
bool Player::ATTACK(int AP, int DP, int DD)
{
	int pidx, adjust, ret;
	int T0 = 15;
	int Dval = (DP - DD) * 4;

	do
	{
		Dval -= AP;
		if (Dval < 0)
		{
			break;
		}
		--T0;
	} while (T0 > 0);

	pidx = T0 - 3;
	if (pidx > 0)
	{
		adjust = pidx * 10;
	}
	else
	{
		adjust = pidx * 25;
	}

	ret = rng.RANDOM() + adjust - 127;
	
	if (ret < 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

// Calculates and assesses damage from a successful attack
bool Player::DAMAGE(int AP, int AMO, int APO,
					int DP, int DMD, int DPD, dodSHORT * DD)
{
	int a;

	a = ((AP * AMO) >> 7);
	a = ((a * DMD) >> 7);
	*DD += (dodSHORT) a;

	a = ((AP * APO) >> 7);
	a = ((a * DPD) >> 7);
	*DD += (dodSHORT) a;

	if ((dodSHORT) DP > *DD)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// Processes CLIMB command
void Player::PCLIMB()
{
	Uint32 ticks1;

	dodBYTE vres;
	int		res;
	dodBYTE	A, B;
	RowCol	rc;
	dodSHORT temp;
	
	rc.setRC(PROW,PCOL);
	vres = dungeon.VFIND(rc);
	if (vres == Dungeon::VF_NULL)
	{
		parser.CMDERR();
		return;
	}
	res = parser.PARSER(parser.DIRTAB, A, B, true);
	if (res <= 0)
	{
		parser.CMDERR();
		return;
	}
	else
	{
		if (A == Parser::DIR_UP)
		{
			// Climb Up
			if (vres == Dungeon::VF_LADDER_UP ||
				((vres == Dungeon::VF_HOLE_UP) && creature.FRZFLG))// can only clumb up pits after you win
			{
				viewer.displayPrepare();
				temp = viewer.display_mode;
				viewer.display_mode = Viewer::MODE_TITLE;
				viewer.draw_game();
				ticks1 = SDL_GetTicks();
				scheduler.curTime = ticks1;
				do
				{
					if (scheduler.curTime >= scheduler.TCBLND[0].next_time)
					{
						scheduler.CLOCK();
					}
					scheduler.curTime = SDL_GetTicks();
				} while (scheduler.curTime < ticks1 + viewer.prepPause);
				viewer.display_mode = temp;
				--game.LEVEL;
				creature.NEWLVL();
				game.INIVU();
			}
			else
			{
				parser.CMDERR();
				return;
			}
		}
		else if (A == Parser::DIR_DOWN)
		{
			// Climb Down
			if (vres == Dungeon::VF_LADDER_DOWN || vres == Dungeon::VF_HOLE_DOWN)
			{
				viewer.displayPrepare();
				temp = viewer.display_mode;
				viewer.display_mode = Viewer::MODE_TITLE;
				viewer.draw_game();
				ticks1 = SDL_GetTicks();
				scheduler.curTime = ticks1;
				do
				{
					if (scheduler.curTime >= scheduler.TCBLND[0].next_time)
					{
						scheduler.CLOCK();
					}
					scheduler.curTime = SDL_GetTicks();
				} while (scheduler.curTime < ticks1 + viewer.prepPause);
				viewer.display_mode = temp;
				++game.LEVEL;
				creature.NEWLVL();
				game.INIVU();
			}
			else
			{
				parser.CMDERR();
				return;
			}
		}
		else
		{
			parser.CMDERR();
			return;
		}
	}
}

// Processes DROP command
void Player::PDROP()
{
	int res, idx;
	res = parser.PARHND();
	if (res == -1)
	{
		return;
	}
	if (res == 0 && PLHAND == -1)
	{
		parser.CMDERR();
		return;
	}
	if (res == 1 && PRHAND == -1)
	{
		parser.CMDERR();
		return;
	}

	if (res == 0)
	{
		idx = PLHAND;
		PLHAND = -1;
	}
	else
	{
		idx = PRHAND;
		PRHAND = -1;
	}

	object.OCBLND[idx].P_OCOWN = 0;
	object.OCBLND[idx].P_OCROW = PROW;
	object.OCBLND[idx].P_OCCOL = PCOL;
	object.OCBLND[idx].P_OCLVL = game.LEVEL;

	POBJWT -= object.OBJWGT[object.OCBLND[idx].obj_type];
	HUPDAT();

	viewer.STATUS();
	viewer.PUPDAT();
}

// Processes EXAMINE command
void Player::PEXAM()
{
	viewer.display_mode = Viewer::MODE_EXAMINE;
	viewer.PUPDAT();
}

// Processes GET command
void Player::PGET()
{
	int res, idx;
	res = parser.PARHND();
	if (res == -1)
	{
		return;
	}
	if (res == 0 && PLHAND != -1)
	{
		parser.CMDERR();
		return;
	}
	if (res == 1 && PRHAND != -1)
	{
		parser.CMDERR();
		return;
	}
	if ( !object.PAROBJ() )
	{
		return;
	}

	bool match = false;
	object.OFINDF = 0;
	do
	{
		idx = object.OFIND(RowCol(PROW, PCOL));
		if (idx == -1)
		{
			parser.CMDERR();
			return;
		}
		if (object.SPEFLG == 0)
		{
			if (object.OBJCLS == object.OCBLND[idx].obj_type)
			{
				match = true;
			}
		}
		else
		{
			if (object.OBJTYP == object.OCBLND[idx].obj_id)
			{
				match = true;
			}
		}
	} while (match == false);

	if (res == 0)
	{
		PLHAND = idx;
	}
	else
	{
		PRHAND = idx;
	}
	++object.OCBLND[idx].P_OCOWN;

	POBJWT += object.OBJWGT[object.OCBLND[idx].obj_type];
	HUPDAT();

	viewer.STATUS();
	viewer.PUPDAT();
}

// Processes INCANT command
void Player::PINCAN()
{
	int		res;
	dodBYTE	A, B;
	RowCol	rc;
	SDL_Event event;
	Uint32 ticks1, ticks2;
	
	res = parser.PARSER(object.ADJTAB, A, B, true);
	if (res <= 0)
	{
		return;
	}

	if (parser.FULFLG == 0)
	{
		return;
	}

	object.OBJTYP = A;
	object.OBJCLS = B;

	if (PLHAND != -1)
	{
		if (object.OCBLND[PLHAND].obj_type == Object::OBJT_RING)
		{
			if (object.OCBLND[PLHAND].P_OCXX1 == object.OBJTYP)
			{
				object.OCBLND[PLHAND].obj_id = object.OBJTYP;
				object.OCBFIL(object.OBJTYP, PLHAND);

				// make ring sound
				Mix_PlayChannel(object.objChannel,
					object.objSound[object.OCBLND[PLHAND].obj_type], 0);
				while (Mix_Playing(object.objChannel) == 1)
				{
					if (scheduler.curTime >= scheduler.TCBLND[0].next_time)
					{
						scheduler.CLOCK();
					}
					scheduler.curTime = SDL_GetTicks();
				}
				
				viewer.STATUS();
				viewer.PUPDAT();
				object.OCBLND[PLHAND].P_OCXX1 = -1;
				if (object.OBJTYP == 0x12)
				{
					// winner
					while(SDL_PollEvent(&event))
					{
						; // clear event buffer
					}

					// Pause so player can see status line
					ticks1 = SDL_GetTicks();
					do
					{
						ticks2 = SDL_GetTicks();
					} while (ticks2 < ticks1 + wizDelay);

					viewer.clearArea(&viewer.TXTSTS);
					viewer.clearArea(&viewer.TXTPRI);
					viewer.ShowFade(Viewer::FADE_VICTORY);
					game.hasWon = true;
				}
				else
				{
					return;
				}
			}
		}
	}

	if (PRHAND != -1)
	{
		if (object.OCBLND[PRHAND].obj_type == Object::OBJT_RING)
		{
			if (object.OCBLND[PRHAND].P_OCXX1 == object.OBJTYP)
			{
				object.OCBLND[PRHAND].obj_id = object.OBJTYP;
				object.OCBFIL(object.OBJTYP, PRHAND);

				// make ring sound
				Mix_PlayChannel(object.objChannel,
					object.objSound[object.OCBLND[PRHAND].obj_type], 0);
				while (Mix_Playing(object.objChannel) == 1)
				{
					if (scheduler.curTime >= scheduler.TCBLND[0].next_time)
					{
						scheduler.CLOCK();
					}
					scheduler.curTime = SDL_GetTicks();
				}
				
				viewer.STATUS();
				viewer.PUPDAT();
				object.OCBLND[PRHAND].P_OCXX1 = -1;
				if (object.OBJTYP == 0x12)
				{
					// Do winner
					while(SDL_PollEvent(&event))
					{
						; // clear event buffer
					}

					// Pause so player can see status line
					ticks1 = SDL_GetTicks();
					do
					{
						ticks2 = SDL_GetTicks();
					} while (ticks2 < ticks1 + wizDelay);

					viewer.clearArea(&viewer.TXTSTS);
					viewer.clearArea(&viewer.TXTPRI);
					viewer.ShowFade(Viewer::FADE_VICTORY);
					game.hasWon = true;
				}
				else
				{
					return;
				}
			}
		}
	}
}

// Processes LOOK command
void Player::PLOOK()
{
	viewer.display_mode = Viewer::MODE_3D;
	viewer.PUPDAT();
}

// Processes MOVE command
void Player::PMOVE()
{
	int		res;
	dodBYTE	A, B;
	Uint32 ticks1;
	
	res = parser.PARSER(parser.DIRTAB, A, B, true);
	if (res < 0)
	{
		parser.CMDERR();
		return;
	}
	else if (res == 0)
	{
		// Move Forward
		--viewer.HLFSTP;
		viewer.PUPDAT();
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
			}
			scheduler.curTime = SDL_GetTicks();
		} while (scheduler.curTime < ticks1 + (moveDelay / 2));
		viewer.HLFSTP = 0;
		PSTEP(0);
		PDAM += (POBJWT >> 3) + 3;
		HUPDAT();
		--viewer.UPDATE;
		viewer.draw_game();
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
			}
			scheduler.curTime = SDL_GetTicks();
		} while (scheduler.curTime < ticks1 + (moveDelay / 2));
		return;
	}
	else if (A == Parser::DIR_BACK)
	{
		// Move Back
		--viewer.BAKSTP;
		viewer.PUPDAT();
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

			}
			scheduler.curTime = SDL_GetTicks();
		} while (scheduler.curTime < ticks1 + (moveDelay / 2));
		viewer.BAKSTP = 0;
		PSTEP(2);
		PDAM += (POBJWT / 8) + 3;
		HUPDAT();
		--viewer.UPDATE;
		viewer.draw_game();
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
			}
			scheduler.curTime = SDL_GetTicks();
		} while (scheduler.curTime < ticks1 + (moveDelay / 2));
		return;
	}
	else if (A == Parser::DIR_RIGHT)
	{
		// Move Right
		if (PSTEP(1))
		{
			if (viewer.display_mode == Viewer::MODE_3D)
			{
				ShowTurn(Parser::DIR_RIGHT);
			}
		}
		PDAM += (POBJWT / 8) + 3;
		HUPDAT();
		--viewer.UPDATE;
		viewer.draw_game();
		return;
	}
	else if (A == Parser::DIR_LEFT)
	{
		// Move Left
		if (PSTEP(3))
		{
			if (viewer.display_mode == Viewer::MODE_3D)
			{
				ShowTurn(Parser::DIR_LEFT);
			}
		}
		PDAM += (POBJWT / 8) + 3;
		HUPDAT();
		--viewer.UPDATE;
		viewer.draw_game();
		return;
	}
	else
	{
		parser.CMDERR();
		return;
	}
}

// Processes PULL command
void Player::PPULL()
{
	if (BAGPTR == -1)
	{
		parser.CMDERR();
		return;
	}
	
	int res;
	res = parser.PARHND();
	if (res == -1)
	{
		return;
	}
	if (res == 0 && PLHAND != -1)
	{
		parser.CMDERR();
		return;
	}
	if (res == 1 && PRHAND != -1)
	{
		parser.CMDERR();
		return;
	}
	if ( !object.PAROBJ() )
	{
		return;
	}

	bool onHead = true;
	bool match = false;
	int curPtr, prevPtr;

	do
	{
		if (onHead)
		{
			curPtr = BAGPTR;
		}
		else
		{
			prevPtr = curPtr;
			curPtr = object.OCBLND[curPtr].P_OCPTR;
			if (curPtr == -1)
			{
				parser.CMDERR();
				return;
			}
		}

		if (object.SPEFLG == 0)
		{
			if (object.OCBLND[curPtr].obj_type == object.OBJCLS)
			{
				match = true;
			}
		}
		else
		{
			if (object.OCBLND[curPtr].obj_id == object.OBJTYP)
			{
				match = true;
			}
		}
		if (match)
		{
			break;
		}
		if (onHead)
		{
			onHead = false;
		}
	} while (true);

	if (onHead)
	{
		BAGPTR = object.OCBLND[curPtr].P_OCPTR;
	}
	else
	{
		object.OCBLND[prevPtr].P_OCPTR = object.OCBLND[curPtr].P_OCPTR;
	}

	if (res == 0)
	{
		PLHAND = curPtr;
	}
	else
	{
		PRHAND = curPtr;
	}

	if (curPtr == PTORCH)
	{
		PTORCH = -1;
	}

	viewer.STATUS();
	viewer.PUPDAT();
}

// Processes REVEAL command
void Player::PREVEA()
{
	int res, idx, req;
	res = parser.PARHND();
	if (res == -1)
	{
		return;
	}
	if (res == 0 && PLHAND == -1)
	{
		return;
	}
	if (res == 1 && PRHAND == -1)
	{
		return;
	}
	if (res == 0)
	{
		idx = PLHAND;
	}
	else
	{
		idx = PRHAND;
	}
	req = object.OCBLND[idx].obj_reveal_lvl;
	if (req && ((req * 25 <= PPOW) || (g_cheats&CHEAT_REVEAL) || (req == 50 && game.VisionScroll && 400 <= PPOW)))
	{
		object.OCBFIL(object.OCBLND[idx].obj_id, idx);
		object.OCBLND[idx].obj_reveal_lvl = 0;
		viewer.STATUS();
	}
}

// Processes STOW command
void Player::PSTOW()
{
	int res;
	res = parser.PARHND();
	if (res == -1)
	{
		return;
	}
	if (res == 0 && PLHAND == -1)
	{
		parser.CMDERR();
		return;
	}
	if (res == 1 && PRHAND == -1)
	{
		parser.CMDERR();
		return;
	}

	if (res == 0)
	{
		object.OCBLND[PLHAND].P_OCPTR = BAGPTR;
		BAGPTR = PLHAND;
		PLHAND = -1;
	}
	else
	{
		object.OCBLND[PRHAND].P_OCPTR = BAGPTR;
		BAGPTR = PRHAND;
		PRHAND = -1;
	}
	viewer.STATUS();
	viewer.PUPDAT();
}

// Processes TURN command
void Player::PTURN()
{
	int		res;
	dodBYTE	A, B;
	
	res = parser.PARSER(parser.DIRTAB, A, B, true);
	if (res != 1)
	{
		parser.CMDERR();
		return;
	}
	if (A == Parser::DIR_LEFT)
	{
		// Left Turn
		--PDIR;
		PDIR = (PDIR & 3);
		if (viewer.display_mode == Viewer::MODE_3D)
		{
			ShowTurn(Parser::DIR_LEFT);
		}
		--viewer.UPDATE;
		viewer.draw_game();
		return;
	}
	else if (A == Parser::DIR_RIGHT)
	{
		// Right Turn
		++PDIR;
		PDIR = (PDIR & 3);
		if (viewer.display_mode == Viewer::MODE_3D)
		{
			ShowTurn(Parser::DIR_RIGHT);
		}
		--viewer.UPDATE;
		viewer.draw_game();
		return;
	}
	else if (A == Parser::DIR_AROUND)
	{
		// About Face
		PDIR += 2;
		PDIR = (PDIR & 3);
		if (viewer.display_mode == Viewer::MODE_3D)
		{
			ShowTurn(Parser::DIR_AROUND);
		}
		--viewer.UPDATE;
		viewer.draw_game();
		return;
	}
	else
	{
		parser.CMDERR();
		return;
	}
}

// Turning Animation
void Player::ShowTurn(dodBYTE A)
{
	int ctr, times, x;
	int offset, dir;
	int inc = 32;
	int lines = 8;
	int y0 = 17;
	int y1 = 135;
	Uint32 ticks1;
	bool redraw = true;

	switch (A)
	{
	case Parser::DIR_LEFT:
		offset = 8;
		dir = 1;
		times = 1;
		break;
	case Parser::DIR_RIGHT:
		offset = 248;
		dir = -1;
		times = 1;
		break;
	case Parser::DIR_AROUND:
		offset = 248;
		dir = -1;
		times = 2;
		break;
	default:
		break;
	}

	viewer.VXSCAL = 0x80;
	viewer.VYSCAL = 0x80;
	viewer.VXSCALf = 128.0f;
	viewer.VYSCALf = 128.0f;
	viewer.RANGE = 0;
	viewer.SETFAD();
	glColor3fv(viewer.fgColor);
	turning = true;
	for (ctr = 0; ctr < times; ++ctr)
	{
		for (x = 0; x < lines; ++x)
		{
			ticks1 = SDL_GetTicks();
			do
			{
				scheduler.curTime = SDL_GetTicks();
				if (scheduler.curTime >= scheduler.TCBLND[0].next_time)
				{
					scheduler.CLOCK();
					if (game.AUTFLG && game.demoRestart == false)
					{
						turning = false;
						return;
					}
					redraw = true;
				}

				if (redraw)
				{
					glClear(GL_COLOR_BUFFER_BIT);
					glLoadIdentity();
					viewer.drawVectorList(viewer.LINES);
					viewer.drawVector((x*inc*dir)+offset,y0,(x*inc*dir)+offset,y1);
					viewer.drawArea(&viewer.TXTSTS);
					viewer.drawArea(&viewer.TXTPRI);
					SDL_GL_SwapBuffers();
					redraw = false;
				}
			} while (scheduler.curTime < ticks1 + turnDelay);
		}
	}
	turning = false;
	--HEARTF;
}

// Processes USE command
void Player::PUSE()
{
	int res, idx;
	res = parser.PARHND();
	if (res == -1)
	{
		return;
	}
	if (res == 0 && PLHAND == -1)
	{
		return;
	}
	if (res == 1 && PRHAND == -1)
	{
		return;
	}
	if (res == 0)
	{
		idx = PLHAND;
	}
	else
	{
		idx = PRHAND;
	}

	if (object.OCBLND[idx].obj_type  == Object::OBJT_TORCH)
	{
		PTORCH = idx;
		if (res == 0)
		{
			object.OCBLND[PLHAND].P_OCPTR = BAGPTR;
			BAGPTR = PLHAND;
			PLHAND = -1;
		}
		else
		{
			object.OCBLND[PRHAND].P_OCPTR = BAGPTR;
			BAGPTR = PRHAND;
			PRHAND = -1;
		}
		viewer.STATUS();
		viewer.PUPDAT();

		// make torch sound
		Mix_PlayChannel(object.objChannel,
			object.objSound[object.OCBLND[idx].obj_type], 0);
		while (Mix_Playing(object.objChannel) == 1)
		{
			if (scheduler.curTime >= scheduler.TCBLND[0].next_time)
			{
				scheduler.CLOCK();
			}
			scheduler.curTime = SDL_GetTicks();
		}
		
		viewer.PUPDAT();
		return;
	}
	else if (object.OCBLND[idx].obj_id  == Object::OBJ_FLASK_THEWS)
	{
		PPOW += 1000;
		object.OCBLND[idx].obj_id = Object::OBJ_FLASK_EMPTY;
		object.OCBLND[idx].obj_reveal_lvl = 0;
		
		// make flask sound
		Mix_PlayChannel(object.objChannel,
			object.objSound[object.OCBLND[idx].obj_type], 0);
		while (Mix_Playing(object.objChannel) == 1)
		{
			if (scheduler.curTime >= scheduler.TCBLND[0].next_time)
			{
				scheduler.CLOCK();
			}
			scheduler.curTime = SDL_GetTicks();
		}
		
		viewer.STATUS();
		HUPDAT();
	}
	else if (object.OCBLND[idx].obj_id  == Object::OBJ_FLASK_HALE) // Hale Flask
	{
		PDAM = 0;
		object.OCBLND[idx].obj_id = Object::OBJ_FLASK_EMPTY;
		object.OCBLND[idx].obj_reveal_lvl = 0;
		
		// make flask sound
		Mix_PlayChannel(object.objChannel,
			object.objSound[object.OCBLND[idx].obj_type], 0);
		while (Mix_Playing(object.objChannel) == 1)
		{
			if (scheduler.curTime >= scheduler.TCBLND[0].next_time)
			{
				scheduler.CLOCK();
			}
			scheduler.curTime = SDL_GetTicks();
		}
		
		viewer.STATUS();
		HUPDAT();
	}
	else if (object.OCBLND[idx].obj_id  == Object::OBJ_FLASK_ABYE) // Abye Flask
	{
		if (!(g_cheats&CHEAT_INVULNERABLE))
			PDAM += (short) ((double) PPOW * 0.8);

		object.OCBLND[idx].obj_id = Object::OBJ_FLASK_EMPTY;
		object.OCBLND[idx].obj_reveal_lvl = 0;
		
		// make flask sound
		Mix_PlayChannel(object.objChannel,
			object.objSound[object.OCBLND[idx].obj_type], 0);
		while (Mix_Playing(object.objChannel) == 1)
		{
			if (scheduler.curTime >= scheduler.TCBLND[0].next_time)
			{
				scheduler.CLOCK();
			}
			scheduler.curTime = SDL_GetTicks();
		}
		
		viewer.STATUS();
		HUPDAT();
	}
	else if (object.OCBLND[idx].obj_id  == Object::OBJ_SCROLL_SEER)
	{
		viewer.showSeerMap = true;
		if (object.OCBLND[idx].obj_reveal_lvl != 0)
		{
			return;
		}
		
		// make scroll sound
		Mix_PlayChannel(object.objChannel,
			object.objSound[object.OCBLND[idx].obj_type], 0);
		while (Mix_Playing(object.objChannel) == 1)
		{
			if (scheduler.curTime >= scheduler.TCBLND[0].next_time)
			{
				scheduler.CLOCK();
			}
			scheduler.curTime = SDL_GetTicks();
		}
		
		HEARTF = 0;
		viewer.display_mode = Viewer::MODE_MAP;
		viewer.PUPDAT();
		return;
	}
	else if (object.OCBLND[idx].obj_id  == Object::OBJ_SCROLL_VISION)
	{
		viewer.showSeerMap = false;
		if (object.OCBLND[idx].obj_reveal_lvl != 0)
		{
			return;
		}
		
		// make scroll sound
		Mix_PlayChannel(object.objChannel,
			object.objSound[object.OCBLND[idx].obj_type], 0);
		while (Mix_Playing(object.objChannel) == 1)
		{
			if (scheduler.curTime >= scheduler.TCBLND[0].next_time)
			{
				scheduler.CLOCK();
			}
			scheduler.curTime = SDL_GetTicks();
		}
		
		HEARTF = 0;
		viewer.display_mode = Viewer::MODE_MAP;
		viewer.PUPDAT();
		return;
	}
}

// Processes ZLOAD command
void Player::PZLOAD()
{
	int tctr = 0;
	int preLen;
	memset(parser.TOKEN, -1, 33);
	memset(oslink.gamefile,0,oslink.gamefileLen);
	strcpy(oslink.gamefile,oslink.savedDir);
	strcat(oslink.gamefile,oslink.pathSep);
	preLen = strlen(oslink.gamefile);
	if (parser.GETTOK())
	{
		strcpy(oslink.gamefile,oslink.savedDir);
		strcat(oslink.gamefile,oslink.pathSep);
		tctr = 0;
		while (parser.TOKEN[tctr] != 0xFF)
		{
			oslink.gamefile[tctr + preLen] = parser.TOKEN[tctr] + 'A' - 1;
			++tctr;
		}
		strcat(oslink.gamefile,".dod");
	}
	else
	{
		strcat(oslink.gamefile,"game.dod");
	}

	if ((oslink.fptr = fopen(oslink.gamefile,"r")) == NULL)
	{
		parser.CMDERR();
		return;
	}
	else
	{
		fclose(oslink.fptr);
		--scheduler.ZFLAG;
		return;
	}
}

// Processes ZSAVE command
void Player::PZSAVE()
{
	int tctr = 0;
	int preLen;
	memset(parser.TOKEN, -1, 33);
	memset(oslink.gamefile,0,oslink.gamefileLen);
	strcpy(oslink.gamefile,oslink.savedDir);
	strcat(oslink.gamefile,oslink.pathSep);
	preLen = strlen(oslink.gamefile);
	if (parser.GETTOK())
	{
		while (parser.TOKEN[tctr] != 0xFF)
		{
			oslink.gamefile[tctr + preLen] = parser.TOKEN[tctr] + 'A' - 1;
			++tctr;
		}
		strcat(oslink.gamefile,".dod");
	}
	else
	{
		strcat(oslink.gamefile,"game.dod");
	}

	++scheduler.ZFLAG;
	return;
}

// Attempts to move player in given direction
bool Player::PSTEP(dodBYTE dir)
{
	dodBYTE B;
	B = dir + PDIR;
	B &= 3;
	if (dungeon.STEPOK(PROW, PCOL, B))
	{
		PROW += dungeon.STPTAB[B * 2];
		PCOL += dungeon.STPTAB[(B * 2) + 1];
		return true;
	}
	else
	{
		// do thud sound
		Mix_PlayChannel(object.objChannel, thud, 0);
		while (Mix_Playing(object.objChannel) == 1)
		{
			if (scheduler.curTime >= scheduler.TCBLND[0].next_time)
			{
				scheduler.CLOCK();
			}
			scheduler.curTime = SDL_GetTicks();
		}
		
		return false;
	}
}

