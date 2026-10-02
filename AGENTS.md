# AGENTS.md

## What this is

SDL 1.2 / OpenGL C++ port of *Dungeons of Daggorath* (Tandy CoCo, 1982), v0.4.0. One
target binary (`dod`). No tests, no CI, no linter, no formatter, no dependency manifest.

**Direction of travel: port to SDL3.** The current code cannot be built or run as-is.
Read "Porting to SDL3" before touching anything else.

## Build & run (current state: all four passes landed — not yet visually verified)

`make -C src` is the only build. SDL 1.2 is gone from this machine, so the checked-in
`Makefile` was rewritten to target SDL3 via `pkg-config`. See "Porting plan" for progress.

Two blockers in the *original* build, both verified, both fixed in Pass 0:
1. `dod.h:24` included `<SDL/SDL.h>` and `Makefile:2` linked `-lSDL -lSDL_mixer`.
   Only SDL3 is installed (`sdl3` 3.4.16; `sdl2-compat` is SDL3 underneath).
2. The link used `$(CC)` = `gcc` on sources that `#include <fstream>` (`oslink.cpp`,
   `sched.cpp`) → undefined `std::ostream::operator<<(int)` / `__gxx_personality_v0`.

Other build notes:
- **There is no native SDL2 here.** `sdl2-compat` 2.32.72 is "an SDL2 compatibility layer
  that uses SDL3 behind the scenes" and depends on `sdl3`. Targeting SDL2 means adding a
  compat layer on top of SDL3 for no gain. Go SDL3.
- `sdl3_mixer` 3.2.4 must be installed: `sudo pacman -S sdl3_mixer`. Note the hyphen:
  the `.pc` file is `sdl3-mixer.pc`, so the Makefile uses `sdl3-mixer`, not `sdl3_mixer`.
- `make -C src clean` is the only cleanup target.
- A `.gitignore` now covers `src/*.o` and `src/dod`.
- **Build → `src/dod`; run from the repo root.** `make -C src` links `src/dod`. Game data
  (`conf/`, `sound/`, `saved/`) is resolved relative to the **working directory**, so the
  binary must be *run* from the repo root — it does not have to *live* there. Any of these
  work: `make -C src install && ./dod`, or just `cd .. && ./src/dod`. `INSTALL` (v0.4.0)
  documents the old `make; cp dod ..` sequence; `make install` now does that copy for you.
  `dod.sh` `cd`s to its own dir and runs `./dod`, which is why the copy exists.
- The root `dod` **is git-tracked** and is still the original ELF32, mode `100644`,
  binary linked against `libSDL-1.2.so.0` and `libSDL_mixer-1.2.so.0` — it cannot run on a
  modern system. It was deliberately *not* replaced: committing a rebuilt binary would put
  an architecture-specific artifact in the tree. So `make install` overwrites a tracked
  file (and dirties the working tree, which `.gitignore` cannot shield). Prefer running
  `./src/dod` from the repo root and leave the tracked file alone; the original is also
  recoverable from commit `4ab53f4` — but do not treat it as a behavioral oracle.

There is no headless or test mode. Verification today is manual: build, launch, ESC to the
meta-menu. Note the game auto-plays an attract-mode demo at boot (`game.AUTFLG`); use
ESC → FILE → START NEW GAME to get a real game.

## Porting plan

Four passes, ordered so you get a running game as early as possible. Progress is noted
below — update this section as passes land.

- **Pass 0 — build plumbing.** ✅ *Done.* Makefile now uses `pkg-config`, links with `g++`,
  has a `make syntax` gate, and errors with `sudo pacman -S sdl3 sdl3-mixer` if deps are
  missing. Dead `rng.cpp`/`rng.h` deleted via `git rm` (recoverable from commit `4ab53f4`).
  Includes updated to `<SDL3/...>`.
  **Two non-obvious fixes found here.** (a) Arch names the mixer `.pc` file
  `sdl3-mixer.pc` (hyphen) — `sdl3_mixer` does not resolve. (b) `-ansi` had to go: SDL3's
  headers need C99+.
- **Pass 1 — window + GL context.** ✅ *Done, not yet runnable.* Added `SDL_Window *window` /
  `SDL_GLContext glContext` to `OS_Link`; deleted `bpp`/`flags`. `changeVideoRes`
  (oslink.cpp:1142) now destroys and recreates the window + context rather than resizing —
  SDL3 has no `SDL_SetVideoMode`, and destroying the window invalidates the GL context.
  24 `SDL_GL_SwapBuffers` → `SDL_GL_SwapWindow(oslink.window)`; all 8 `SDL_VIDEOEXPOSE`
  cases deleted; `SDL_WM_SetCaption` gone (title now passed to `SDL_CreateWindow`).
  `SDL_Init` no longer takes `SDL_INIT_TIMER` — gone in SDL3, timers are always available.
  Verified by compiling and *running* the new code against real SDL3 in isolation.
- **Pass 2 — input.** ✅ *Done, not yet runnable.* `SDL_keysym` → `const SDL_KeyboardEvent *`
  in `oslink.h`, `sched.h:41,47`, and the call sites. `event.key.keysym.sym` → `event.key.key`.
  `keys[256]` → `std::map<SDL_Keycode, dodBYTE>` plus a `keyToChar(keycode, deflt)` helper.
  Renamed the 28 SDL3-poisoned identifiers (26 lowercase `SDLK_a`→`SDLK_A`, `AUDIO_S16`,
  `SDL_KEYDOWN`→`SDL_EVENT_KEY_DOWN`, `SDL_QUIT`→`SDL_EVENT_QUIT`).
- **Pass 3 — audio.** ✅ *Done — the game builds, links, and runs.* All 93 `Mix_*` sites
  converted. `Mix_Chunk*` → `MIX_Audio*` (11 declarations), the five `int` channel members
  → `MIX_Track*`, and `Mix_AllocateChannels(4)` → four `MIX_CreateTrack()` calls in
  `init()`. Volume is scaled inside `OS_Link` wrappers so the original int 0–128 math and
  `volumeLevel=128` in `conf/opts.ini` keep working.
  **The 23 blocking sound loops — ported, NOT deleted (I was wrong to advise deleting).**
The idiom `Mix_PlayChannel(...); while(Mix_Playing(ch) == 1) { scheduler.CLOCK(); }` appears
23 times. They were left in place and converted to
`oslink.playSound(...); while(oslink.isSoundPlaying(ch)) { scheduler.CLOCK(); }`.

Auditing the bodies first showed they are **not** one uniform pattern, and deleting them
would have broken real game logic:

- 13 are pure heartbeat pumps — genuinely removable, the `CLOCK()` task keeps the
  heartbeat alive on its own.
- 5 contain live logic: `if (game.AUTFLG && game.demoRestart == false) return;` and the
  wizard-fade key aborts (`if (fadeMode == 1 && scheduler.keyCheck()) { ...; return false; }`).
  Deleting the loop deletes the input polling.
- 5 in `viewer.cpp` have **empty bodies** — they busy-wait for the *length* of `kaboom` to
  synchronize the wizard fade animation. These are timing-critical, not "sound overlapping
  the heartbeat".

If you want to remove them, do it as a separate, behavior-checked change — not as part of
a port. `viewer.cpp`'s empty ones are now a tight CPU spin on `MIX_TrackPlaying`; prefer
`MIX_SetTrackStoppedCallback()`.

**Gotchas that cost real debugging time here:**
- **`MIX_CreateMixerDevice(0, ...)` fails** with "Invalid audio device instance ID".
  The device must be `SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK` (0xFFFFFFFF). SDL 1.2's
  `Mix_OpenAudio` took a rate, so passing 0 "looked" harmless.
- **The mixer handle is now required state.** `MIX_LoadAudio` needs it, so `OS_Link` gained
  a `MIX_Mixer *mixer` field. `MIX_LoadAudio(mixer, path, false)` — the `false` is
  `predecode`, the closest analogue of `Mix_LoadWAV`.
- **Tracks must not be reset in `Reset()`.** `Creature::Reset()`, `Object::Reset()` and
  `Scheduler::Reset()` used to set `creChannel = 1` etc. Since those members are now
  `MIX_Track*`, the `int` assignments were hard errors, and re-adding them would clobber the
  track pointers on every game restart. The assignments were replaced with comments; the
  tracks are created once in `OS_Link::init()`. `Viewer::fadChannel` is initialized to
  `NULL` in the constructor instead of `3`.
- **The original volume scale is preserved deliberately.** `MIX_SetTrackGain` takes a
  float 0.0–1.0, but the wizard fade math
  (`((32 - VCTFAD) / 2) * (oslink.volumeLevel / 16)`) is integer 0–128 arithmetic. The
  `OS_Link` wrappers divide by `DOD_MIX_MAX_VOLUME` (=128) at the boundary, so
  `conf/opts.ini`'s `volumeLevel=128` and every fade formula still mean what they did.
- **`Mix_Volume(-1, v)` meant "every channel"** → `setMasterGain()` (mixer gain), not a
  track. Only two call sites used `-1`.
- **`Mix_SetPanning(ch, left, right)`** → `MIX_SetTrackStereo` via `setTrackPanning`, which
  converts the 0–255 panning values to the float `MIX_StereoGains` struct.
- `gluOrtho2D` needs an explicit `#include <GL/glu.h>` in `viewer.h`. SDL 1.2 bundled the
  GL headers; SDL3's `SDL_opengl.h` does not pull in GLU.
- SDL3 **removed** five keycodes outright (no renamed equivalent): `SDLK_LSUPER`,
  `SDLK_RSUPER`, `SDLK_COMPOSE`, `SDLK_NUMLOCK` (→ `SDLK_NUMLOCKCLEAR`), `SDLK_SCROLLOCK`
  (→ `SDLK_SCROLLLOCK`). `LMETA`/`RMETA` already covered Super.

**Verification status.** `make syntax` is clean across all 11 TUs and the binary links
(64-bit now, was ELF32). Verified by running against real SDL3: window creation and GL
context succeed, `MIX_CreateMixerDevice`/`MIX_CreateTrack`/`MIX_LoadAudio`/`MIX_PlayTrack`
all return valid handles, and the process survives a 25s run of the main loop. **Not yet
verified: on-screen rendering.** The compositor's screen-capture path grabs input and
blocks the shell, so nobody has confirmed the game actually draws. Run `./dod` yourself and
check — that is the one gap in the port.

**Header includes:** SDL3 uses `<SDL3/SDL.h>` / `<SDL3_mixer/SDL_mixer.h>` with angle
brackets and subdirectories. The `#ifdef LINUX` `<SDL/SDL.h>` vs `<SDL.h>` split at
`dod.h:22-30` and `oslink.h:23-29` can collapse to a single unconditional include.

**What does *not* need porting:** rendering is fixed-function OpenGL 1.x — 40 `glBegin`/
`glEnd` blocks with `GL_LINES`/`GL_QUADS`/`GL_POINTS`, `GL_LINE_SMOOTH`, and
`gluOrtho2D` in `Viewer::setup_opengl` (viewer.cpp:349). SDL3 still hands you a legacy GL
context, so this all carries over as-is. The pure-logic files — `dungeon.cpp`, `parser.cpp`,
`object.cpp`, `enhanced.cpp` — have **zero** SDL references and should not be edited.

## Sizing / centering (fixed after the port; easy to regress)

The game must letterbox a 4:3 area *centered* in whatever the real drawable is. Three
things have to agree, and they historically did not:

- **`OS_Link::queryDrawableSize()` (oslink.cpp) is the only correct source of truth.**
  In fullscreen, the compositor owns the final size, so SDL keeps reporting the size we
  *requested* — `SDL_GetWindowSizeInPixels()` returns 1024x768 while the actual drawable
  is 1920x1080. Asking SDL for the window size in fullscreen is simply wrong. Use
  `SDL_GetCurrentDisplayMode()` there; use `SDL_GetWindowSizeInPixels()` only when
  windowed. Note `SDL_GL_GetDrawableSize()` was removed in SDL3 and has no replacement
  that fixes this.
- **`Coordinate::setCurWH(W, H)` takes the real window size on both axes** and centers
  the 4:3 box on each independently, preserving the "multiple of 256" rule so maze cells
  stay square. The old single-argument version assumed 4:3 and derived `offY` from
  `offX * 0.75`, so the offset was always 0 for the requested resolution.
- **`oslink.width`/`height` must hold the real drawable size**, because
  `Viewer::setup_opengl` builds `glViewport` and `gluOrtho2D` from them.

**Do not add `SDL_WINDOW_RESIZABLE`.** The window should be exactly the selected game
resolution; letting the compositor resize it decouples the viewport from the window and
the picture is drawn partly outside it. `OS_Link::syncViewport()` (wired to
`SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED`) re-syncs if a size change happens anyway.

Verified on 1920x1080: content measures exactly 1280x960 at offset (320, 60) — equal
320px gutters — via `magick -trim`. Verified on 4:3 (2560x1440), 5:4 (1280x1024), windowed
1024x768/800x600/640x480, and portrait.

## Runtime data contracts

Everything is CWD-relative (`oslink.cpp:57-59` hardcodes `conf`, `sound`, `saved`).
Never launch from `src/`.

- **`conf/opts.ini`** — `key=value`, parsed by whitespace `>>` in `OS_Link::loadOptFile`
  (oslink.cpp:946). Watch out:
  - Every line must contain `=`. The guard at oslink.cpp:970 is `if(breakPoint || ...)`
    (should be `&&`), so a line without `=` dereferences NULL and crashes.
  - `saveDirectory=` is parsed but then ignored — the value is hardcoded back to `"saved"`
    (oslink.cpp:1019). Matches the "still a todo" note in `src/ChangeLog.txt`.
  - `OS_Link::saveOptFile` rewrites the whole file and appends 6 gameplay flags
    (`RandomMaze`, `ShieldFix`, `VisionScroll`, `CreaturesIgnoreObjects`,
    `CreaturesInstaRegen`, `MarkDoorsOnScrollMaps`) that are absent from the committed file.
    Expect a `conf/opts.ini` diff after any use of the menu or of `SETOPT`/`SETCHEAT`.
  - Numeric fields are integers only, no floats.
- **`sound/`** — 26 WAVs loaded by exact name at init. `Utils::LoadSound` (dod.cpp:159)
  never NULL-checks, so a missing or renamed file hands a NULL `Mix_Chunk*` to the mixer.
- **`saved/`** — `ZSAVE`/`ZLOAD` default to `game.dod`. A custom name must be a single
  uppercase A–Z word (`NICK.dod` is checked in); digits are rejected. Format is one ASCII
  integer per line.
  - **Running the game can overwrite these.** `game.dod` is the default save target, and a
    stray typed `ZSAVE` (the demo never sends one) writes it immediately. This happened
    once during verification when keystrokes meant for the shell reached the focused game
    window. **Copy them somewhere safe before launching the game**, or restore with
    `git checkout saved/` afterwards.
  - `Scheduler::SAVE` (sched.cpp:506) and `Scheduler::LOAD` (sched.cpp:629) are
    hand-paired positional `sprintf`/`sscanf` lists. Adding, removing, or reordering a
    field in one silently corrupts every save. Edit them together or not at all.
  - Changing array sizes (`MAZLND[1024]`, `CCBLND[32]`, `OCBLND[72]`) invalidates all
    committed `.dod` files.

## Architecture

- **Everything is a global.** `src/dod.cpp:37-47` declares the single instance of each
  class; every other file re-declares them `extern` and mutates members directly. Members
  are `public` by design. There is no encapsulation to lean on — grep for a member before
  changing it, it very likely has callers in several subsystems.
- **Two loops.** `OS_Link::init()` (oslink.cpp:69) is the outer loop over games; it never
  returns except through `quitSDL()`. It calls `Scheduler::SCHED()` (sched.cpp:116), the
  real loop: a 38-entry `Task` array `TCBLND` with millisecond frequencies — CLOCK, PLAYER,
  LUKNEW, HSLOW, BURNER, CREGEN, and CMOVE (32 instances). `SCHED` returns on death, on
  victory, or on a `ZLOAD`; the outer loop then restarts or resumes.
- **Input path.** `OS_Link::handle_key_down` → `keys[]` remap → `parser.KBDPUT` → the
  PLAYER task → `Player::*` handlers. Commands are **not** matched with string compares:
  `Parser::PARSER` decodes packed hex tables `CMDTAB`/`DIRTAB` (parser.cpp:68-69) via
  `EXPAND`/`GETFIV`. Adding a player command means extending those tables.
- **Console commands / cheats** live in `enhanced.cpp`, not in `Player`. Globals
  `g_options` / `g_cheats` (enhanced.h). `PreTranslateCommand` (enhanced.cpp:253) intercepts
  typed commands *before* the parser and calls `oslink.saveOptFile()`.
- **Attract-mode demo** is a hex blob of scripted keystrokes: `dodGame::DEMO_CMDS` +
  `DEMOPTR` (dodgame.cpp:52-69), fed through the normal parser.
- **Graphics data is hex strings in code.** ~200 `*_VLA` vector tables are filled in the
  `Viewer` constructor via `Utils::LoadFromHex` (dod.h:471). `src/readme2.txt:58` says these
  live in `data.cpp` — that file does not exist; they are in `viewer.cpp` now.
- **Coordinates.** `Coordinate` (dod.h) maps original 256×192 CoCo coords to the current
  resolution. `setCurWH` assumes 4:3 and rounds the width down to a multiple of 256.

## Conventions

- Tabs, Allman braces, `PascalCase_Method()` for methods (`PLAYER`, `PZSAVE`, `COMINI`),
  `UPPER_CASE` members. Nothing enforces this; match the surrounding file.
- **`dungeon.cpp` is latin-1, not UTF-8** (a raw `\xb7` byte at ~offset 931). Any scripted
  edit must read/write it with `encoding='latin-1'`, or it will corrupt the byte.
- Don't add `-DSDL_ENABLE_OLD_NAMES` to make old SDL2 names compile — it un-poisons ~550
  identifiers that mask genuine SDL3 API changes. See the porting section.
- `-ansi` (i.e. `-std=c++98`) was dropped in Pass 0: SDL3's headers need C99+ and reject
  `-ansi`. The code itself stays C++98-compatible — **don't introduce C++11+ syntax** in
  game logic, to keep the source portable to the original CoCo-porting constraints.
- `dodBYTE` (`unsigned char`) / `dodSHORT` are deliberate: bit-level code ported from 6809
  assembly depends on exact widths. Don't "fix" these to `int`.
- `parser.cpp` and `dodgame.cpp` intentionally keep the original `goto`-based control flow
  and commented-out 6809 tables. Leave them alone unless the task is about them.
- **Docs are stale.** Root `readme.txt` documents the Windows v0.3 port (`dodwin.exe`,
  `conf/opts.txt`) and claims ESC exits the program. In 0.4.0 ESC opens a meta-menu with
  FILE / CONFIGURE / HELP (constants in dod.h:538-562) and the config is `conf/opts.ini`.
  `src/ChangeLog.txt` is the accurate version history.
- **Checked-in junk — don't extend or "clean up" unasked:** `src/.Makefile.swp`,
  `src/.oslink.cpp.swp` (vim swap files), `src/dodwin.*` (MSVC 5 project),
  `src/dod.ico` / `wiz.ico` / `wizicon.*` / `dod_private.*` (Windows resources),
  `src/dump` (old Autoconf cache).
- Do not commit or push unless explicitly asked. The repo has one commit and no conventions.
