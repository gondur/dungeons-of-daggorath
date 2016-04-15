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
// Filename: dod.cpp
//
// All primary objects are declared in global space
// to minimize dereferencing of pointers.  The main
// function simply calls init on the oslink object
// which contains all the SDL setup code.
//
// Every other file that uses these primary objects
// needs to include the objects header and declare
// an external reference to the object, which will
// resolve to the declarations below.  This can
// probably be simplified with some common mechanism.

#include "dod.h"
#include "dodgame.h"
#include "player.h"
#include "object.h"
#include "creature.h"
#include "dungeon.h"
#include "sched.h"
#include "viewer.h"
#include "oslink.h"
#include "parser.h"

dodGame		game;
Coordinate	crd;
RNG			rng;
Player		player;
Object		object;
Creature	creature;
Dungeon		dungeon;
Scheduler	scheduler;
Viewer		viewer;
OS_Link		oslink;
Parser		parser;

// Could include some command line arguments for
// various purposes (like configurations) if desired later.
//
// However, I think a config file would be nicer
//

void printvls();
void printthem(int *, int, char *);

int main(int argc, char * argv[])
{
	//printvls();
	//exit(0);

	printf( "Starting DoD.\n" );

	oslink.init();
	return 0;
}

void printalpha(int * vl, int len, char * name)
{
	int ctr;
	printf("Utils::LoadFromDecDigit(%s, \"", name);
	for (ctr = 1; ctr < len; ++ctr)
	{
		printf("%02X",vl[ctr]);
	}
	printf("\");\n");
}

void printvls()
{
	printalpha(viewer.SHIE_VLA, 14, "SHIE_VLA");
	printalpha(viewer.SWOR_VLA, 11, "SWOR_VLA");
	printalpha(viewer.TORC_VLA, 10, "TORC_VLA");
	printalpha(viewer.RING_VLA, 12, "RING_VLA");
	printalpha(viewer.SCRO_VLA, 12, "SCRO_VLA");
	printalpha(viewer.FLAS_VLA, 10, "FLAS_VLA");

/*
	printalpha(viewer.SP_VLA, 39, "SP_VLA");
	printalpha(viewer.WR_VLA, 42, "WR_VLA");
	printalpha(viewer.SC_VLA, 41, "SC_VLA");
	printalpha(viewer.BL_VLA, 66, "BL_VLA");
	printalpha(viewer.GL_VLA, 141, "GL_VLA");
	printalpha(viewer.VI_VLA, 65, "VI_VLA");
	printalpha(viewer.S1_VLA, 130, "S1_VLA");
	printalpha(viewer.S2_VLA, 126, "S2_VLA");
	printalpha(viewer.K1_VLA, 153, "K1_VLA");
	printalpha(viewer.K2_VLA, 149, "K2_VLA");
	printalpha(viewer.W0_VLA, 133, "W0_VLA");
	printalpha(viewer.W1_VLA, 199, "W1_VLA");
	printalpha(viewer.W2_VLA, 185, "W2_VLA");
	printalpha(viewer.LAD_VLA, 56, "LAD_VLA");
	printalpha(viewer.HUP_VLA, 29, "HUP_VLA");
	printalpha(viewer.HDN_VLA, 19, "HDN_VLA");
	printalpha(viewer.CEI_VLA, 6, "CEI_VLA");
	printalpha(viewer.LPK_VLA, 12, "LPK_VLA");
	printalpha(viewer.RPK_VLA, 12, "RPK_VLA");
	printalpha(viewer.FSD_VLA, 8, "FSD_VLA");
	printalpha(viewer.LSD_VLA, 8, "LSD_VLA");
	printalpha(viewer.RSD_VLA, 8, "RSD_VLA");
	printalpha(viewer.RWAL_VLA, 10, "RWAL_VLA");
	printalpha(viewer.LWAL_VLA, 10, "LWAL_VLA");
	printalpha(viewer.FWAL_VLA, 11, "FWAL_VLA");
	printalpha(viewer.RPAS_VLA, 15, "RPAS_VLA");
	printalpha(viewer.LPAS_VLA, 15, "LPAS_VLA");
	printalpha(viewer.FPAS_VLA, 1, "FPAS_VLA");
	printalpha(viewer.RDOR_VLA, 24, "RDOR_VLA");
	printalpha(viewer.LDOR_VLA, 24, "LDOR_VLA");
	printalpha(viewer.FDOR_VLA, 25, "FDOR_VLA");

	printalpha(viewer.A_VLA, 33, "A_VLA");
	printalpha(viewer.B_VLA, 49, "B_VLA");
	printalpha(viewer.C_VLA, 41, "C_VLA");
	printalpha(viewer.D_VLA, 33, "D_VLA");
	printalpha(viewer.E_VLA, 33, "E_VLA");
	printalpha(viewer.F_VLA, 25, "F_VLA");
	printalpha(viewer.G_VLA, 49, "G_VLA");
	printalpha(viewer.H_VLA, 25, "H_VLA");
	printalpha(viewer.I_VLA, 25, "I_VLA");
	printalpha(viewer.J_VLA, 25, "J_VLA");
	printalpha(viewer.K_VLA, 65, "K_VLA");
	printalpha(viewer.L_VLA, 17, "L_VLA");
	printalpha(viewer.M_VLA, 41, "M_VLA");
	printalpha(viewer.N_VLA, 41, "N_VLA");
	printalpha(viewer.O_VLA, 33, "O_VLA");
	printalpha(viewer.P_VLA, 33, "P_VLA");
	printalpha(viewer.Q_VLA, 57, "Q_VLA");
	printalpha(viewer.R_VLA, 57, "R_VLA");
	printalpha(viewer.S_VLA, 57, "S_VLA");
	printalpha(viewer.T_VLA, 33, "T_VLA");
	printalpha(viewer.U_VLA, 25, "U_VLA");
	printalpha(viewer.V_VLA, 41, "V_VLA");
	printalpha(viewer.W_VLA, 41, "W_VLA");
	printalpha(viewer.X_VLA, 73, "X_VLA");
	printalpha(viewer.Y_VLA, 41, "Y_VLA");
	printalpha(viewer.Z_VLA, 57, "Z_VLA");
	printalpha(viewer.PER_VLA, 9, "PER_VLA");
	printalpha(viewer.UND_VLA, 9, "UND_VLA");
	printalpha(viewer.EXP_VLA, 17, "EXP_VLA");
	printalpha(viewer.QSM_VLA, 49, "QSM_VLA");
	printalpha(viewer.SHL_VLA, 9, "SHL_VLA");
	printalpha(viewer.SHR_VLA, 33, "SHR_VLA");
	printalpha(viewer.LHL_VLA, 17, "LHL_VLA");
	printalpha(viewer.LHR_VLA, 41, "LHR_VLA");
*/
}

Mix_Chunk *Utils::LoadSound(char *snd)
{
	char fn[256];
	sprintf(fn, "%s%s%s", oslink.soundDir, oslink.pathSep, snd);
	return Mix_LoadWAV(fn);
}
