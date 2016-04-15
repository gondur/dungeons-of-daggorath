#include <ctype.h>
#include "oslink.h"
#include "dodgame.h"
#include "parser.h"
#include "enhanced.h"

extern OS_Link	oslink;
extern dodGame	game;

// these globals hold the options and cheat flag bits
unsigned int g_options=OPT_STEREO; //OPT_HIRES|OPT_STEREO;
unsigned int g_cheats=0;

// translate a DOD string into a standard C string
void GetDodStr(char *pstr, dodBYTE *dodstr)
{
	int x;
	// for each character in dodstr
	for (x=0;dodstr[x]!=Parser::I_NULL;x++) {
		if (dodstr[x]==Parser::I_SP)
			pstr[x]=' ';		  // translate I_SP into ascii
		else
			pstr[x]=dodstr[x]+64; // translate into ascii
	}
	pstr[x]='\0';
}

// translate a standard C string int a DOD string
void SetDodStr(dodBYTE *dodstr, char *pstr)
{
	int x;
	char c;
	for (x=0;pstr[x];x++) { // for each character in pstr
		c=toupper(pstr[x]);		// convert it to uppercase
		if (c>='A' || c<='Z')
			dodstr[x]=c-64;
		else
			dodstr[x]=Parser::I_SP; // replace it with a space
		
	}
	dodstr[x]=Parser::I_NULL;
}

// parse out the first keyword(name) and all remaining data(value)
// this lets us parse commands like SETOPT GFX NORMAL
// the SETOPT part is parsed by PretranslateCommand
// GFX is the name and NORMAL is the value
void ParseOpt(char *opt, char* name, char*val)
{
	// assume val is empty by default
	val[0]='\0';

	// skip leading white space
	while (opt[0] && (opt[0]<=' '))
		opt++;

	// search for next space in string
	char *pDelim=strchr(opt,' ');
	if (pDelim) {
		// copy name part before space
		strncpy(name,opt,pDelim-opt);
		name[pDelim-opt]='\0'; // terminate name, strncpy wont

		// move opt ptr after next space
		opt=pDelim+1;
		// skip white space
		while (opt[0] && (opt[0]<=' '))
			opt++;
		// copy any remaining text into val
		strcpy(val,opt);
	}
	else // no white space, copy it all to name
		strcpy(name,opt);
}

bool SetOption(char *opt)
{
	bool bSuccess=false;
	char name[255];
	char value[255];

	ParseOpt(opt,name,value);

	int nlen=strlen(name);
	if (!nlen) return false; //  no opt name, error
	int vlen=strlen(value);

	if (0==strncmp(name,"GFX",nlen) && vlen) {
		if (0==strncmp(value,"NORMAL",vlen)) {
			// turn off all gfx bits
			g_options &= ~(OPT_VECTOR|OPT_HIRES);
			bSuccess = true;
		}
		else if (0==strncmp(value,"HIRES",vlen)) {
			// turn off all gfx bits but HIRES
			g_options &= ~(OPT_VECTOR);
			g_options |= OPT_HIRES;
			bSuccess = true;
		}
		else if (0==strncmp(value,"VECTOR",vlen)) {
			// turn off all gfx bits but VECTOR
			g_options &= ~(OPT_HIRES);
			g_options |= OPT_VECTOR;
			bSuccess = true;
		}
	}
	else if (name[0]=='S') {
		if (nlen > 1 && 0==strncmp(name+1,"ND",nlen-1) && vlen) {
			if (0==strncmp(value,"MONO",vlen)) {
				// turn off STEREO
				g_options &= ~OPT_STEREO;
				bSuccess = true;
			}
			else if (0==strncmp(value,"STEREO",vlen)) {
				// turn on STEREO
				g_options |= OPT_STEREO;
				bSuccess = true;
			}
		}
		else if (nlen > 1 && 0==strncmp(name+1,"HIELDFIX",nlen-1) && vlen) {
			if (0==strncmp(value,"TRUE",vlen)) {
				// turn on shield fix
				game.ShieldFix = true;
				bSuccess = true;
			}
			else if (0==strncmp(value,"FALSE",vlen)) {
				// turn off shield fix
				game.ShieldFix = false;
				bSuccess = true;
			}
		}
	}
	else if (0==strncmp(name,"RANDOMMAZE",nlen) && vlen) {
		if (0==strncmp(value,"TRUE",vlen)) {
			// turn on random mazes
			game.RandomMaze = true;
			bSuccess = true;
		}
		else if (0==strncmp(value,"FALSE",vlen)) {
			// turn off random mazes
			game.RandomMaze = false;
			bSuccess = true;
		}
	}
	else if (0==strncmp(name,"VISIONSCROLL",nlen) && vlen) {
		if (0==strncmp(value,"TRUE",vlen)) {
			// turn on extra blob w/ vision scroll in level 1
			game.VisionScroll = true;
			bSuccess = true;
		}
		else if (0==strncmp(value,"FALSE",vlen)) {
			// turn off extra blob w/ vision scroll in level 1
			game.VisionScroll = false;
			bSuccess = true;
		}
	}
	else if (0==strncmp(name,"MARKDOORSONMAPS",nlen) && vlen) {
		if (0==strncmp(value,"TRUE",vlen)) {
			// turn on marking doors on maps
			game.MarkDoorsOnScrollMaps = true;
			bSuccess = true;
		}
		else if (0==strncmp(value,"FALSE",vlen)) {
			// turn off marking doors on maps
			game.MarkDoorsOnScrollMaps = false;
			bSuccess = true;
		}
	}
	else if (0==strncmp(name,"CRI",3)) {
		if (nlen > 3 && 0==strncmp(name+3,"GNOREOBJECTS",nlen-3) && vlen) {
			if (0==strncmp(value,"TRUE",vlen)) {
				// turn on creatures ignoring objects when in same room as player
				game.CreaturesIgnoreObjects = true;
				bSuccess = true;
			}
			else if (0==strncmp(value,"FALSE",vlen)) {
				// turn off creatures ignoring objects when in same room as player
				game.CreaturesIgnoreObjects = false;
				bSuccess = true;
			}
		}
		else if (nlen > 3 && 0==strncmp(name+3,"NSTAREGEN",nlen-3) && vlen) {
			if (0==strncmp(value,"TRUE",vlen)) {
				// turn on creatures getting reassigned for level no death
				game.CreaturesInstaRegen = true;
				bSuccess = true;
			}
			else if (0==strncmp(value,"FALSE",vlen)) {
				// turn off creatures getting reassigned for level no death
				game.CreaturesInstaRegen = false;
				bSuccess = true;
			}
		}
	}

	return bSuccess; // string not parsed, error
}

bool SetCheat(char *str)
{
	char name[255];
	char value[255];

	ParseOpt(str,name,value);

	int len=strlen(name);
	if (!len) return false; // no cheat name, error

	if (0==strncmp(name,"NONE",len)) {
		g_cheats=0;
		return true;
	}
	else if (0==strncmp(name,"TORCH",len)) {
		g_cheats|=CHEAT_TORCH;
		return true;
	}
//	else if (0==strncmp(name,"RING",len)) {
//		g_cheats|=CHEAT_RING;
//		return true;
//	}
	else if (name[0]=='R') {
		if (len > 1 && 0==strncmp(name+1,"ING",len-1)) {
			g_cheats|=CHEAT_RING;
			return true;
		}
		else if (len > 1 && 0==strncmp(name+1,"EVEAL",len-1)) {
			g_cheats|=CHEAT_REVEAL;
			return true;
		}
	}
	else if (0==strncmp(name,"CRTSCALE",len)) {
		g_cheats|=CHEAT_REGEN_SCALING;
		return true;
	}
//	else if (0==strncmp(name,"REVEAL",len)) {
//		g_cheats|=CHEAT_REVEAL;
//		return true;
//	}
	else if (name[0]=='I') {
		if (len > 1 && 0==strncmp(name+1,"TEMS",len-1)) {
			g_cheats|=CHEAT_ITEMS;
			return true;
		}
		else if (len > 1 && 0==strncmp(name+1,"NVULNERABLE",len-1)) {
			g_cheats|=CHEAT_INVULNERABLE;
			return true;
		}
	}

	return false;
}

bool PreTranslateCommand(dodBYTE *str)
{
	char buffer[256];
	char *pBuffer=buffer;
	GetDodStr(buffer,str);

	while (pBuffer[0] && (pBuffer[0]<=' ')) pBuffer++;

	if (0==strncmp(pBuffer,"SETOPT ",7)) {
		if (SetOption(pBuffer+7)) {
			SetDodStr(str,"");
			oslink.saveOptFile();  //Save config file change.
			return true;
		}
	}
	else if (0==strncmp(pBuffer,"SO ",3)) {
		if (SetOption(pBuffer+3)) {
			SetDodStr(str,"");
			oslink.saveOptFile();  //Save config file change.
			return true;
		}
	}
	else if (0==strncmp(pBuffer,"SETCHEAT ",9)) {
		if (SetCheat(pBuffer+9)) {
			SetDodStr(str,"");
			oslink.saveOptFile();
			return true;
		}
	}
	else if (0==strncmp(pBuffer,"SC ",3)) {
		if (SetCheat(pBuffer+3)) {
			SetDodStr(str,"");
			oslink.saveOptFile();
			return true;
		}
	}
	else if (0==strncmp(pBuffer,"RESTART", 7)) {
		SetDodStr(str,"");
		return false;
	}
	return true;
}
