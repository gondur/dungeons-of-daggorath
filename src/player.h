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
// Filename: player.h
//
// This class manages the Player data

#ifndef DOD_PLAYER_HEADER
#define DOD_PLAYER_HEADER

#include "dod.h"

class Player
{
public:
	// Constructor
	Player();

	// Public Interface
	int			PLAYER();
	bool		HUMAN(dodBYTE c);
	int			HSLOW();
	int			BURNER();
	void		HUPDAT();
	void		setInitialObjects(bool isGame);
	void		PATTK();
	void		PCLIMB();
	void		PDROP();
	void		PEXAM();
	void		PGET();
	void		PINCAN();
	void		PLOOK();
	void		PMOVE();
	void		PPULL();
	void		PREVEA();
	void		PSTOW();
	void		PTURN();
	void		PUSE();
	void		PZLOAD();
	void		PZSAVE();
	bool		PSTEP(dodBYTE dir);
	bool		ATTACK(int AP, int DP, int DD);
	bool		DAMAGE(int AP, int AMO, int APO,
					   int DP, int DMD, int DPD, dodSHORT * DD);
	void		Reset();
	void		LoadSounds();
	void		ShowTurn(dodBYTE A);
	
	// Public Data Members
	dodBYTE		PROW;
	dodBYTE		PCOL;
	dodSHORT	POBJWT;
	ATB			PLRBLK;
	dodSHORT&	PPOW;
	dodBYTE&	PMGO;
	dodBYTE&	PMGD;
	dodBYTE&	PPHO;
	dodBYTE&	PPHD;
	int			PLHAND;
	int			PRHAND;
	dodSHORT&	PDAM;
	dodBYTE		PDIR;
	int			PTORCH;
	dodBYTE		PRLITE;
	dodBYTE		PMLITE;
	dodBYTE		FAINT;
	int			BAGPTR;
	dodBYTE		HEARTF;
	dodBYTE		HEARTC;
	dodBYTE		HEARTR;
	dodBYTE		HEARTS;
	dodBYTE		HBEATF;
	OCB			EMPHND;

	int			turnDelay;
	int			moveDelay;
	int			wizDelay;
	bool		turning;

	Mix_Chunk *	klink;
	Mix_Chunk * thud;
	Mix_Chunk * bang;

private:
};

#endif // DOD_PLAYER_HEADER
