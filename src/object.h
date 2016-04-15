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
// Filename: object.h
//
// This class manages the objects (torches, etc.) in the
// game.

#ifndef DOD_OBJECT_HEADER
#define DOD_OBJECT_HEADER

#include "dod.h"

class Object
{
public:
	// Constructor
	Object();

	// Public Interface
	void	CreateAll();
	int		FNDOBJ();
	void	OBJNAM(int idx);
	int		OFIND(RowCol rc);
	int		OBIRTH(dodBYTE OBJCNT, dodBYTE OBJLVL);
	void	OCBFIL(dodBYTE OBJTYP, int ptr);
	bool	PAROBJ();
	void	Reset();
	void	LoadSounds();
	
	// Public Data Membes
	OCB			OCBLND[72];		// Holds most of the object data
	int			OFINDF;
	dodBYTE		ADJTAB[119];
	dodBYTE		GENTAB[30];
	dodBYTE		OBJTYP;
	dodBYTE		OBJCLS;
	dodBYTE		SPEFLG;
	int			OBJWGT[6];
	int			objChannel;
	Mix_Chunk *	objSound[6];

	// Constants
	enum {

		OBJ_SWORD_WOOD=17,
		OBJ_SWORD_IRON=13,
		OBJ_SWORD_ELVISH=2,
		
		OBJ_SHIELD_LEATHER=16,
		OBJ_SHIELD_BRONZE=11,
		OBJ_SHIELD_MITHRIL=3,

		OBJ_SCROLL_SEER=4,
		OBJ_SCROLL_VISION=7,

		OBJ_RING_JOULE=1,
			OBJ_RING_ENERGY=19, // incanted
		OBJ_RING_RIME=6,
			OBJ_RING_ICE=20,	// incanted
		OBJ_RING_VULCAN=12,
			OBJ_RING_FIRE=21,	// incanted
		OBJ_RING_SUPREME=0,
			OBJ_RING_FINAL=18,	// incanted
		OBJ_RING_GOLD=22,

		OBJ_FLASK_THEWS=5,
		OBJ_FLASK_ABYE=8,
		OBJ_FLASK_HALE=9,
		OBJ_FLASK_EMPTY=23,

		OBJ_TORCH_SOLAR=10,
		OBJ_TORCH_LUNAR=14,
		OBJ_TORCH_PINE=15,
		OBJ_TORCH_DEAD=24,

		OBJT_FLASK=0,
		OBJT_RING=1,
		OBJT_SCROLL=2,
		OBJT_SHIELD=3,
		OBJT_WEAPON=4,
		OBJT_TORCH=5,
	};

	int			OCBPTR;
	int			OFINDP;

private:
	// Data Fields
	ODB			ODBTAB[25];
	XDB			XXXTAB[11];
	dodBYTE		OMXTAB[18];
	dodBYTE		GENVAL[6];
};

#endif // DOD_OBJECT_HEADER
