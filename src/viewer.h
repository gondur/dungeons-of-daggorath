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
// Filename: viewer.h
//
// This class manages drawing to the screen, including
// setting up the OpenGL data.  It's a work in progress.
//
// At the moment it is huge, which means that it really
// needs to be broken into smaller, more well-defined
// classes.  But it works for the present.

#ifndef DOD_VIEWER_HEADER
#define DOD_VIEWER_HEADER

#include "dod.h"
#include "dodgame.h"

extern dodGame	game;

class Viewer
{
public:
	// Constructor
	Viewer();

	// Public Interface
	void		setup_opengl();
	void		draw_game();
	bool		draw_fade();
	void		enough_fade();
	void		death_fade(int WIZ[]);
	void		displayCopyright();
	void		displayWelcomeMessage();
	void		displayDeath();
	void		displayWinner();
	void		displayEnough();
	void		displayPrepare();
	void		drawArea(TXB * a);
	void		clearArea(TXB * a);
	void		drawTorchHighlite();
	void		WIZIN0();
	int			LUKNEW();
	void		PUPDAT();
	void		PUPSUB();
	void		STATUS();
	void		PROMPT();
	void		EXAMIN();
	void		PCRLF();
	void		PRTOBJ(int X, bool highlite);
	void		OUTSTI(dodBYTE * comp);
	void		OUTSTR(dodBYTE * str);
	void		OUTCHR(dodBYTE c);
	void		TXTXXX(dodBYTE c);
	void		TXTSCR();
	void		VIEWER();
	void		SETSCL();
	void		DRAWIT(int * vl);
	void		PDRAW(int * vl, dodBYTE dir, dodBYTE pdir);
	void		CMRDRW(int * vl, int creNum);
	void		SETFAD();
	dodSHORT	ScaleX(int x);
	dodSHORT	ScaleY(int y);
	float		ScaleXf(float x);
	float		ScaleYf(float y);
	void		MAPPER();
	void		setVidInv(bool inv);
	void		drawVectorList(int VLA[]);
	void		drawVector(float X0, float Y0, float X1, float Y1);
	void		Reset();
	bool		ShowFade(int fadeMode);
	void            drawMenu(menu, int, int);
	void		drawMenuList(int, int, char *, char *[], int, int);
	void		drawMenuScrollbar(char *, int);
	void		drawMenuStringTitle(char *);
	void		drawMenuString(char *);
	void		aboutBox(void);

	// Public Data Fields
	dodBYTE		VCTFAD;
	dodBYTE		RANGE;
	bool		showSeerMap;
	Uint32		delay, delay1, delay2;
	bool		done;
	int			fadeVal;
	dodBYTE		UPDATE;
	dodSHORT	display_mode; // 0 = map, 1 = 3D, 2 = Examine, 3 = Prepare
	int			fadChannel;

	int			buzzStep;
	int			midPause;
	int			prepPause;

	dodBYTE		Scale[21];
	float		Scalef[21];
	int *		LArch[4];
	int *		FArch[4];
	int *		RArch[4];

	char		textArea[(32*4)+1];
	char		examArea[(32*19)+1];
	char		statArea[(32*1)+1];

	TXB			TXTPRI;
	TXB			TXTEXA;
	TXB			TXTSTS;

	GLfloat		bgColor[3];
	GLfloat		fgColor[3];
	dodBYTE		RLIGHT;
	dodBYTE		MLIGHT;
	dodBYTE		OLIGHT;
	dodBYTE		VXSCAL;
	dodBYTE		VYSCAL;
	float		VXSCALf;
	float		VYSCALf;

	dodBYTE		TXBFLG;
	TXB *		TXB_U;
	int			tcaret;
	int			tlen;

	dodBYTE		enough1[21];
	dodBYTE		enough2[20];
	dodBYTE		winner1[21];
	dodBYTE		winner2[17];
	dodBYTE		death[21];
	dodBYTE		copyright[21];
	dodBYTE		welcome1[14];
	dodBYTE		welcome2[20];
	dodBYTE		prepstr[6];
	dodBYTE		exps[3];
	dodBYTE		exam1[9];
	dodBYTE		exam2[8];
	dodBYTE		exam3[7];
	dodBYTE		NEWLUK;

	int			LINES[11];

	dodBYTE		HLFSTP;
	dodBYTE		BAKSTP;
	dodBYTE		NEWLIN;

	enum {
		MODE_MAP=0,
		MODE_3D,
		MODE_EXAMINE,
		MODE_TITLE,

		FADE_BEGIN=1,
		FADE_MIDDLE,
		FADE_DEATH,
		FADE_VICTORY,
	};

private:
	// Internal Implementation
	void drawVectorListAQ(int VLA[]);
	void drawCharacter(char c);
	void drawString(int x, int y, char * str, int len);
	void drawString_internal(int x, int y, dodBYTE * str, int len);
	void plotPoint(double X, double Y);
	char dod_to_ascii(dodBYTE c);

	// Data Fields
	dodSHORT	VCNTRX;
	dodSHORT	VCNTRY;
	dodBYTE		OROW;
	dodBYTE		OCOL;
	dodBYTE		MAGFLG;
	dodBYTE		PASFLG;
	int			HLFSCL;
	int			BAKSCL;
	int			NORSCL;
public:
	int A_VLA[33];
	int B_VLA[49];
	int C_VLA[41];
	int D_VLA[33];
	int E_VLA[33];
	int F_VLA[25];
	int G_VLA[49];
	int H_VLA[25];
	int I_VLA[25];
	int J_VLA[25];
	int K_VLA[65];
	int L_VLA[17];
	int M_VLA[41];
	int N_VLA[41];
	int O_VLA[33];
	int P_VLA[33];
	int Q_VLA[57];
	int R_VLA[57];
	int S_VLA[57];
	int T_VLA[33];
	int U_VLA[25];
	int V_VLA[41];
	int W_VLA[41];
	int X_VLA[73];
	int Y_VLA[41];
	int Z_VLA[57];
	int NM0_VLA[33];
	int NM1_VLA[25];
	int NM2_VLA[49];
	int NM3_VLA[57];
	int NM4_VLA[33];
	int NM5_VLA[49];
	int NM6_VLA[41];
	int NM7_VLA[49];
	int NM8_VLA[57];
	int NM9_VLA[49];
	int PER_VLA[9];
	int UND_VLA[9];
	int EXP_VLA[17];
	int QSM_VLA[49];
	int SHL_VLA[9];
	int SHR_VLA[33];
	int LHL_VLA[17];
	int LHR_VLA[41];
	int FSL_VLA[41];	//Forward Slash
	int BSL_VLA[41];	//Back Slash
	int PCT_VLA[57];	//Percent
	int PLS_VLA[17];	//Plus
	int DSH_VLA[9];		//Dash
	int * AZ_VLA[50];

	int SP_VLA[39];
	int WR_VLA[42];
	int SC_VLA[41];
	int BL_VLA[66];
	int GL_VLA[141];
	int VI_VLA[65];
	int S1_VLA[130];
	int S2_VLA[126];
	int K1_VLA[153];
	int K2_VLA[149];
	int W0_VLA[133];

public:
	int W1_VLA[199];
	int W2_VLA[185];

//private:
	int LAD_VLA[56];
	int HUP_VLA[29];
	int HDN_VLA[19];
	int CEI_VLA[6];
	int LPK_VLA[12];
	int RPK_VLA[12];
	int FSD_VLA[8];
	int LSD_VLA[8];
	int RSD_VLA[8];
	int RWAL_VLA[10];
	int LWAL_VLA[10];
	int FWAL_VLA[11];
	int RPAS_VLA[15];
	int LPAS_VLA[15];
	int FPAS_VLA[1];
	int RDOR_VLA[24];
	int LDOR_VLA[24];
	int FDOR_VLA[25];

	int FLATAB[3];
	int ** FLATABv[3];
	int * FWDOBJ[6];
	int * FWDCRE[12];

	int SHIE_VLA[14];
	int SWOR_VLA[11];
	int TORC_VLA[10];
	int RING_VLA[12];
	int SCRO_VLA[12];
	int FLAS_VLA[10];
};

#endif // DOD_VIEWER_HEADER
