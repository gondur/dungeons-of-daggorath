CC       = g++
PKGCONFIG = pkg-config

# NOTE: Arch names the mixer .pc file "sdl3-mixer.pc" (hyphen). pkg-config normally
# treats - and _ as equivalent, but the module must match what is actually installed,
# so use the hyphenated spelling here.
PKGS      = sdl3 sdl3-mixer

# Fail loudly and actionably rather than emitting pkg-config's error on every
# invocation (which happens once per $(shell ...)).
ifeq ($(shell $(PKGCONFIG) --exists $(PKGS) && echo ok),)
$(error Missing build dependencies: $(PKGS). Install with: sudo pacman -S $(PKGS))
endif

SDL_CFLAGS := $(shell $(PKGCONFIG) --cflags $(PKGS))
SDL_LIBS   := $(shell $(PKGCONFIG) --libs   $(PKGS))

CXXFLAGS  = -Wall -c -DLINUX $(SDL_CFLAGS)
LDLIBS    = $(SDL_LIBS) -lGL -lGLU

TARGET  = dod
SRCS    = creature.cpp dod.cpp dodgame.cpp dungeon.cpp enhanced.cpp object.cpp \
          oslink.cpp parser.cpp player.cpp sched.cpp viewer.cpp
OBJS    = $(SRCS:.cpp=.o)
HDRS    = creature.h dod.h dodgame.h dungeon.h enhanced.h object.h oslink.h \
          parser.h player.h sched.h viewer.h

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

%.o: %.cpp $(HDRS)
	$(CC) $(CXXFLAGS) -o $@ $<

# Copy the binary to the repo root, which is what ./dod.sh and the checked-in
# launcher expect. The binary does not have to *live* in the root - it only has
# to be *run* from there, because conf/, sound/ and saved/ are resolved relative
# to the working directory. `cd .. && ./src/dod` works just as well.
install: $(TARGET)
	cp -f $(TARGET) ../dod
	chmod +x ../dod
	@echo "Installed ../dod - run it with: ./dod"

# Syntax-only check across every translation unit. This is the Pass 0 gate.
.PHONY: syntax
syntax:
	@for f in $(SRCS); do \
		echo "  [syntax] $$f"; \
		$(CC) $(CXXFLAGS) -fsyntax-only $$f || exit 1; \
	done; \
	echo "All translation units parse cleanly."

clean:
	@echo -n Cleaning...
	@rm -f $(OBJS) $(TARGET)
	@echo Done

.PHONY: all install syntax clean