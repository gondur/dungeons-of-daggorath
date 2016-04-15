Daggorath PC-Port
Version 0.3
By Richard Hunerlach
May 16, 2003

Other contributors:
Dan Gendreau: Graphics, sound, and cheat enhancements
Tim Lindner: WAV files
Michael Spencer: Discussion Forum Host
Louis Jordan: Inspirer of the 'Adventure Survivors'

==========

The copyright for Dungeons of Daggorath is still held by the original author, Douglas J. Morgan.
(c) 1982, DynaMicro

==========

Several new commands have been added to control some recent enhancements added by Dan Gendreau. The new in-game commands are as follows:

The game graphics can be changed to one of the following 3 options:
SETOPT GFX NORMAL - sets the game to the original COCO resolution
SETOPT GFX HIRES  - sets the game graphics to dotted line vectors in 1024x768
SETOPT GFX VECTOR - renders the game graphics as gray-scale line vectors in 1024x768

The game sounds can be set to one of the following 2 modes:
SETOPT SND MONO   - mono sound effects
SETOPT SND STEREO - stereo sound effects

The following cheats are available:
SETCHEAT TORCH    - Infinite torch life
SETCHEAT RING     - Infinite ring charges
SETCHEAT CRTSCALE - Creature regens are scaled by the current dungeon level, so you won't ecounter Wraiths, for example, when returning to level 1.
SETCHEAT REVEAL   - the reveal command always succeeds
SETCHEAT ITEMS    - after you die, the next game will start with an iron sword, bronze shield, seer scroll and lunar torch.
SETCHEAT INVULNERABLE - you will not take damage from monsters or ABYE flasks.
SETCHEAT NONE     - turn off all active cheats

The above commands can also be abbreviated:
For example, SETOPT GFX VECTOR can also be typed as: SO G V
And SETCHEAT TORCH can be shortened to: SC T

You'll need to type at least two letters to distinguish between RING and REVEAL, and also between ITEMS and INVULNERABLE.

There is also a new command RESTART, which cannot be abbreviated, for safety reasons.  It will restart the game on the spot, keeping all the current graphics, sound, and cheat settings intact.  This will be especially useful for the ITEMS cheat, since it doesn't take effect until the next time the game restarts.

The game now starts in STEREO mode by default.

The graphics mode still defaults to the original CoCo low-res mode.  This will eventually be added to the configuration file or the application menu, if that ever gets built.

==========

The 'opts.txt' file in the 'conf' directory must include 3 integers [don't use floats].  The first is a percentage of creature speed (100 makes it the same as the 0.2 speed, 200 would double it, 50 would halve it, etc.  A very small number, like 10, will make the creatures move very fast, but their sounds will still slow them down.).  The second number is the frame rate in milliseconds for the turning animation (currently 20 in version 0.2), and the third is the delay in milliseconds for the player movement animation (crrently 25 in version 0.2).  The player movement is not really an animation.  It just takes a half step, then pauses, then goes the rest of the way (or back if there's a wall there).  The the delay is basically the pause after the half step.

There is a fourth line in the 'opts.txt' file, which takes either a zero for QWERTY keyboards, or one for Dvorak.  It is set to zero (QWERTY) by default.

==========

Games are saved in the 'saved' subdirectory.  There is a default filename for saving and loading, so that ZSAVE by itself will create a file called 'game.dod', and ZLOAD by itself will try to load that file.  If you want, however, you can specify your own filenames, but you can only use the uppercase letters A-Z, and only a single word.  So you might want to use LEVELA, since you can't use LEVEL1.

==========

Because this port uses SDL, the LGPL license text is in an 'sdl' subdirectory, along with a link to download the source if you want it.

==========

To run this program, double click on the 'dodwin.exe' file.  Be careful not to move either of the DLL files, or any of the WAV files (found in the 'sound' directory).  Saved games will be stored in the 'saved' directory, and will automatically be given the '.dod' extension.  Games saved with the previous version won't work with this version.

Everything in the game works except that the built-in game demo is not quite timed correctly.  Things are supposed to work just like the original, so if you find any discrepancies, please post them to the forum.  You can no longer climb up holes.  In previous versions I inadvertantly allowed this, but you never could in the orignal game.  After you kill the wizard, however, and before you incant the final ring, you can climb back up to the fourth level if you want.

The Escape key will exit the program.

The game initializes to an 1024 x 768 screen resolution.  The gutters (top, bottom, and sides) on the screen (if any show) are the same color as the background color of the level you are on.

Some unzipping programs have failed to properly create the subdirectories.  So if you have problems, like the sound doesn't work, make sure you have the three main subdirectories: conf (contains opts.txt), saved (contains game.dod), and sound (contains all the WAV files).

Please post comments, suggestions, and especially any problems, to the Developers forum at:

http://mspencer.net/forum/

Thanks in advance to everyone who uses and tests this version, and to everyone who has and will participate in the Daggorath discussions on the forum.

EXTREMELY SPECIAL THANKS to Dan Gendreau for doing these enhancements!!!

Credits:
Original Port and code maintainer:
	Rick Hunerlach

Graphics, sound and game cheat enhancements:
	Dan Gendreau

WAV files:
	Tim Lindner

May 16, 2003
