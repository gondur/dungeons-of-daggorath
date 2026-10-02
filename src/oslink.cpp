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
// Filename: oslink.cpp
//
// Implementation of OS_Link class

#include <iostream>
#include <fstream>

using namespace std;

#include "oslink.h"
#include "dodgame.h"
#include "viewer.h"
#include "sched.h"
#include "player.h"
#include "dungeon.h"
#include "parser.h"
#include "object.h"
#include "creature.h"
#include "enhanced.h"

extern Creature		creature;
extern Object		object;
extern Dungeon		dungeon;
extern Player		player;
extern Coordinate	crd;
extern Viewer		viewer;
extern dodGame		game;
extern Scheduler	scheduler;
extern Parser		parser;

// Constructor
OS_Link::OS_Link() : width(0), height(0),
					 window(NULL), glContext(NULL), mixer(NULL),
					 audio_rate(44100), audio_format(SDL_AUDIO_S16LE),
					 audio_channels(2), audio_buffers(512),
					 gamefileLen(50), keylayout(0)
{
	printf ("OS_LINK Constructor");
#define MACOSX
#ifdef MACOSX
	strcpy(pathSep,"/");
#else
	strcpy(pathSep,"\\");
#endif

	strcpy(confDir, "conf");
	strcpy(soundDir, "sound");
	strcpy(savedDir, "saved");
	memset(gamefile,0,gamefileLen);
}

// This routine will eventually need updated to allow
// user customization of screen size and resolution.
// It currently asks for an 1024x768 screen size.
// Updated - Now defaults to whatever is in the opts.ini file
// if opts.ini doesn't exist or has invalid or missing values
// uses defaults set by loadDefaults function (1024x768)
void OS_Link::init()
{
	printf("Starting OS_Link::init()\n");
	loadOptFile();

	Uint32 ticks1, ticks2;
	// SDL_INIT_TIMER is gone in SDL3; the timer subsystem is always available
	// and SDL_GetTicks() works without being initialized.
	if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
	{
		fprintf(stderr, "Video initialization failed: %s\n", SDL_GetError());
		quitSDL(1);
	}

	// SDL3_mixer requires an explicit MIX_Init() followed by creating a mixer.
	// SDL_mixer 1.2's Mix_OpenAudio() did both implicitly.
	if(!MIX_Init())
	{
		fprintf(stderr, "Unable to initialize audio!\n");
		quitSDL(1);
	}

	// The audio_rate/format/channels/buffers fields are retained from the
	// original code but SDL3_mixer converts formats itself, so we let it pick
	// a reasonable device default rather than forcing a spec. Note the device
	// must be SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK (0xFFFFFFFF) - passing a
	// literal 0 fails with "Invalid audio device instance ID".
	mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
	if(!mixer)
	{
		fprintf(stderr, "Unable to open audio!\n");
		quitSDL(1);
	}

	// Mix_AllocateChannels(4) pre-created four channels; SDL3_mixer creates
	// tracks one at a time, so create the same four here. The per-subsystem
	// members are assigned from these tracks below.
	MIX_Track * trackHrt   = createTrack();  // was channel 0
	MIX_Track * trackMain  = createTrack();  // was channel 1
	MIX_Track * trackVoice = createTrack();  // was channel 2
	MIX_Track * trackFade  = createTrack();  // was channel 3

	scheduler.hrtChannel  = trackHrt;
	creature.creChannel  = trackMain;
	object.objChannel    = trackMain;
	creature.creChannelv = trackVoice;
	viewer.fadChannel    = trackFade;

	creature.LoadSounds();
	object.LoadSounds();
	scheduler.LoadSounds();
	player.LoadSounds();

	setMasterGain(DOD_MIX_MAX_VOLUME);

	// GL attributes must be set before the window/context is created. The 5-bit
	// RGB request reproduces the original CoCo-era color depth.
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	changeVideoRes(width); // All changing video res code was moved here

	keys.clear();
	// Every key the parser does not explicitly map yields a space, which
	// preserves the original behavior of memset(keys, C_SP) over the table.
	if (keylayout == 0) // QWERTY
	{
		keys[SDLK_A] = 'A';
		keys[SDLK_B] = 'B';
		keys[SDLK_C] = 'C';
		keys[SDLK_D] = 'D';
		keys[SDLK_E] = 'E';
		keys[SDLK_F] = 'F';
		keys[SDLK_G] = 'G';
		keys[SDLK_H] = 'H';
		keys[SDLK_I] = 'I';
		keys[SDLK_J] = 'J';
		keys[SDLK_K] = 'K';
		keys[SDLK_L] = 'L';
		keys[SDLK_M] = 'M';
		keys[SDLK_N] = 'N';
		keys[SDLK_O] = 'O';
		keys[SDLK_P] = 'P';
		keys[SDLK_Q] = 'Q';
		keys[SDLK_R] = 'R';
		keys[SDLK_S] = 'S';
		keys[SDLK_T] = 'T';
		keys[SDLK_U] = 'U';
		keys[SDLK_V] = 'V';
		keys[SDLK_W] = 'W';
		keys[SDLK_X] = 'X';
		keys[SDLK_Y] = 'Y';
		keys[SDLK_Z] = 'Z';
		keys[SDLK_BACKSPACE] = parser.C_BS;
		keys[SDLK_RETURN] = parser.C_CR;
		keys[SDLK_SPACE] = parser.C_SP;
	}
	else if (keylayout == 1) // Dvorak
	{
		keys[SDLK_A] = 'A';
		keys[SDLK_N] = 'B';
		keys[SDLK_I] = 'C';
		keys[SDLK_H] = 'D';
		keys[SDLK_D] = 'E';
		keys[SDLK_Y] = 'F';
		keys[SDLK_U] = 'G';
		keys[SDLK_J] = 'H';
		keys[SDLK_G] = 'I';
		keys[SDLK_C] = 'J';
		keys[SDLK_V] = 'K';
		keys[SDLK_P] = 'L';
		keys[SDLK_M] = 'M';
		keys[SDLK_L] = 'N';
		keys[SDLK_S] = 'O';
		keys[SDLK_R] = 'P';
		keys[SDLK_X] = 'Q';
		keys[SDLK_O] = 'R';
		keys[SDLK_SEMICOLON] = 'S';
		keys[SDLK_K] = 'T';
		keys[SDLK_F] = 'U';
		keys[SDLK_PERIOD] = 'V';
		keys[SDLK_COMMA] = 'W';
		keys[SDLK_B] = 'X';
		keys[SDLK_T] = 'Y';
		keys[SDLK_SLASH] = 'Z';
		keys[SDLK_BACKSPACE] = parser.C_BS;
		keys[SDLK_RETURN] = parser.C_CR;
		keys[SDLK_SPACE] = parser.C_SP;
	}

	// Delay to wait for monitor to change modes if necessary
	// This ought to be made more intelligent
	ticks1 = SDL_GetTicks();
	do
	{
		ticks2 = SDL_GetTicks();
	} while (ticks2 < ticks1 + 2500);
	game.COMINI();
 	while (true)
	{
		scheduler.SCHED();
		if (scheduler.ZFLAG == 0xFF)
		{
			game.LoadGame();
			scheduler.ZFLAG = 0;
		}
		else
		{
			if (game.AUTFLG)
			{
				if (game.demoRestart)
				{
					// Restart demo
					game.hasWon = false;
					game.DEMOPTR = 0;
					object.Reset();
					creature.Reset();
					parser.Reset();
					player.Reset();
					scheduler.Reset();
					viewer.Reset();
					dungeon.VFTPTR = 0;
					game.COMINI();
				}
				else
				{
					// Start new game
					game.AUTFLG = false;
					game.Restart();
				}
			}
			else
			{
				game.Restart();
			}
		}
	}
	printf("Init complete\n");
}

// Used to check for keystrokes and application termination
void OS_Link::process_events()
{
	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
		case SDL_EVENT_KEY_DOWN:
			handle_key_down(&event.key);
			break;
		case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
			syncViewport();
			break;
		case SDL_EVENT_QUIT:
			quitSDL(0);
			break;
		}
	}
}

// Determines the real pixel size of the area we are drawing into.
//
// This is subtle and was the cause of the "renders into one corner" bug. In
// fullscreen the compositor owns the final size and ignores the resolution we
// asked for, so SDL keeps reporting the *requested* size (1024x768) even
// though the real drawable is the whole display (e.g. 1920x1080). Asking
// SDL_GetWindowSizeInPixels() therefore gives the wrong answer for exactly the
// case where it matters. In windowed mode the window is not resized, so the
// window size is correct and is what we want.
//
// Returns false if no usable size could be determined.
bool OS_Link::queryDrawableSize(int * w, int * h)
{
	const SDL_DisplayMode * mode;

	if(!window)
	{
		return false;
	}

	if(FullScreen)
	{
		SDL_DisplayID displayID = SDL_GetDisplayForWindow(window);

		if(displayID == 0)
		{
			displayID = SDL_GetPrimaryDisplay();
		}
		mode = SDL_GetCurrentDisplayMode(displayID);
		if(mode && mode->w > 0 && mode->h > 0)
		{
			*w = mode->w;
			*h = mode->h;
			return true;
		}
		return false;
	}

	return SDL_GetWindowSizeInPixels(window, w, h) && *w > 0 && *h > 0;
}

// Re-reads the real drawable size and rebuilds the viewport, projection and
// centering offsets. Called after any size change so the picture never
// straddles the window edge.
void OS_Link::syncViewport()
{
	int w = 0, h = 0;

	if(!window || !glContext)
	{
		return;
	}
	if(!queryDrawableSize(&w, &h))
	{
		return;
	}
	if(w == width && h == height)
	{
		return;
	}

	width  = w;
	height = h;
	crd.setCurWH((double) width, (double) height);

	SDL_GL_MakeCurrent(window, glContext);
	viewer.setup_opengl();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// Quits application
void OS_Link::quitSDL(int code)
{
	if(mixer)
	{
		MIX_DestroyMixer(mixer);
		mixer = NULL;
	}
	MIX_Quit();
	SDL_Quit();
	exit(code);
}

// Translates an SDL keycode into the character code the parser expects.
dodBYTE OS_Link::keyToChar(SDL_Keycode keycode, dodBYTE deflt)
{
	std::map<SDL_Keycode, dodBYTE>::const_iterator it;

	it = keys.find(keycode);
	if(it == keys.end())
	{
		return deflt;
	}
	return it->second;
}

// Audio wrappers. These adapt the SDL_mixer 1.2 channel-based API this game
// was written against to the SDL3_mixer track-based API, while preserving the
// original integer 0-128 volume semantics at every call site.
void OS_Link::playSound(MIX_Track * track, MIX_Audio * audio, int loops)
{
	SDL_PropertiesID options;

	if(!track || !audio)
	{
		return;
	}

	MIX_SetTrackAudio(track, audio);

	// loops == 0 means "play once" in both APIs; -1 means loop forever.
	options = SDL_CreateProperties();
	SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOPS_NUMBER, loops);
	MIX_PlayTrack(track, options);
	SDL_DestroyProperties(options);
}

void OS_Link::stopSound(MIX_Track * track)
{
	if(!track)
	{
		return;
	}
	MIX_StopTrack(track, 0);
}

bool OS_Link::isSoundPlaying(MIX_Track * track)
{
	if(!track)
	{
		return false;
	}
	return MIX_TrackPlaying(track);
}

// Converts the historical int 0-128 volume into SDL3_mixer's 0.0-1.0 gain.
void OS_Link::setTrackGain(MIX_Track * track, int volume)
{
	if(!track)
	{
		return;
	}
	MIX_SetTrackGain(track, (float) volume / (float) DOD_MIX_MAX_VOLUME);
}

// setMasterGain() sets the volume of *every* channel; the equivalent in
// SDL3_mixer is the mixer's master gain.
void OS_Link::setMasterGain(int volume)
{
	if(!mixer)
	{
		return;
	}
	MIX_SetMixerGain(mixer, (float) volume / (float) DOD_MIX_MAX_VOLUME);
}

void OS_Link::setTrackPanning(MIX_Track * track, int left, int right)
{
	MIX_StereoGains gains;

	if(!track)
	{
		return;
	}
	gains.left  = (float) left  / 255.0f;
	gains.right = (float) right / 255.0f;
	MIX_SetTrackStereo(track, &gains);
}

MIX_Track * OS_Link::createTrack()
{
	if(!mixer)
	{
		return NULL;
	}
	return MIX_CreateTrack(mixer);
}

// Processes key strokes.
void OS_Link::handle_key_down(const SDL_KeyboardEvent * key)
{
	dodBYTE c;
	if (viewer.display_mode == Viewer::MODE_MAP)
	{
		switch(key->key)
		{
		case SDLK_ESCAPE:
			main_menu();
			break;
		default:
			viewer.display_mode = Viewer::MODE_3D;
			--viewer.UPDATE;
			parser.KBDPUT(32); // This is a (necessary ???) hack.
			break;
		}

	}
	else
	{
		switch(key->key)
		{
		case SDLK_RSHIFT:
		case SDLK_LSHIFT:
		case SDLK_RCTRL:
		case SDLK_LCTRL:
		case SDLK_RALT:
		case SDLK_LALT:
		case SDLK_RMETA:
		case SDLK_LMETA:
		case SDLK_MODE:
		case SDLK_NUMLOCKCLEAR:
		case SDLK_CAPSLOCK:
		case SDLK_SCROLLLOCK:
			// ignore these keys
			return;

		case SDLK_ESCAPE:
			main_menu();   // Enter the meta-menu routine
                        return;

//		case SDLK_EXCLAIM: c = '!'; break;
//		case SDLK_LESS: c = '<'; break;
//		case SDLK_GREATER: c = '>'; break;
//		case SDLK_QUESTION: c = '?'; break;
//		case SDLK_UNDERSCORE: c = '_'; break;
//		case SDLK_PERIOD: c = '.'; break;
//		case SDLK_LEFTBRACKET: c = '{'; break;
//		case SDLK_RIGHTBRACKET: c = '}'; break;

		default:
			c = keyToChar(key->key, parser.C_SP);
			break;
		}
		parser.KBDPUT(c);
	}
}

/*********************************************************
  Member: main_menu

  Function: Implements the menu, and dispatches commands

  Returns:  true  - If a new game is started
            false - otherwise
*********************************************************/
bool OS_Link::main_menu()
{
 bool end = false;
 static int row = 0, col = 0;
 static menu mainMenu;

 scheduler.pause(true);
 viewer.drawMenu(mainMenu, col, row);
 
 do
   {
   SDL_Event event;
   while(SDL_PollEvent(&event))
   {
   switch(event.type)
     {
     case SDL_EVENT_KEY_DOWN:
      switch(event.key.key)
        {
        case SDLK_RETURN:
	 end = menu_return(col, row, mainMenu);

	   // Used for Wizard fade functions, if it's a new game, it will trigger a key press
	 if(col == FILE_MENU_SWITCH && row == FILE_MENU_NEW)
	   return true;

         break;
        case SDLK_UP:
	 (row < 1) ? row = mainMenu.getMenuSize(col) - 1 : row--;
         break;
        case SDLK_DOWN:
	 (row > mainMenu.getMenuSize(col) - 2) ? row = 0 : row++;
         break;
        case SDLK_LEFT:
	 (col < 1) ? col = NUM_MENU - 1 : col--;
	 row = 0;
         break;
        case SDLK_RIGHT:
	 (col > 1) ? col = 0 : col++;
	 row = 0;
	 break;
        case SDLK_ESCAPE:
         end = true;
	 break;
        default:
	 break;
        }
      viewer.drawMenu(mainMenu, col, row);
      break;
     case SDL_EVENT_QUIT:
      quitSDL(0);
      break;
      }
     }
  } while(!end);

  scheduler.pause(false);

  return false;
}

/* Function to process menu commands
*
*  Returns:  false - if menu should be redrawn
*            true  - otherwise */

bool OS_Link::menu_return(int menu_id, int item, menu Menu)
{
switch(menu_id)
 {
  // File Menu
 case FILE_MENU_SWITCH:
 switch(item)
  {
  case FILE_MENU_NEW:
   //New Game
   scheduler.pause(false);  // Needed so that the game can be paused again later

   if(!game.AUTFLG)
    {
    game.hasWon = true;
    game.demoRestart = false;
    }
   return true;
   break;

  case FILE_MENU_RETURN:
   //Return
   return true;
   break;

  case FILE_MENU_ABORT:
   //Abort (Restart)
   scheduler.pause(false);  // Needed so that the game can be paused again later

   if(!game.AUTFLG)
    {
    game.AUTFLG = true;
    game.hasWon = true;
    game.demoRestart = true;
    }
   return true;
   break;

  case FILE_MENU_EXIT:
   //Exit
   quitSDL(0);
  }

  // Configuration Menu
 case CONFIG_MENU_SWITCH:
 switch(item)
  {
  case CONFIG_MENU_FULL_SCREEN:
   //Full Screen
   {
   char *menuList[]={ "ON", "OFF" };

   switch(menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 2))
    {
    case 0:
      if(!FullScreen)
       changeFullScreen();
      break;

    case 1:
      if(FullScreen)
       changeFullScreen();
      break;

    default:
      return false;
      break;
    }
   }
   return false;
   break;

  case CONFIG_MENU_VIDEO_RES:
   // Video Res
   {
   char *menuList[]={ "640X480", "800X600", "1024X768", "1280X1024" };

   switch(menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 4))
    {
    case 0:
     changeVideoRes(640);
     break;

    case 1:
     changeVideoRes(800);
     break;

    case 2:
     changeVideoRes(1024);
     break;

    case 3:
     changeVideoRes(1280);
     break;

    default:
     return false;
     break;
    }
   }
   return false;
   break;

  case CONFIG_MENU_GRAPHICS:
    // Graphics (Normal /HIRes / vect)
   {
   char *menuList[]={ "NORMAL GRAPHICS", "HIRES GRAPHICS", "VECTOR GRAPHICS" };

   switch(menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 3))
    {
    case 0:
     g_options &= ~(OPT_VECTOR|OPT_HIRES);
     break;

    case 1:
     g_options &= ~(OPT_VECTOR);
     g_options |= OPT_HIRES;
     break;

    case 2:
     g_options &= ~(OPT_HIRES);
     g_options |= OPT_VECTOR;
     break;

    default:
     return false;
     break;
    }
   }
   return true;
   break;

  case CONFIG_MENU_COLOR:
   // Color (B&W / Art. / Full)
   {
   char *menuList[]={ "BLACK WHITE" };

   switch(menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 1))
    {
    default:
     return false;
     break;
    }
   }
   return true;
   break;

  case CONFIG_MENU_VOLUME:
   // Volume
   {
   volumeLevel = menu_scrollbar("VOLUME LEVEL", 0, 128, volumeLevel);
   setMasterGain(volumeLevel);
   }
   return false;
   break;

  case CONFIG_MENU_SAVEDIR:
   // Save Dir
   {
   char *menuList[]={ "EDIT OPTS.INI FILE" };

   switch(menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 1))
    {
    default:
     return false;
     break;
    }
   }
   return false;
   break;

  case CONFIG_MENU_CREATURE_SPEED:
    // Creature Speed
   {
   char *menuList[2] = {"COCO", "CUSTOM"};

   switch(menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 2))
    {
    case 0:
     //Coco Speed
     creature.creSpeedMul = 200;
     creature.UpdateCreSpeed();
     break;
    case 1:
     //Custom Speed
     creature.creSpeedMul = menu_scrollbar("CREATURE SPEED", 50, 200, volumeLevel);
     creature.UpdateCreSpeed();
     return false;
     break;

    default:
     return false;
     break;
    }
   }
   break;

  case CONFIG_MENU_REGEN_SPEED:
   // Regen Speed
   {
   char *menuList[] = { "5 MINUTES", "3 MINUTES", "1 MINUTE" };

   switch(menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 3))
     {
     case 0:
      creatureRegen = 5;
      scheduler.updateCreatureRegen(creatureRegen);
      break;
     case 1:
      creatureRegen = 3;
      scheduler.updateCreatureRegen(creatureRegen);
      break;
     case 2:
      creatureRegen = 1;
      scheduler.updateCreatureRegen(creatureRegen);
      break;
     default:
      return false;
      break;
     }
   }
   return true;
   break;

  case CONFIG_MENU_RANDOM_MAZE:
   // Random Mazes
   {
   char *menuList[]={ "ON", "OFF" };

   switch(menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 2))
    {
	case 0:
		game.RandomMaze = true;
		break;
	case 1:
		game.RandomMaze = false;
	    break;
    default:
     return false;
     break;
    }
   }
   return false;
   break;

  case CONFIG_MENU_SND_MODE:
   // Sound Style (Sync, Stereo)
   {
   char *menuList[2] = {"STEREO", "MONO"};

   switch(menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 2))
    {
    case 0:
    g_options |= OPT_STEREO;
    break;

    case 1:
    g_options &= ~OPT_STEREO;
    break;

    default:
    return false;
    break;
    }
   }
   break;

  case CONFIG_MENU_SAVE_OPT:
   saveOptFile();
   return true;
   break;

  case CONFIG_MENU_DEFAULTS:
   loadDefaults();
   changeVideoRes(width);
   return true;
   break;
  }

  // Help menu
 case HELP_MENU_SWITCH:
 switch(item)
  {
  case HELP_MENU_HOWTOPLAY:
   // How to play
   {
   char *menuList[]={ "SEE FILE HOWTOPLAY.TXT" };

   menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 1);
   }
   return false;
   break;

  case HELP_MENU_LICENSE:
   // License
   {
   char *menuList[]={ "SEE FILE README.TXT" };

   menu_list(menu_id * 5, item + 2, Menu.getMenuItem(menu_id, item), menuList, 1);
   }
   return false;
   break;

  case HELP_MENU_ABOUT:
   // About
   {
   SDL_Event event;

   viewer.aboutBox();
   while(true)
    {
    while(SDL_PollEvent(&event))
     {
     switch(event.type)
      {
      case SDL_EVENT_KEY_DOWN:
       return false;
       break;
      case SDL_EVENT_QUIT:
       quitSDL(0);  // Quits SDL
       break;
      }
     }
    }
   return false;
   }
   break;
  }
 }
 return true;
}

/*****************************************************************************
*  Function used to draw a list, move among that list, and return the item selected
*
*  Arguments: x        - The top-left x-coordinate to draw list at
*             y        - The top-left y-coordinate to draw list at
*             title    - The title of the list
*             list     - An array of strings (the list to be chosen from
*             listSize - The size of the array
******************************************************************************/
int OS_Link::menu_list(int x, int y, char *title, char *list[], int listSize)
 {
 int currentChoice = 0;

 while(true)
   {
   viewer.drawMenuList(x, y, title, list, listSize, currentChoice);
   SDL_Event event;
   while(SDL_PollEvent(&event))
   {
   switch(event.type)
     {
     case SDL_EVENT_KEY_DOWN:
      switch(event.key.key)
        {
        case SDLK_RETURN:
         return(currentChoice);
         break;

        case SDLK_UP:
	 (currentChoice < 1) ? currentChoice = listSize - 1 : currentChoice--;
         break;

        case SDLK_DOWN:
	 (currentChoice > listSize - 2) ? currentChoice = 0 : currentChoice++;
         break;

        case SDLK_ESCAPE:
	 return(-1);
	 break;

        default:
	 break;
        }
      break;
     case SDL_EVENT_QUIT:
      quitSDL(0);
      break;
      }
     }
  } // End of while loop

 return(-1);
 }

/*****************************************************************************
*  Function used to draw a scrollbar, and return the value
*
*  Arguments: title     - The title of the entry
*             min       - The minimum value the scroll bar can take
*             max       - The maximum value the scroll bar can take
*             current   - The current position of the scrollbar
*
*  Returns: The value the user entered, or if they hit escape, the original
*           value.
******************************************************************************/
int OS_Link::menu_scrollbar(char *title, int min, int max, int current)
 {
 int oldvalue  = current; //Save the old value in case the user escapes
 int increment = (max - min) / 31;  // 31 is the number of columns

   // Calculate a relative max and min and corresponding current number
 int newMax    = increment * 31;
 int newMin    = 0;
     current   = current - min;

 viewer.drawMenuScrollbar(title, (current - newMin) / increment);

 while(true)
   {
   SDL_Event event;

   while(SDL_PollEvent(&event))
    {
    switch(event.type)
      {
      case SDL_EVENT_KEY_DOWN:
       switch(event.key.key)
        {
        case SDLK_RETURN:
         return(current + min);  // Readjust back to absolute value
         break;

        case SDLK_LEFT:
	 (current > newMin) ? current -= increment : current = newMin;
         break;

        case SDLK_RIGHT:
	 (current < newMax) ? current += increment : current = newMax;
         break;

        case SDLK_ESCAPE:
	 return(oldvalue);
	 break;

        default:
	 break;
	}
       viewer.drawMenuScrollbar(title, (current - newMin) / increment);
       break;
      case SDL_EVENT_QUIT:
       quitSDL(0);
       break;
      }
    }
   }
 }

/*****************************************************************************
*  Function used to draw a box for a string entry, then return it
*
*  Arguments: newString - The string to be returned
*             title     - The title of the entry
*             maxLength - The maximum size of the entry
******************************************************************************/
void OS_Link::menu_string(char *newString, char *title, int maxLength)
 {
 *newString = '\0';
 viewer.drawMenuStringTitle(title);
 viewer.drawMenuString(newString);

 while(true)
   {
   SDL_Event event;

   while(SDL_PollEvent(&event))
   {
   switch(event.type)
     {
     case SDL_EVENT_KEY_DOWN:
      switch(event.key.key)
        {
        case SDLK_RETURN:
         return;
         break;

        case SDLK_RSHIFT:
        case SDLK_LSHIFT:
        case SDLK_RCTRL:
        case SDLK_LCTRL:
        case SDLK_RALT:
        case SDLK_LALT:
        case SDLK_RMETA:
        case SDLK_LMETA:
        case SDLK_MODE:
        case SDLK_NUMLOCKCLEAR:
        case SDLK_CAPSLOCK:
        case SDLK_SCROLLLOCK:
        case SDLK_UP:
        case SDLK_DOWN:
          // ignore these keys
          break;

        case SDLK_BACKSPACE:
	case SDLK_LEFT:
	 if(strlen(newString) > 0)
	  {
	  *(newString + strlen(newString) - 1) = '\0';
          viewer.drawMenuStringTitle(title);  // Update with the new word
          viewer.drawMenuString(newString);
	  }
         break;

        case SDLK_ESCAPE:
         *(newString) = '\0';
	 return;
	 break;

        default:
	 if(strlen(newString) < maxLength)
	  {
	  *(newString + strlen(newString) + 1) = '\0';
	  *(newString + strlen(newString)) = keyToChar(event.key.key, parser.C_SP);
          viewer.drawMenuStringTitle(title);  // Update with the new word
          viewer.drawMenuString(newString);
	  }
	 break;
        }
      break;
     case SDL_EVENT_QUIT:
      quitSDL(0);
      break;
      }
     }
  } // End of while loop
 }

/******************************************************************************
*  Function used to load & parse options file
*
*  Arguments: None
******************************************************************************/
void OS_Link::loadOptFile(void)
 {
 char     inputString[80];
 char     fn[20];
 int      in;
 ifstream fin;
 char *   breakPoint;

 loadDefaults(); // In case some variables aren't in the opts file, and if no file exists

 sprintf(fn, "%s%s%s", confDir, pathSep, "opts.ini");
	
 fin.open(fn);
 if (!fin)
 {
 return;
 }

 fin >> inputString;
 while(fin)
  {
  breakPoint = strchr(inputString, '=');

   // Ignore strings that have no equals, or are only an equals, or have no end
  if(breakPoint || breakPoint == inputString || breakPoint == (inputString + strlen(inputString) - 1))
   {
   *(breakPoint) = '\0';
   breakPoint++;

   if(!strcmp(inputString, "creatureSpeed"))
    {
    if (1==sscanf(breakPoint, "%d", &in)) creature.creSpeedMul = in;
    }
   else if(!strcmp(inputString, "turnDelay"))
    {
    if (1==sscanf(breakPoint, "%d", &in)) player.turnDelay = in;
    }
   else if(!strcmp(inputString, "moveDelay"))
    {
    if (1==sscanf(breakPoint, "%d", &in)) player.moveDelay = in;
    }
   else if(!strcmp(inputString, "keylayout"))
    {
    if (1==sscanf(breakPoint, "%d", &in)) keylayout = in;
    }
   else if(!strcmp(inputString, "graphicsMode"))
    {
    if(!strcmp(breakPoint, "NORMAL"))
     g_options &= ~(OPT_VECTOR|OPT_HIRES);
    else if(!strcmp(breakPoint, "HIRES"))
     {
     g_options &= ~(OPT_VECTOR);
     g_options |= OPT_HIRES;
     }
    else if(!strcmp(breakPoint, "VECTOR"))
     {
     g_options &= ~(OPT_HIRES);
     g_options |= OPT_VECTOR;
     }
    }
   else if(!strcmp(inputString, "stereoMode"))
    {
    if(!strcmp(breakPoint, "STEREO"))
     g_options |= OPT_STEREO;
    else if(!strcmp(breakPoint, "MONO"))
     g_options &= ~OPT_STEREO;
    }
   else if(!strcmp(inputString, "volumeLevel"))
    {
    if (1==sscanf(breakPoint, "%d", &in)) volumeLevel = in;
    }
   else if(!strcmp(inputString, "saveDirectory"))
    {
    strncpy(savedDir, "saved", MAX_FILENAME_LENGTH);
    }
   else if(!strcmp(inputString, "fullScreen"))
    {
    if (1==sscanf(breakPoint, "%d", &in)) FullScreen = in;
    }
   else if(!strcmp(inputString, "screenWidth"))
    {
    if (1==sscanf(breakPoint, "%d", &in)) width = in;
    }
   else if(!strcmp(inputString, "creatureRegen"))
    {
    if (1==sscanf(breakPoint, "%d", &in)) creatureRegen = in;
    }
   else if(!strcmp(inputString, "RandomMaze"))
    {
    if (1==sscanf(breakPoint, "%d", &in)) game.RandomMaze = in;
    }
   else if(!strcmp(inputString, "ShieldFix"))
    {
    if (1==sscanf(breakPoint, "%d", &in)) game.ShieldFix = in;
    }
   else if(!strcmp(inputString, "VisionScroll"))
    {
    if (1==sscanf(breakPoint, "%d", &in)) game.VisionScroll = in;
    }
   else if(!strcmp(inputString, "CreaturesIgnoreObjects"))
    {
    if (1==sscanf(breakPoint, "%d", &in)) game.CreaturesIgnoreObjects = in;
    }
   else if(!strcmp(inputString, "CreaturesInstaRegen"))
    {
    if (1==sscanf(breakPoint, "%d", &in)) game.CreaturesInstaRegen = in;
    }
   else if(!strcmp(inputString, "MarkDoorsOnScrollMaps"))
    {
    if (1==sscanf(breakPoint, "%d", &in)) game.MarkDoorsOnScrollMaps = in;
    }
   }

  fin >> inputString;
  }

 fin.close();
 scheduler.updateCreatureRegen(creatureRegen);
 creature.UpdateCreSpeed();
 }

/******************************************************************************
*  Function used to save the options file from current settings
*
*  Arguments: None
*
*  Returns:   true - file saved successfully, false - file not saved
******************************************************************************/
bool OS_Link::saveOptFile(void)
 {
 ofstream fout;
 char     fn[MAX_FILENAME_LENGTH];

 sprintf(fn, "%s%s%s", confDir, pathSep, "opts.ini");

 fout.open(fn);
 if(!fout)
   return false;

 fout << "creatureSpeed=" << creature.creSpeedMul << endl;
 fout << "turnDelay=" << player.turnDelay << endl;
 fout << "moveDelay=" << player.moveDelay << endl;
 fout << "keylayout=" << keylayout << endl;
 fout << "volumeLevel=" << volumeLevel << endl;
 fout << "saveDirectory=" << savedDir << endl;
 fout << "fullScreen=" << FullScreen << endl;
 fout << "screenWidth=" << width << endl;
 fout << "creatureRegen=" << creatureRegen << endl;

 fout << "graphicsMode=";
 if(g_options & OPT_VECTOR)
   fout << "VECTOR" << endl;
 else if(g_options & OPT_HIRES)
   fout << "HIRES" << endl;
 else
   fout << "NORMAL" << endl;

 fout << "stereoMode=";
 if(g_options & OPT_STEREO)
   fout << "STEREO" << endl;
 else
   fout << "MONO" << endl;

 fout << "RandomMaze=" << game.RandomMaze << endl;
 fout << "ShieldFix=" << game.ShieldFix << endl;
 fout << "VisionScroll=" << game.VisionScroll << endl;
 fout << "CreaturesIgnoreObjects=" << game.CreaturesIgnoreObjects << endl;
 fout << "CreaturesInstaRegen=" << game.CreaturesInstaRegen << endl;
 fout << "MarkDoorsOnScrollMaps=" << game.MarkDoorsOnScrollMaps << endl;

 fout.close();

 return true;
 }

/******************************************************************************
*  Function used to load the options file from current settings
*
*  Arguments: None
******************************************************************************/
void OS_Link::loadDefaults(void)
 {
 player.turnDelay = 37;
 player.moveDelay = 500;
 keylayout   = 0;
 volumeLevel = DOD_MIX_MAX_VOLUME;
 creature.creSpeedMul = 200;
 creature.UpdateCreSpeed();
 strcpy(savedDir, "saved");
 FullScreen = true;
 width = 1024;
 creatureRegen = 5;
 scheduler.updateCreatureRegen(creatureRegen);

 g_options &= ~(OPT_VECTOR|OPT_HIRES);
 g_options |= OPT_STEREO;
 }

/******************************************************************************
*  Function used to swap fullscreen mode
*
*  Arguments: None
******************************************************************************/
void OS_Link::changeFullScreen(void)
 {
 FullScreen = !FullScreen;
 changeVideoRes(width);
 }

/******************************************************************************
*  Function used to change the video resolution
*
*  Arguments: newWidth - The screen width to change to
******************************************************************************/
void OS_Link::changeVideoRes(int newWidth)
 {
 int newHeight;
 SDL_WindowFlags wflags;

 newHeight = (int) (newWidth * 0.75);

 wflags = SDL_WINDOW_OPENGL;
 if(FullScreen)
  {
  wflags |= SDL_WINDOW_FULLSCREEN;
  SDL_ShowCursor();
  SDL_HideCursor();
  }
 else
  {
  // Deliberately NOT SDL_WINDOW_RESIZABLE. The window should be exactly the
  // selected game resolution; letting the compositor resize it decouples the
  // GL viewport from the window and the picture ends up drawn partly outside.
  SDL_ShowCursor();
  }

 // Recreate the window rather than resizing it. SDL3 has no SDL_SetVideoMode()
 // equivalent, and the GL context is invalidated when the window is destroyed,
 // so this is the only correct place to (re)establish both.
 if(window)
  {
  SDL_GL_DestroyContext(glContext);
  SDL_DestroyWindow(window);
  window = NULL;
  glContext = NULL;
  }

 window = SDL_CreateWindow("Dungeons of Daggorath",
                           newWidth, newHeight, wflags);
 if(!window)
  {
  fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
  exit(1);
  }

 glContext = SDL_GL_CreateContext(window);
 if(!glContext)
  {
  fprintf(stderr, "GL context creation failed: %s\n", SDL_GetError());
  exit(1);
  }

 SDL_GL_MakeCurrent(window, glContext);

 // Use the ACTUAL drawable size, not the size we asked for. In fullscreen the
 // compositor owns the final size; see queryDrawableSize() for why asking SDL
 // directly would be wrong. Both the GL viewport and the centering offsets must
 // be derived from this same real size.
 if(!queryDrawableSize(&width, &height))
  {
   width  = newWidth;
   height = newHeight;
  }

 crd.setCurWH((double) width, (double) height);

 viewer.setup_opengl();
 glMatrixMode(GL_MODELVIEW);
 glLoadIdentity();
 }
