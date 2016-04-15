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
// Filename: dungeon.cpp
//
// Implementation of Dungeon class

#include "dungeon.h"
#include "dodgame.h"
#include "player.h"
#include "sched.h"

extern Scheduler	scheduler;
extern Player		player;
extern dodGame		game;

// Scaffolding Code

#include <stdio.h>

// Prints a text drawing of the maze
void Dungeon::printMaze()
{
	int idx, row, x;
	dodBYTE val, n, e, s, w;
	for (idx=0; idx<1024; idx+=32)
	{
		for (x = 0; x < 3; ++x)
		{
			for (row = 0; row < 32; ++row)
			{
				val = MAZLND[idx+row];
				n = (val & 0x03);
				e = (val & 0x0C) >> 2;
				s = (val & 0x30) >> 4;
				w = (val & 0xC0) >> 6;
				switch (x)
				{
				case 0:
					printf("·%c", NS[n]);
					if (row >= 31)
					{
						printf("·");
					}
					break;
				case 1:
					printf("%c", EW[w]);
					if (val == 0xFF)
						printf("#");
					else
						printf(" ");
					if (row >= 31)
					{
						printf("%c", EW[e]);
					}
					break;
				case 2:
					if (idx >= 992)
					{
						printf("·%c", NS[s]);
						if (row >= 31)
						{
							printf("·");
						}
					}
				}
			}
			if (x < 2)
			{
				printf("\n");
			}
		}
	}
}

// Constructor
Dungeon::Dungeon() : VFTPTR(0)
{
	SetLEVTABOrig();  //Original seed values will be overwritten (in Player::setInitialObjects())
					 //if new random map game.
	
	MSKTAB[0] = 0x03;
	MSKTAB[1] = 0x0C;
	MSKTAB[2] = 0x30;
	MSKTAB[3] = 0xC0;

	DORTAB[0] = HF_DOR;
	DORTAB[1] = (HF_DOR << 2);
	DORTAB[2] = (HF_DOR << 4);
	DORTAB[3] = (HF_DOR << 6);

	SDRTAB[0] = HF_SDR;
	SDRTAB[1] = (HF_SDR << 2);
	SDRTAB[2] = (HF_SDR << 4);
	SDRTAB[3] = (HF_SDR << 6);

	STPTAB[0] = -1;
	STPTAB[1] = 0;
	STPTAB[2] = 0;
	STPTAB[3] = 1;
	STPTAB[4] = 1;
	STPTAB[5] = 0;
	STPTAB[6] = 0;
	STPTAB[7] = -1;

	SetVFTTABOrig();  //Original vertical feature table values will be overwritten (in Dungeon::DGNGEN())
					 //if new random map game.

	// Scaffolding Inits
	NS[3]='—';
	NS[2]='=';
	NS[1]='—';
	NS[0]=' ';
	EW[3]='|';
	EW[2]=')';
	EW[1]='|';
	EW[0]=' ';
}

// This method can probably be streamlined since it
// was written very early.  It builds the maze.
void Dungeon::DGNGEN()
{
	/* Locals */
	int		mzctr;
	int		maz_idx;
	int		cell_ctr;
	dodBYTE	a_row;
	dodBYTE	a_col;
	dodBYTE	b_row;
	dodBYTE	b_col;
	dodBYTE	DIR;
	dodBYTE	DST;
	RowCol	DROW;
	RowCol	ROW;
	int		spin;
	
	/* Phase 1: Create Maze */

	/* Set Cells to 0xFF */
	for (mzctr=0; mzctr<1024; ++mzctr)
	{
		MAZLND[mzctr] = 0xFF;
	}

	rng.setSEED(LEVTAB[game.LEVEL], LEVTAB[game.LEVEL+1], LEVTAB[game.LEVEL+2]);  //Initialize Random Number Generator
	cell_ctr = 500;  // Room Counter

	/* Set Starting Room */
	if (!game.RandomMaze || game.IsDemo)
	{  //Is this an original game?  Yes:		
		a_col = (rng.RANDOM() & 31);
		a_row = (rng.RANDOM() & 31);
		DROW.setRC(a_row, a_col);
		RndDstDir(&DIR, &DST);
		SetVFTTABOrig();  //Make sure the vertical feature table isn't overwritten from pervious new game.
	} else {  //Is this an original game?  No:
		switch (game.LEVEL)
		{
			case 0:
			case 3:
				a_col = (rng.RANDOM() & 31);
				a_row = (rng.RANDOM() & 31);
				break;
			case 1:
				a_row = VFTTAB[5];
				a_col = VFTTAB[6];
				break;
			case 2:
				a_row = VFTTAB[9];
				a_col = VFTTAB[10];
				break;
			default:
				a_row = VFTTAB[14];
				a_col = VFTTAB[15];
				break;
		}
	
		if (player.PROW == 0x10 && player.PCOL == 0x0B && game.LEVEL == 0)
		{  //Are we starting a new game?
			player.PROW = a_row;
			player.PCOL = a_col;

			//Override veritical features.
			//Will override other level's col & row during map generation.
			SetVFTTABRandomMap();
			VFTTAB[1] = a_row;
			VFTTAB[2] = a_col;
		}  //Are we starting a new game?

		//Original didn't tunnel out original room.
		//Need to do it now so that player doesn't start in wall in beginning of game.
		//Also need to make sure ladder back up to each level is in a tunneled out room.
		DROW.setRC(a_row, a_col);
		RndDstDir(&DIR, &DST);
		maz_idx = RC2IDX(a_row, a_col);
		MAZLND[maz_idx] = 0;
		--cell_ctr;
	}  //Is this an original game?


	while (cell_ctr > 0)
	{
		/* Take a step */
		b_row = DROW.row;
		b_col = DROW.col;
		b_row += STPTAB[DIR * 2];
		b_col += STPTAB[(DIR * 2) + 1];

		/* Check if it's out of bounds */
		if (BORDER(b_row, b_col) == false)
		{
			RndDstDir(&DIR, &DST);
			continue;
		}

		/* Store index and temp room */
		maz_idx = RC2IDX(b_row, b_col);
		ROW.setRC(b_row, b_col);

		/* If not yet touched */
		if (MAZLND[maz_idx] == 0xFF)
		{
			FRIEND(ROW);
			if (NEIBOR[3] + NEIBOR[0] + NEIBOR[1] == 0 ||
				NEIBOR[1] + NEIBOR[2] + NEIBOR[5] == 0 ||
				NEIBOR[5] + NEIBOR[8] + NEIBOR[7] == 0 ||
				NEIBOR[7] + NEIBOR[6] + NEIBOR[3] == 0)
			{
				RndDstDir(&DIR, &DST);
				continue;
			}
			MAZLND[maz_idx] = 0;
			--cell_ctr;
		}
		if (cell_ctr > 0)
		{
			DROW = ROW;
			--DST;
			if (DST == 0)
			{
				RndDstDir(&DIR, &DST);
				continue;
			}
			else
			{
				continue;
			}
		}
	}

	/* Phase 2: Create Walls */

	for (DROW.row = 0; DROW.row < 32; ++DROW.row)
	{
		for (DROW.col = 0; DROW.col < 32; ++DROW.col)
		{
			maz_idx = RC2IDX(DROW.row, DROW.col);
			if (MAZLND[maz_idx] != 0xFF)
			{
				FRIEND(DROW);
				if (NEIBOR[1] == 0xFF)
					MAZLND[maz_idx] |= N_WALL;
				if (NEIBOR[3] == 0xFF)
					MAZLND[maz_idx] |= W_WALL;
				if (NEIBOR[5] == 0xFF)
					MAZLND[maz_idx] |= E_WALL;
				if (NEIBOR[7] == 0xFF)
					MAZLND[maz_idx] |= S_WALL;
			}
		}
	}

	/* Phase 3: Create Doors/Secret Doors */

	for (mzctr = 0; mzctr < 70; ++mzctr)
	{
		MAKDOR(this->DORTAB);
	}

	for (mzctr = 0; mzctr < 45; ++mzctr)
	{
		MAKDOR(this->SDRTAB);
	}

	/* Phase 4: Create vertical feature */
	if (game.RandomMaze && !game.IsDemo && (game.LEVEL == 0 || game.LEVEL == 1 || game.LEVEL == 3))
	{
		do
		{
			do
			{
				a_col = (rng.RANDOM() & 31);
				a_row = (rng.RANDOM() & 31);
				ROW.setRC(a_row, a_col);
				maz_idx = RC2IDX(a_row, a_col);
			} while (MAZLND[maz_idx] == 0xFF);
		} while ((game.LEVEL == 0 && VFTTAB[1] == a_row && VFTTAB[2] == a_col) ||
				 (game.LEVEL == 1 && VFTTAB[5] == a_row && VFTTAB[6] == a_col));
		switch (game.LEVEL)
		{
			case 0:
				if (VFTTAB[5] == 0 && VFTTAB[6] == 0) {
					VFTTAB[5] = a_row;
					VFTTAB[6] = a_col;
				}
				break;
			case 1:
				if (VFTTAB[9] == 0 && VFTTAB[10] == 0) {
					VFTTAB[9] = a_row;
					VFTTAB[10] = a_col;
				}
				break;
			default:
				if (VFTTAB[14] == 0 && VFTTAB[15] == 0) {
					VFTTAB[14] = a_row;
					VFTTAB[15] = a_col;
				}
				break;
		}
	}



	// Spin the RNG
	if (scheduler.curTime == 0)
	{
		if (game.LEVEL == 0)
		{
			spin = 6;
		}
		else
		{
			spin = 21;
		}
	}
	else
	{
		spin = (scheduler.curTime % 60);
	}

	while (spin > 0)
	{
		rng.RANDOM();
		--spin;
	}
}

// Adds vertical features
void Dungeon::CalcVFI()
{
	dodBYTE lvl = game.LEVEL;
	dodBYTE idx = 0;
	do
	{
		VFTPTR = idx;
		while (VFTTAB[idx++] != 0xFF)
			;	// loop !!!
		--lvl;
	} while (lvl != 0xFF);
}

// Checks if a hole/ladder is in cell
// It has to check above and below, since each
// vertical feature is stored only once in the VFT
dodBYTE	Dungeon::VFIND(RowCol rc)
{
	int u = VFTPTR;
	dodBYTE a = 0;
	bool res;
	res = VFINDsub(a, u, &rc);
	if (res == true)
		return a;
	res = VFINDsub(a, u, &rc);
	if (res == true)
		return a + 2;
	else
		return -1;
}

// Used by VFIND
bool Dungeon::VFINDsub(dodBYTE & a, int & u, RowCol * rc)
{
	dodBYTE r, c;
	
	do
	{
		a = VFTTAB[u++];
		if (a == 0xFF)
			return false;
		r = VFTTAB[u++];
		c = VFTTAB[u++];
	} while ( !((r == rc->row) && (c == rc->col)) );
	return true;
}

// Checks for a wall in the given direction
bool Dungeon::TryMove(dodBYTE dir)
{
	int idx;
	dodBYTE a;
	idx = RC2IDX(player.PROW, player.PCOL);
	a = ((MAZLND[idx] >> (dir * 2)) & 3);
	if (a != 3)
		return true;
	else
		return false;
}

// Adds doors
void Dungeon::MAKDOR(dodBYTE * table)
{
	dodBYTE	a_row;
	dodBYTE	a_col;
	int		maz_idx;
	dodBYTE	val;
	dodBYTE	DIR;
	RowCol	ROW;

	do
	{
		do
		{
			a_col = (rng.RANDOM() & 31);
			a_row = (rng.RANDOM() & 31);
			ROW.setRC(a_row, a_col);
			maz_idx = RC2IDX(a_row, a_col);
			val = MAZLND[maz_idx];
		} while (val == 0xFF);
	
		DIR = (rng.RANDOM() & 3);
	} while ((val & MSKTAB[DIR]) != 0);

	MAZLND[maz_idx] |= table[DIR];

	ROW.row += STPTAB[DIR * 2];
	ROW.col += STPTAB[(DIR * 2) + 1];
	DIR += 2;
	DIR &= 3;
	maz_idx = RC2IDX(ROW.row, ROW.col);
	MAZLND[maz_idx] |= table[DIR];
}

// Finds surrounding cells
void Dungeon::FRIEND(RowCol RC)
{
	dodBYTE r3, c3;
	int u = 0;

	for (r3 = RC.row; r3 <= (RC.row+2); ++r3)
	{
		for (c3 = RC.col; c3 <= (RC.col+2); ++c3)
		{
			if (BORDER((r3-1), (c3-1)) == false)
			{
				NEIBOR[u] = 0xFF;
			}
			else
			{
				NEIBOR[u] = MAZLND[RC2IDX((r3-1), (c3-1))];
			}
			++u;
		}
	}
}

// Checks if a step can be taken from the given row/col
// in the given direction
bool Dungeon::STEPOK(dodBYTE R, dodBYTE C, dodBYTE dir)
{
	R += STPTAB[dir * 2];
	C += STPTAB[(dir * 2) + 1];
	if (BORDER(R,C) == false) return false;
	if (MAZLND[RC2IDX(R,C)] == 255) return false;
	return true;
}

//Sets original vertical feature table values.
void Dungeon::SetVFTTABOrig()
{
	VFTTAB[0] = -1;
	VFTTAB[1] = 1;
	VFTTAB[2] = 0;
	VFTTAB[3] = 23;
	VFTTAB[4] = 0;
	VFTTAB[5] = 15;
	VFTTAB[6] = 4;
	VFTTAB[7] = 0;
	VFTTAB[8] = 20;
	VFTTAB[9] = 17;
	VFTTAB[10] = 1;
	VFTTAB[11] = 28;
	VFTTAB[12] = 30;
	VFTTAB[13] = -1;
	VFTTAB[14] = 1;
	VFTTAB[15] = 2;
	VFTTAB[16] = 3;
	VFTTAB[17] = 0;
	VFTTAB[18] = 3;
	VFTTAB[19] = 31;
	VFTTAB[20] = 0;
	VFTTAB[21] = 19;
	VFTTAB[22] = 20;
	VFTTAB[23] = 0;
	VFTTAB[24] = 31;
	VFTTAB[25] = 0;
	VFTTAB[26] = -1;
	VFTTAB[27] = -1;
	VFTTAB[28] = 0;
	VFTTAB[29] = 0;
	VFTTAB[30] = 31;
	VFTTAB[31] = 0;
	VFTTAB[32] = 5;
	VFTTAB[33] = 0;
	VFTTAB[34] = 0;
	VFTTAB[35] = 22;
	VFTTAB[36] = 28;
	VFTTAB[37] = 0;
	VFTTAB[38] = 31;
	VFTTAB[39] = 16;
	VFTTAB[40] = -1;
	VFTTAB[41] = -1;
}

//Sets original maze seed values.
void Dungeon::SetLEVTABOrig()
{
	LEVTAB[0] = 0x73;
	LEVTAB[1] = 0xC7;
	LEVTAB[2] = 0x5D;
	LEVTAB[3] = 0x97;
	LEVTAB[4] = 0xF3;
	LEVTAB[5] = 0x13;
	LEVTAB[6] = 0x87;
}

//Override original vertical feature table values with new ones.
//Will override other level's col & row when during map generation.
void Dungeon::SetVFTTABRandomMap()
{
	VFTTAB[0] = 0;
	VFTTAB[1] = 0;
	VFTTAB[2] = 0;
	VFTTAB[3] = -1;
	VFTTAB[4] = 1;
	VFTTAB[5] = 0;
	VFTTAB[6] = 0;
	VFTTAB[7] = -1;
	VFTTAB[8] = 1;
	VFTTAB[9] = 0;
	VFTTAB[10] = 0;
	VFTTAB[11] = -1;
	VFTTAB[12] = -1;
	VFTTAB[13] = 1;
	VFTTAB[14] = 0;
	VFTTAB[15] = 0;
	VFTTAB[16] = -1;
	VFTTAB[17] = -1;
}

//Override seeds with true random numbers.
void Dungeon::SetLEVTABRandomMap()
{
	srand(GetTickCount());
	LEVTAB[0] = rand() & 255;
	LEVTAB[1] = rand() & 255;
	LEVTAB[2] = rand() & 255;
	LEVTAB[3] = rand() & 255;
	LEVTAB[4] = rand() & 255;
	LEVTAB[5] = rand() & 255;
	LEVTAB[6] = rand() & 255;
}