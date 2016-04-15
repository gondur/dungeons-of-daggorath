Daggorath PC-Port
Version 0.2.2
By Richard Hunerlach
November 14, 2002

==========

The copyright for Dungeons of Daggorath is still held by the original author, Douglas J. Morgan.
(c) 1982, DynaMicro

==========

This update adds configurable support for the Dvorak keyboard.  It is of no interest to non-Dvorak people.  Nothing else has changed.

There is a fourth line in the 'opts.txt' file in the 'conf' subdirectory.  Make it 0 for QWERTY, or 1 for Dvorak.  In this distribution, it is set to 1, but in future distributions it will be defaulted back to 0.

==========

[From the 0.2.1 readme file]

This update fixes the turning animation bug.  Also, it is more compatible with Mac OS X.  It adds a default save file name ('game.dod').

The biggest thing in this release is a file called 'opts.txt' in the new 'conf' directory.  See the main readme file for the explantion.

I've included the source for SDL 1.2.5.  I haven't actually upgrade from 1.2.4 yet, but that was all I could find on their site.  The next release will have the upgraded DLLs.

==========

[From version 0.2 readme2 file]

I thought I would attempt some source code documentation this time, since the C++ source is being distributed with the binary executable.  This is an open source project.  Programmers are encouraged to make corrections and enhancements.  However, I would like to maintain (at least for the present) a single body of code.  In order to do this (since we don't yet have a CVS system), I'm asking that any changes be well commented, and sent to me by email (rick_h_1002@yahoo.com), so that I can incorporate them into the overall code base.  The main reason for this is that I will still be modifying the code quite a bit on some fundamental, structural levels -- i.e., it's not an extremely stable code base just yet.  But I do encourage other interested developers to make all the contributions that they can.  Eventually, the code base will stabilize, and things will be much less messy.

The first thing that will be immediately obvious to any C++ programmer who examines the code is that it is not really very object oriented.  This was a design decision made when I began porting from the original 6809 assembly source.  Most of the code looks more like straight C, rather than C++.  The reason I went with C++ is that I simply like it better in terms of packaging, and I have a vague desire to eventually turn it into a full-fledged object-oriented program, for a future port to Java.

Structurally, there are several classes, all of which have a single global instance instantiated in the bootstrap file 'dod.cpp'.  Also, most of the data members in these classes are public.  I know this is evil (to OO people).  It was done as a convenience to simplify referencing them, and also to minimize the number of pointer dereferences required during execution.  So, a general word of caution: be very careful if you decide to modify any of these public variables, since they very well may used elsewhere in some crucial way.

The core classes are Creature, Dungeon, Object, Parser, Player, Scheduler, and Viewer.  Pretty much all the game functionality is defined in methods of these classes.  Basically, creature movement is in Creature, all the Player command handlers are in Player, etc.  Because I wrote this piecemeal, some things are in classes where they probably don't belong, and eventually should get moved.

The other main classes are dodGame and OS_Link.  OS_Link has a method called 'init()', which is called from 'main'.  This initializes the SDL framework.  I plan to move all the keyboard handling code here, and the saving and loading, but I haven't done it yet.  The dodGame class, has global game management routines.  I thought there would be more here, but it has turned out pretty lame and may be refactored out of existence.

There are two main loops in the program.  The first is at the end of 'OS_Link::init()'.  This loops through games played.  All it does it call the "real" main loop 'Scheduler::SCHED()', which only returns when the game is restarted (after a loss or victory), or when a ZLOAD command is process (which aborts the game in progress and does a restart.  There is no way out of this loop.  Currently the only way to exit the program is to hit the ESC key, which sidesteps the normal flow of execution and simply terminates the program.  This should eventually be made more sophisticated, since the ESC key will eventually be tied to a configuration menu.

The "real" main loop is in the scheduler.  Originally, I ported the rather complex scheduler used in the original source code, but I have replaced it with a very simple version.  There are seven Tasks that the scheduler executes (one of which has 32 instances).  Each task has a frequency and a 'next_time' measured in milliseconds.  When the task's method is executed, it updates its 'next_time'.  After each task, it checks for saving, loading, death, and victory (the last three of which cause a return to the outer loop in OS_Link).

The tasks are:
CLOCK: manages the heartbeat and polls the keyboard
PLAYER: processes keyboard input and executes player commands
LUKNEW: updates screen if needed
HSLOW: damage recovery, slows heart, does fainting and death
BURNER: maintains torch timers
CREGEN: generates random creatures
CMOVE: does creature actions [32 instances]

The various wizard fade in and out sequences are processed in separate loops with separate keyboard handlers.

There are a set of classes which are all defined in 'dod.h'.  These store information about specific creatures and objects and tasks.  Most are lifted directly from the 6809 source.  There are also a couple utility classes, and a class called RNG, which is a port of the original random number generator.

The biggest class by far is Viewer.  It really could benefit from refactoring.  It has all the rendering code and all the definitions for all the vectors and the font.  The vector lists for all the graphics are defined in a file called 'data.cpp'.  This is pretty ugly, and will for sure be changed to make it nicer looking.

==========

You will need the SDL and SDL_Mixer library and include files in order to compile and build the source.  These can be downloaded from the SDL web page (http://www.libsdl.org).  The current version is using SDL 1.2.4, and SDL_Mixer 1.2.  You will also need the OpenGL development files which can be downloaded from their web page (http://www.opengl.org).

I developed this program using Microsoft Visual C++ Version 5.0, although I tried to make it depend as little as possible on the Microsoft platform.  The source included with this distribution includes my VC5 workspace.  If you build this using other tools, I'd love to hear about it.  I hope to someday have a Linux version, and Tim Lindner has expressed interest in doing a Mac version.  Because of this, I have tried to keep the source itself as independent of the environment as possible.

==========

At present, all the sounds are stored in WAV files and loaded into memory during initialization.  The names of the sounds are taken directly from the 6809 source.  They are: squeak (spider), rattle (viper), growl (club stone giant), beoop (blob), klank (no-shield knight), grawl (axe stone giant), pssst (scorpion), kklank (shield knight), pssht (wriath), snarl (galdrog), bdlbdl (wizard), gluglg (flask), phaser (ring), whoop (scroll), clang (shield), whoosh (sword), chuck (torch), klink (player hit creature), clank (creature hit player), thud (player hit wall), bang (player killed creature), kaboom (wizard fade in crash), heart (heartbeat), buzz (wizard fade in buzz).

==========

Ideas for contributions: configuration file (probably more useful than command line arguments), configuration menu (graphical using DoD font), SDL video set up code (for screen resolutions), artifacting (this is a tough one), general code clean-up, making the code more object oriented, fixing coding errors (obviously an important one), getting rid of magic numbers, enhancements (like adding heartbeat monitor in 3D view gutters, translucent map overlay, etc.), maybe a score report at the end of the game (player power, number of creatures killed, etc.), and if anyone (like Matt Palmer) wants to tackle making the mazes truly random, have at it.

I'm always available to answer people's questions via email (rick_h_1002@yahoo.com).

Rick Hunerlach
November 14, 2002
