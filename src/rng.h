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
// Filename: rng.h
//
// This class is a port of Daggorath's custom RNG
// All utility methods are inlined for speed.

#ifndef DOD_RNG_HEADER
#define DOD_RNG_HEADER

#include "dod.h"

class RNG
{
public:
	// Constructor
	RNG();

	// Accessors
	dodBYTE RANDOM();
	dodBYTE getSEED(int idx) { return SEED[idx]; }

	// Mutators
	void setSEED(int idx, dodBYTE val) { SEED[idx] = val; }
	void setSEED(dodBYTE val0, dodBYTE val1, dodBYTE val2)
	{
		SEED[0] = val0;
		SEED[1] = val1;
		SEED[2] = val2;
	}

	dodBYTE SEED[3];
	dodBYTE carry;

private:
	// Internal Implementation
	dodBYTE lsl(dodBYTE c);
	dodBYTE lsr(dodBYTE c);
	dodBYTE rol(dodBYTE c);
};

// Inline Definitons
inline dodBYTE RNG::lsl(dodBYTE c)
{
	carry = (((c & 128) == 128) ? 1 : 0);
	return c << 1;
}

inline dodBYTE RNG::lsr(dodBYTE c)
{
	carry = (((c & 1) == 1) ? 1 : 0);
	return c >> 1;
}

inline dodBYTE RNG::rol(dodBYTE c)
{
	dodBYTE cry;
	cry = (((c & 128) == 128) ? 1 : 0);
	c <<= 1;
	c += carry;
	carry = cry;
	return c;
}

#endif // DOD_RNG_HEADER
