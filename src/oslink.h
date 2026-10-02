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

#include <map>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#include "dod.h"

	 // Arbitrary Length of 80, maybe be changed if needed
#define MAX_FILENAME_LENGTH 80

// SDL_mixer 1.2 used MIX_MAX_VOLUME == 128 as its "full volume" constant.
// SDL3_mixer removed the macro and replaced it with a 0.0-1.0 float gain,
// so keep the integer constant for the unmodified volume math in this tree.
#define DOD_MIX_MAX_VOLUME 128

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

	// Audio helpers. SDL3_mixer replaced the channel-index API with track
	// objects and changed the volume scale from int 0-128 to float 0.0-1.0.
	// These wrappers keep every existing call site's original integer 0-128
	// arithmetic intact, so all the wizard fade-volume math and the
	// volumeLevel value in conf/opts.ini keep working unchanged.
	void	playSound(MIX_Track * track, MIX_Audio * audio, int loops);
	void	stopSound(MIX_Track * track);
	bool	isSoundPlaying(MIX_Track * track);
	void	setTrackGain(MIX_Track * track, int volume);   // volume 0-128
	void	setMasterGain(int volume);                     // volume 0-128
	void	setTrackPanning(MIX_Track * track, int left, int right);
	MIX_Track * createTrack();

	// Translates an SDL keycode into the character code the parser expects,
	// honoring the active QWERTY/Dvorak layout. Unmapped keys yield `deflt`
	// (C_SP historically) rather than reading out of bounds.
	dodBYTE	keyToChar(SDL_Keycode keycode, dodBYTE deflt);

	// Public Data Fields
	int		width;	// actual screen width after video setup
	int		height;	// same for height
	int     volumeLevel; // Volume level

	// SDL3 owns these explicitly. SDL 1.2 created the GL context implicitly
	// inside SDL_SetVideoMode(), so these fields did not exist before; they
	// are required because SDL_GL_SwapWindow() takes the window as an argument.
	SDL_Window *	window;
	SDL_GLContext	glContext;

	// SDL3_mixer also requires explicit ownership: there is no global
	// "audio device" any more. MIX_LoadAudio() needs the mixer handle, and
	// tracks are created per-subsystem against it.
	MIX_Mixer *	mixer;

	char	gamefile[50];
	int		gamefileLen;
	char	pathSep[2];
	FILE *	fptr;
	char	confDir[5];
	char	soundDir[6];
	char	savedDir[MAX_FILENAME_LENGTH + 1];
	// Maps SDL_Keycode -> the character the parser should receive. This was
	// a dodBYTE[256] array indexed by keysym->sym, which cannot survive the
	// SDL3 port: SDL_Keycode is a Uint32 and real keycodes reach 0x40000052
	// (SDLK_UP, SDLK_F1, ...), so the array silently overflowed. A map also
	// makes the unmapped-key case explicit instead of an out-of-bounds read.
	std::map<SDL_Keycode, dodBYTE>	keys;
	int		keylayout;	// 0 = QWERTY, 1 = Dvorak

	int		audio_rate;
	Uint16	audio_format; 
	int		audio_channels;
	int		audio_buffers;

private:
	// Internal Implementation
	void handle_key_down(const SDL_KeyboardEvent * key);	// keyboard handler
	bool menu_return(int, int, menu);		// Used by main menu
	int  menu_list(int x, int y, char *title, char *list[], int listSize);
	void menu_string(char *newString, char *title, int maxLength);
	int  menu_scrollbar(char *title, int min, int max, int current);
void	loadOptFile(void);
	void	loadDefaults(void);
	void	changeFullScreen(void);
	void	changeVideoRes(int newWidth);
	void	syncViewport();
	bool	queryDrawableSize(int * w, int * h);

	// Data Fields
	bool FullScreen;    // FullScreen
	int  creatureRegen; // Creature Regen Speed
};

#endif // OS_LINK_HEADER
