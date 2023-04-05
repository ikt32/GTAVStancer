#pragma once

namespace SuspensionOffsets {
    //0x000 - ?
    //0x004 - toe
    const int Camber = 0x008;
    //0x00C - padding? Can't see difference + no change @ bump
    const int CamberInv = 0x010; // 
    //0x014 - offsetYPos but also affects camber?
    //0x018 - height-related?
    //0x01C - padding? Can't see difference + no change @ bump
    const int TrackWidth2 = 0x020; // Same as 0x030, can't see difference tho
    //0x024 - ???
    //0x02C - padding? Can't see difference + no change @ bump
    //0x028 - height + 0x128?
    const int TrackWidth = 0x030;
    const int YPos = 0x034;
    const int Height = 0x038; // affected by hydraulics! 0x028 also.
    //0x03C - padding? Can't see difference + no change @ bump
    //0x128 - physics-related?
    //0x12C - ??
    //0x130 - stiffness?

    const int TyreRadius = 0x110;
    const int RimRadius = 0x114;
    const int TyreWidth = 0x118;
}

namespace VisualOffsets {
    const int Size = 0x008;
    const int Width = 0xB80; // 0xBA0?
}
