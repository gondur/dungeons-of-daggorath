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
// Filename: dod.h
//
// This file contains common definitions of secondary objects
// used in the game.  It gets included in nearly every other
// file, so the SDL hearders are included here also.

#ifndef DOD_COMMON_HEADER
#define DOD_COMMON_HEADER

// SDL Headers
#ifdef LINUX
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <SDL/SDL_mixer.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_mixer.h>
#endif

// Standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef LINUX
#include <time.h>
#define GetTickCount() time(NULL)
#endif

// The original source code used mostly 8-bit bytes and 16-bit
// words for RAM variable storages.  Many of the operations in
// the source are bit-level so there is a need to have precise
// types where the number of bits is known.  These types should
// be used until/unless it is certain that more abstract types
// (i.e., native C types) will work correctly.
typedef unsigned char	dodBYTE;
typedef unsigned short	dodSHORT;

// This class is used to translate between original 256x192
// coordinates and whichever modern coordinate system is used.
// It sets 512x384 as the default, but when the video system
// gets set up during initialization, these values are set to
// whatever screen resolution is chosen.
//
// This could be expanded later to do screen centering in cases
// where the available (or user-chosen) screen resolution is
// not an exact multiple of 256x192.
//
// The absolute versions are used for most things, and the
// relative versions are used for drawing the font
class Coordinate
{
public:
	Coordinate() : orgWidth(256.0L), orgHeight(192.0L),
				   curWidth(512.0L), curHeight(384.0L)
	{}
	
	// Sets the data members based on screen width
	// (assumes a 4/3 width/height ratio)
	void setCurWH(double W)
	{
		curWidth = ((int) W / (int) 256) * (int) 256;
		curHeight = (curWidth * 0.75L);
		offX = (W - curWidth) / 2;
		offY = (offX * 0.75L);
	}
	
	// Calculates absolute screen X-coordinate based on DoD X-coordinate
	GLfloat newX(double orgX)
	{
		return ((GLfloat) ((orgX) / orgWidth * curWidth) + (GLfloat) offX);
	}
	
	// Calculates relative screen X-coordinate based on DoD X-coordinate
	GLfloat newXa(double orgX)
	{
		return ((GLfloat) ((orgX) / orgWidth * curWidth));
	}
	
	// Calculates absolute screen Y-coordinate based on DoD Y-coordinate
	GLfloat newY(double orgY)
	{
		return ((GLfloat)  ((orgHeight - (orgY)) / orgHeight * curHeight) + (GLfloat) offY);
	}
	
	// Calculates relative screen Y-coordinate based on DoD Y-coordinate
	GLfloat newYa(double orgY)
	{
		return ((GLfloat)  ((orgY) / orgHeight * curHeight));
	}

private:
	double orgWidth;
	double orgHeight;
	double curWidth;
	double curHeight;
	double offX;	// offsets in case the screen dimension is not
	double offY;	// an exact multiple of 256 or 192
};

// This class stores 32x32 row/column values
class RowCol
{
public:
	// Constructors
	RowCol() : row(0), col(0) {}
	RowCol(dodBYTE r, dodBYTE c) : row(r), col(c) {}
	RowCol(int idx) : row(idx/32), col(idx%32) {}

	// Mutator
	void setRC(dodBYTE r, dodBYTE c) { row=r; col=c; }
	
	// Fields
	dodBYTE row;
	dodBYTE col;
};

// The original source has several structured blocks.
// They all have their size as the last field, which is
// probably unnecessary for us.  I've made them into
// classes with all public memebers, and some functions.

// Text Block
//
// This has been entirely modified from the original
// for use in the modern system.
class TXB
{
public:
	TXB() {}

	void SetVals(char * a, int c, int l, int t)
	{
		area = a;
		caret = c;
		len = l;
		top = t;
	}

	// Fields
	char *	area;
	int		caret;
	int		len;
	int		top;
};

// Attack Block
// Creatures and players use the same algorithm
// for attacking and for damage infliction.  These
// values are the common ones used.
class ATB
{
public:
	// Fields
	dodSHORT	P_ATPOW;
	dodBYTE		P_ATMGO;
	dodBYTE		P_ATMGD;
	dodBYTE		P_ATPHO;
	dodBYTE		P_ATPHD;
	dodSHORT	P_ATXX1;
	dodSHORT	P_ATXX2;
	dodSHORT	P_ATDAM;
};

// Creature control block
// Holds all the data for a particular creature.
class CCB
{
public:
	// Constructor
	CCB()
	{ clear(); }

	void clear()
	{
	P_CCPOW = 0;
	P_CCMGO = 0;
	P_CCMGD = 0;
	P_CCPHO = 0;
	P_CCPHD = 0;
	P_CCTMV = 0;
	P_CCTAT = 0;
	P_CCOBJ = -1;
	P_CCDAM = 0;
	P_CCUSE = 0;
	creature_id = 0;
	P_CCDIR = 0;
	P_CCROW = 0;
	P_CCCOL = 0;
	}

	// Fields
	dodSHORT	P_CCPOW;
	dodBYTE		P_CCMGO;
	dodBYTE		P_CCMGD;
	dodBYTE		P_CCPHO;
	dodBYTE		P_CCPHD;
	int			P_CCTMV;
	int			P_CCTAT;
	int			P_CCOBJ;
	dodSHORT	P_CCDAM;
	dodBYTE		P_CCUSE;
	dodBYTE		creature_id;
	dodBYTE		P_CCDIR;
	dodBYTE		P_CCROW;
	dodBYTE		P_CCCOL;
};

// Creature definition block
// Holds the data for a creature type.
class CDB
{
public:
	// Constructors
	CDB(dodSHORT pow, dodBYTE mgo, dodBYTE mgd,
		dodBYTE pho, dodBYTE phd, int tmv,
		int tat)
		: P_CDPOW(pow), P_CDMGO(mgo), P_CDMGD(mgd),
		  P_CDPHO(pho), P_CDPHD(phd), P_CDTMV(tmv),
		  P_CDTAT(tat)
	{}

	CDB()
		: P_CDPOW(0), P_CDMGO(0), P_CDMGD(0),
		  P_CDPHO(0), P_CDPHD(0), P_CDTMV(0),
		  P_CDTAT(0)
	{}

	// Fields
	dodSHORT	P_CDPOW;
	dodBYTE		P_CDMGO;
	dodBYTE		P_CDMGD;
	dodBYTE		P_CDPHO;
	dodBYTE		P_CDPHD;
	int			P_CDTMV;
	int			P_CDTAT;
};

// Object control block
// Hold the data for a particular object.
class OCB
{
public:
	// Default constructor
	OCB()
		{ clear(); }

	void clear()
	{
		P_OCPTR = -1;
		P_OCROW = 0;
		P_OCCOL = 0;
		P_OCLVL = 0;
		P_OCOWN = 0;
		P_OCXX0 = 0;
		P_OCXX1 = 0;
		P_OCXX2 = 0;
		obj_id = 0;
		obj_type = 0;
		obj_reveal_lvl = 0;
		P_OCMGO = 0;
		P_OCPHO = 0;
	}

	// Fields
	int			P_OCPTR;
	dodBYTE		P_OCROW;
	dodBYTE		P_OCCOL;
	dodBYTE		P_OCLVL;
	dodBYTE		P_OCOWN;
	dodSHORT	P_OCXX0;
	dodSHORT	P_OCXX1;
	dodSHORT	P_OCXX2;
	dodBYTE		obj_id;
	dodBYTE		obj_type;
	dodBYTE		obj_reveal_lvl;
	dodBYTE		P_OCMGO;
	dodBYTE		P_OCPHO;
};

// Object definition block
// Used for constructing specific objects.
class ODB
{
public:
	// Constructors
	ODB(dodBYTE cls, dodBYTE rev, dodBYTE mgo,
		dodBYTE pho)
		: P_ODCLS(cls), P_ODREV(rev), P_ODMGO(mgo),
		  P_ODPHO(pho)
	{}

	ODB()
		: P_ODCLS(0), P_ODREV(0), P_ODMGO(0),
		  P_ODPHO(0)
	{}

	// Fields
	dodBYTE		P_ODCLS;
	dodBYTE		P_ODREV;
	dodBYTE		P_ODMGO;
	dodBYTE		P_ODPHO;
};

// Extra definition block
// Holds extra data for torches, rings, and shields:
// torch timers, ring shot counters and incantation indices,
// and shield magical and physical defense values.
class XDB
{
public:
	// Constructors
	XDB(int idx, dodSHORT x0, dodSHORT x1,
		dodSHORT x2)
		: P_OXIDX(idx), P_OXXX0(x0), P_OXXX1(x1),
		  P_OXXX2(x2)
	{}

	XDB()
		: P_OXIDX(-1), P_OXXX0(0), P_OXXX1(0),
		  P_OXXX2(0)
	{}

	// Fields
	int			P_OXIDX;
	dodSHORT	P_OXXX0;
	dodSHORT	P_OXXX1;
	dodSHORT	P_OXXX2;
};

// The new Task class for use in the rewritten
// Scheduler algorithm.  Not all fields are being
// used currently.  They may go away, if the current
// algorthim tests well, otherwise, they may be used
// for increasing the accuracy of the scheduler.
struct Task
{
public:
	int		type;		// One of the seven task types
	int		data;		// Stores creatures ID
	Uint32	frequency;	// in milliseconds
	Uint32	prev_time;	// previous execution timestamp
	Uint32	next_time;	// next scheduled execution timestamp
	long	count;		// number of times executed
	
	Task()
		{ clear(); }

	// Convenience Setter
	void setValues(int t, int d, long f, long p, long n, long c, bool e)
	{
		type = t;
		data = d;
		frequency = f;
		prev_time = p;
		next_time = n;
		count = c;
	}

	void clear()
	{
		type = -1;
		data = -1;
		frequency = 0;
		prev_time = 0;
		next_time = 0;
		count = 0;
	}
};

// This class is a port of Daggorath's custom RNG
class RNG
{
public:
	// Constructor
	RNG()
	{
		carry = 0;
		SEED[0] = 0;
		SEED[1] = 0;
		SEED[2] = 0;
	}

	// Accessors
	dodBYTE RANDOM()
	{
		int x, y;
		dodBYTE a, b;
		carry = 0;
		for (x = 8; x != 0; --x)
		{
			b = 0;
			a = (SEED[2] & 0xE1);
			for (y = 8; y != 0; --y)
			{
				a = lsl(a);
				if (carry != 0)
					++b;
			}
			b = lsr(b);
			SEED[0] = rol(SEED[0]);
			SEED[1] = rol(SEED[1]);
			SEED[2] = rol(SEED[2]);
		}
		return SEED[0];
	}

	dodBYTE getSEED(int idx)
	{
		return SEED[idx];
	}

	// Mutators
	void setSEED(int idx, dodBYTE val)
	{
		SEED[idx] = val;
	}

	void setSEED(dodBYTE val0, dodBYTE val1, dodBYTE val2)
	{
		SEED[0] = val0;
		SEED[1] = val1;
		SEED[2] = val2;
	}

	dodBYTE SEED[3];
	dodBYTE carry;

private:
	// Internal Implementation
	dodBYTE lsl(dodBYTE c)
	{
		carry = (((c & 128) == 128) ? 1 : 0);
		return c << 1;
	}

	dodBYTE lsr(dodBYTE c)
	{
		carry = (((c & 1) == 1) ? 1 : 0);
		return c >> 1;
	}

	dodBYTE rol(dodBYTE c)
	{
		dodBYTE cry;
		cry = (((c & 128) == 128) ? 1 : 0);
		c <<= 1;
		c += carry;
		carry = cry;
		return c;
	}
};

// Not much here at the moment, but it could grow
class Utils
{
public:
	// This could be made into a template someday
	static void LoadFromHex(dodBYTE * b, char * h)
	{
		char hexbuf[3];
		char * end;
		hexbuf[2] = 0;
		int ctr = 0;

		while (*h)
		{
			hexbuf[0] = *h;
			hexbuf[1] = *(h + 1);
			*(b + ctr) = (dodBYTE) strtoul(hexbuf, &end, 16);
			++ctr;
			h += 2;
		}
	}

	// This could be made into a template someday
	static void LoadFromHex(int * b, char * h)
	{
		char hexbuf[3];
		char * end;
		hexbuf[2] = 0;
		int ctr = 0;

		while (*h)
		{
			hexbuf[0] = *h;
			hexbuf[1] = *(h + 1);
			*(b + ctr) = (int) strtoul(hexbuf, &end, 16);
			++ctr;
			h += 2;
		}
	}

	static void LoadFromDecDigit(dodBYTE * b, char * dd)
	{
		while (*dd)
		{
			*b++ = (*dd++ - '0');
		}
	}

	static void LoadFromDecDigit(int * b, char * dd)
	{
		while (*dd)
		{
			*b++ = (*dd++ - '0');
		}
	}

	static Mix_Chunk *LoadSound(char *snd);
};

/*******************************************************************
  Class: menu

  Function: implements the menus

  Note: implementation of commands is at OS_Link::menu_return
*******************************************************************/
#define NUM_MENU   3
#define NUM_FILE   4
#define NUM_CONFIG 12
#define NUM_HELP   3
#define NUM_LENGTH 35

#define FILE_MENU_SWITCH   0
#define CONFIG_MENU_SWITCH 1
#define HELP_MENU_SWITCH   2

#define FILE_MENU_NEW    0
#define FILE_MENU_RETURN 1
#define FILE_MENU_ABORT  2
#define FILE_MENU_EXIT   3

#define CONFIG_MENU_FULL_SCREEN    0
#define CONFIG_MENU_VIDEO_RES      1
#define CONFIG_MENU_GRAPHICS       2
#define CONFIG_MENU_COLOR          3
#define CONFIG_MENU_VOLUME         4
#define CONFIG_MENU_SAVEDIR        5
#define CONFIG_MENU_CREATURE_SPEED 6
#define CONFIG_MENU_REGEN_SPEED    7
#define CONFIG_MENU_RANDOM_MAZE    8
#define CONFIG_MENU_SND_MODE       9
#define CONFIG_MENU_SAVE_OPT      10
#define CONFIG_MENU_DEFAULTS      11

#define HELP_MENU_HOWTOPLAY 0
#define HELP_MENU_LICENSE   1
#define HELP_MENU_ABOUT     2

class menu
{
private:
	char MENU_NAME[NUM_MENU][NUM_LENGTH];
	int MENU_SIZE[NUM_MENU];
	char FILE_MENU[NUM_FILE][NUM_LENGTH];
	char CONFIG[NUM_CONFIG][NUM_LENGTH];
	char HELP[NUM_HELP][NUM_LENGTH];

public:

	 // Constructor -- initalizes strings 
	menu() 
	 { 
	 strncpy(&MENU_NAME[FILE_MENU_SWITCH][0], "FILE", NUM_LENGTH);
	 strncpy(&MENU_NAME[CONFIG_MENU_SWITCH][0], "CONFIGURE", NUM_LENGTH);
	 strncpy(&MENU_NAME[HELP_MENU_SWITCH][0], "HELP", NUM_LENGTH);

	 MENU_SIZE[0] = NUM_FILE;
	 MENU_SIZE[1] = NUM_CONFIG;
	 MENU_SIZE[2] = NUM_HELP;

	 strncpy(&FILE_MENU[FILE_MENU_NEW][0], "START NEW GAME", NUM_LENGTH);
	 strncpy(&FILE_MENU[FILE_MENU_RETURN][0], "RETURN TO GAME", NUM_LENGTH); 
	 strncpy(&FILE_MENU[FILE_MENU_ABORT][0], "ABORT GAME", NUM_LENGTH);
	 strncpy(&FILE_MENU[FILE_MENU_EXIT][0], "EXIT", NUM_LENGTH);

	 strncpy(&CONFIG[CONFIG_MENU_FULL_SCREEN][0], "FULL SCREEN", NUM_LENGTH);
	 strncpy(&CONFIG[CONFIG_MENU_VIDEO_RES][0], "VIDEO RESOLUTION", NUM_LENGTH);
	 strncpy(&CONFIG[CONFIG_MENU_GRAPHICS][0], "GRAPHICS MODE", NUM_LENGTH);
	 strncpy(&CONFIG[CONFIG_MENU_COLOR][0], "COLOR MODE", NUM_LENGTH);
	 strncpy(&CONFIG[CONFIG_MENU_VOLUME][0], "SOUND VOLUME", NUM_LENGTH);
	 strncpy(&CONFIG[CONFIG_MENU_SAVEDIR][0], "SAVE DIRECTORY", NUM_LENGTH);
	 strncpy(&CONFIG[CONFIG_MENU_CREATURE_SPEED][0], "CREATURE SPEED", NUM_LENGTH);
	 strncpy(&CONFIG[CONFIG_MENU_REGEN_SPEED][0], "REGEN SPEED", NUM_LENGTH);
	 strncpy(&CONFIG[CONFIG_MENU_RANDOM_MAZE][0], "RANDOM MAZES", NUM_LENGTH);
	 strncpy(&CONFIG[CONFIG_MENU_SND_MODE][0], "SOUND MODES", NUM_LENGTH);
	 strncpy(&CONFIG[CONFIG_MENU_SAVE_OPT][0], "SAVE CURRENT OPTIONS", NUM_LENGTH); 
	 strncpy(&CONFIG[CONFIG_MENU_DEFAULTS][0], "RESTORE DEFAULTS", NUM_LENGTH);

	 strncpy(&HELP[HELP_MENU_HOWTOPLAY][0], "HOW TO PLAY", NUM_LENGTH); 
	 strncpy(&HELP[HELP_MENU_LICENSE][0], "LICENSE", NUM_LENGTH); 
	 strncpy(&HELP[HELP_MENU_ABOUT][0], "ABOUT DOD", NUM_LENGTH);
	 }

	 // Retrieves the menu name based on an id
	char *getMenuName(int menu_id)
	 {
	 return &MENU_NAME[menu_id][0];
	 }

	 // Retrieves the menu item specified
	char *getMenuItem(int menu_id, int item)
	 {
	 switch(menu_id)
	  {
	  case FILE_MENU_SWITCH:
	   return &FILE_MENU[item][0];
	   break;
          case CONFIG_MENU_SWITCH:
	   return &CONFIG[item][0];
	   break;
          case HELP_MENU_SWITCH:
	   return &HELP[item][0];
	   break;
          }
	 return NULL;
	 }

	 // Returns the size of the specified menu
	int getMenuSize(int menu_id)
	 {
	 return MENU_SIZE[menu_id];
	 }
} ;

#endif // DOD_COMMON_HEADER
