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
// Filename: rng.cpp
//
// Implementation of RNG class

// Constructor
RNG::RNG() : carry(0)
{
	SEED[0] = 0;
	SEED[1] = 0;
	SEED[2] = 0;
}

// Returns the next random number based on seeds
// and updates the seeds accordingly.
dodBYTE RNG::RANDOM()
{
	int x, y;
	dodBYTE a, b;
	carry = 0;
	for (x = 8; x != 0; --x)
	{
		b = 0;
		a = (SEED[2] & 0xE1);
		for (y = 8; y != 0; --y)
		{
			a = lsl(a);
			if (carry != 0)
				++b;
		}
		b = lsr(b);
		SEED[0] = rol(SEED[0]);
		SEED[1] = rol(SEED[1]);
		SEED[2] = rol(SEED[2]);
	}
	return SEED[0];
}
