#pragma once

// todo: Finish this
// todo: use proper reversed names you idiot

struct CEverythingThatHasWheels {
	// wheels (num indicated by int before this struct in vehicle struct)
	// plenty 'o space for more pointers for wheels
	// height, other shit
}; // very incomplete as you can see lmao

struct CWheel {
	float unknown0;			// 0x000 - 0x004
	float unknown_;			// 0x004 - 0x008
	float camber;			// 0x008 - 0x00C
	float unknown1;			// 0x00C - 0x010
	float camberInv;		// 0x010 - 0x014
	char unknown2[0x01C];	// 0x014 - 0x030
	float trackWidth;		// 0x030 - 0x034
	float unknown3;			// 0x034 - 0x038
	float height;			// 0x038 - 0x03C
	char theRest[0x1D4];
	// there's a wheel health, tyre health, suspension health somewhere around here
}; // size = 0x210 (probably)
