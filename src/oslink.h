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
// Filename: oslink.h
//
// This class manages the SDL operations, which abstract
// the link to the operating system.  By keeping these
// separate, it will be somewhat easier to change to a
// different library if necessary.

#ifndef OS_LINK_HEADER
#define OS_LINK_HEADER

#ifdef LINUX
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#endif

#include "dod.h"

 // Arbitrary Length of 80, maybe be changed if needed
#define MAX_FILENAME_LENGTH 80
class OS_Link
{
public:
	// Constructor
	OS_Link();

	// Public Interface
	void init();			// main entry point for dod application
	void quitSDL(int code);	// shuts down SDL before exiting
	void process_events();	// used mainly to retrieve keystrokes
	bool main_menu();       // used to implement the meta-menu
	bool saveOptFile(void);

	// Public Data Fields
	int		width;	// actual screen width after video setup
	int		height;	// same for height
	int     volumeLevel; // Volume level

	char	gamefile[50];
	int		gamefileLen;
	char	pathSep[2];
	FILE *	fptr;
	char	confDir[5];
	char	soundDir[6];
	char	savedDir[MAX_FILENAME_LENGTH + 1];
	dodBYTE	keys[256];
	int		keylayout;	// 0 = QWERTY, 1 = Dvorak
	int		keyLen;

	int		audio_rate;
	Uint16	audio_format; 
	int		audio_channels;
	int		audio_buffers;

private:
	// Internal Implementation
	void handle_key_down(SDL_keysym * keysym);	// keyboard handler
	bool menu_return(int, int, menu);		// Used by main menu
	int  menu_list(int x, int y, char *title, char *list[], int listSize);
	void menu_string(char *newString, char *title, int maxLength);
	int  menu_scrollbar(char *title, int min, int max, int current);
	void loadOptFile(void);
	void loadDefaults(void);
	void changeFullScreen(void);
	void changeVideoRes(int newWidth);

	// Data Fields
	int  bpp;	    // bits per pixel
	int  flags;	    // SDL flags
	bool FullScreen;    // FullScreen
	int  creatureRegen; // Creature Regen Speed
};

#endif // OS_LINK_HEADER
