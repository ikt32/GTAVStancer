#pragma once


//0x000 - ?
//0x004 - toe
const int offsetCamber = 0x008;
//0x00C - padding? Can't see difference + no change @ bump
const int offsetCamberInv = 0x010; // 
//0x014 - offsetYPos but also affects camber?
//0x018 - height-related?
//0x01C - padding? Can't see difference + no change @ bump
const int offsetTrackWidth2 = 0x020; // Same as 0x030, can't see difference tho
//0x024 - ???
//0x02C - padding? Can't see difference + no change @ bump
//0x028 - height + 0x128?
const int offsetTrackWidth = 0x030;
const int offsetYPos = 0x034;
const int offsetHeight = 0x038; // affected by hydraulics! 0x028 also.
//0x03C - padding? Can't see difference + no change @ bump
//0x128 - physics-related?
//0x12C - ??
//0x130 - stiffness?

const int offTyreRadius = 0x110;
const int offRimRadius = 0x114;
const int offTyreWidth = 0x118;

