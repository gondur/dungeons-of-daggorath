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
// Filename: dungeon.h
//
// This class manages the maze data, and includes
// methods related to row/column calculations.

#ifndef DOD_DUNGEON_HEADER
#define DOD_DUNGEON_HEADER

#include "dod.h"

extern RNG rng;	// Inculded here for inlines below

class Dungeon
{
public:
	// Scaffolding Code
	void printMaze();
	void SetLEVTABOrig();
	void SetVFTTABOrig();
	void SetVFTTABRandomMap();
	void SetLEVTABRandomMap();
	void ReseedMap();

	// Constructor
	Dungeon();

	// Public Interface
	void	DGNGEN();
	void	CalcVFI();
	int		RC2IDX(dodBYTE R, dodBYTE C);
	bool	STEPOK(dodBYTE R, dodBYTE C, dodBYTE dir);
	dodBYTE	VFIND(RowCol rc);
	bool	TryMove(dodBYTE dir);

	// Public Data Fields
	dodBYTE		MAZLND[1024];	// The Maze
	dodBYTE		NEIBOR[9];		// The cells around the player
								// Also used to store the walls/doors
								// of a given cell (for the 3D-Viewer)
	dodBYTE		LEVTAB[7];		// The RNG seeds
	RowCol		DROW;
	int			STPTAB[8];
	dodBYTE		VFTTAB[42];
	int			VFTPTR;

	// Constants
	enum {
		N_WALL=0x03,
		E_WALL=0x0c,
		S_WALL=0x30,
		W_WALL=0xc0,
		HF_PAS=0,
		HF_DOR=1,
		HF_SDR=2,
		HF_WAL=3,

		VF_HOLE_UP=0,
		VF_LADDER_UP=1,
		VF_HOLE_DOWN=2,
		VF_LADDER_DOWN=3,
		VF_NULL=255,
	};

private:
	// Private Implementation
	bool	BORDER(dodBYTE R, dodBYTE C);
	void	MAKDOR(dodBYTE * table);
	void	FRIEND(RowCol RC);
	void	RndDstDir(dodBYTE * DIR, dodBYTE * DST);
	bool	VFINDsub(dodBYTE & a, int & u, RowCol * rc);

	// Data Fields
	dodBYTE		MSKTAB[4];
	dodBYTE		DORTAB[4];
	dodBYTE		SDRTAB[4];

	// Scaffolding Data Fields
	char NS[4];
	char EW[4];
};

// Inline Definitions
inline void Dungeon::RndDstDir(dodBYTE * DIR, dodBYTE * DST)
{
	*DIR = (rng.RANDOM() & 3);
	*DST = (rng.RANDOM() & 7) + 1;
}

// Private Implementation
inline bool Dungeon::BORDER(dodBYTE R, dodBYTE C)
{
	if ((R & 224) != 0) return false;
	if ((C & 224) != 0) return false;
	return true;
}

inline int Dungeon::RC2IDX(dodBYTE R, dodBYTE C)
{
	R &= 31;
	C &= 31;
	return (R * 32 + C);
}

#endif // DOD_DUNGEON_HEADER
