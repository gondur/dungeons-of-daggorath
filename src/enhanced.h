bool PreTranslateCommand(dodBYTE *str);

extern unsigned int g_options;

enum {
	OPT_VECTOR=1,
	OPT_HIRES=2,
	OPT_STEREO=4,
};

extern unsigned int g_cheats;

enum {
	CHEAT_TORCH			=0x01,
	CHEAT_RING			=0x02,
	CHEAT_REGEN_SCALING	=0x04,
	CHEAT_ITEMS			=0x08,
	CHEAT_REVEAL		=0x10,
	CHEAT_INVULNERABLE	=0x20,
};

