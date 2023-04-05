#include "VehicleExtensions.hpp"

#include "NativeMemory.hpp"
#include "Offsets.hpp"
#include "SuspensionOffsets.hpp"
#include "VehicleFlags.hpp"
#include "Versions.hpp"

#include "../Util/Logger.hpp"

#include <inc/main.h>

#include <vector>
#include <functional>

// <= b1493: 8  (Top gear = 7)
// >= b1604: 11 (Top gear = 10)
uint8_t g_numGears = 8;

eGameVersion g_gameVersion = getGameVersion();

namespace {
    template <typename T> T Sign(T val) {
        return static_cast<T>((T{} < val) - (val < T{}));
    }

    int rocketBoostActiveOffset = 0;
    int rocketBoostChargeOffset = 0;
    int hoverTransformRatioOffset = 0;
    int hoverTransformRatioLerpOffset = 0;
    int fuelLevelOffset = 0;
    int lightsBrokenOffset = 0;
    int lightsBrokenVisuallyOffset = 0;
    int nextGearOffset = 0;
    int currentGearOffset = 0;
    int topGearOffset = 0;
    int gearRatiosOffset = 0;
    int driveForceOffset = 0;
    int initialDriveMaxFlatVelOffset = 0;
    int driveMaxFlatVelOffset = 0;
    int currentRPMOffset = 0;
    int clutchOffset = 0;
    int throttleOffset = 0;
    int turboOffset = 0;
    int arenaBoostOffset = 0;
    int handlingOffset = 0;
    int lightStatesOffset = 0;
    int indicatorTimingOffset = 0;
    int steeringAngleInputOffset = 0;
    int steeringAngleOffset = 0;
    int throttlePOffset = 0;
    int brakePOffset = 0;
    int handbrakeOffset = 0;
    int dirtLevelOffset = 0;
    int engineTempOffset = 0;
    int dashSpeedOffset = 0;
    int wheelsPtrOffset = 0;
    int numWheelsOffset = 0;
    int modelTypeOffset = 0;
    int vehicleModelInfoOffset = 0x020;
    int vehicleFlagsOffset = 0;

    int wheelSteerMultOffset = 0;

    int gravityOffset = 0;

    // Wheel stuff
    int wheelHealthOffset = 0;
    int wheelSuspensionCompressionOffset = 0;
    int wheelSteeringAngleOffset = 0;
    int wheelAngularVelocityOffset = 0;
    int wheelOverheatOffset = 0;
    int wheelTractionVectorLengthOffset = 0;
    int wheelTractionVectorYOffset = 0;
    int wheelTractionVectorXOffset = 0;
    int wheelPowerOffset = 0;
    int wheelBrakeOffset = 0;
    int wheelFlagsOffset = 0;
    int wheelDownforceOffset = 0;
    int wheelLoadOffset = 0;

    int wheelMatTyreGripOffset = 0;
    int wheelMatWetGripOffset = 0;
    int wheelMatTyreDragOffset = 0;
    int wheelMatTopSpeedMultOffset = 0;
    int wheelMatTypeOffset = 0;

    // Rendering
    // About time I yoinked something back from FiveM.
    int StreamRenderGfxPtrOffset = 0;
    int DrawHandlerPtrOffset = 0;

    uint32_t StreamRenderWheelWidthOffset = 0;
    uint8_t StreamRenderWheelSizeOffset = 0;
}

void VehicleExtensions::SetVersion(int version) {
    g_gameVersion = static_cast<eGameVersion>(version);
    if (g_gameVersion >= G_VER_1_0_1604_0_STEAM) {
        g_numGears = 11;
    }
}

uint8_t VehicleExtensions::GearsAvailable() {
    return g_numGears;
}

/*
 * Offsets/patterns done by me might need revision, but they've been checked
 * against b1180.2 and b877.1 and are okay.
 */
void VehicleExtensions::Init() {
    mem::init();

    // Figuring out indicator timing: LieutenantDan
    uintptr_t addr = mem::FindPattern("\x44\x0F\xB7\x91\xDC\x00\x00\x00\x0F\xB7\x81\xB0\x0A\x00\x00\x41\xB9\x01\x00\x00\x00\x44\x03\x15\x8C\x63\xDF\x01",
        "xxxx????xxx????xxxxxxxxx????");
    indicatorTimingOffset = addr == 0 ? 0 : *(int*)(addr + 4);
    LOG(indicatorTimingOffset == 0 ? WARN : DEBUG, "Indicator timing offset: 0x{:03X}", indicatorTimingOffset);

    addr = mem::FindPattern("3A 91 ? ? ? ? 74 ? 84 D2");
    rocketBoostActiveOffset = addr == 0 ? 0 : *(int*)(addr + 2);
    LOG(rocketBoostActiveOffset == 0 ? WARN : DEBUG, "Rocket Boost Active Offset: 0x{:X}", rocketBoostActiveOffset);

    addr = mem::FindPattern("\x48\x8B\x47\x00\xF3\x44\x0F\x10\x9F\x00\x00\x00\x00", "xxx?xxxxx????");
    rocketBoostChargeOffset = addr == 0 ? 0 : *(int*)(addr + 9);
    LOG(rocketBoostChargeOffset == 0 ? WARN : DEBUG, "Rocket Boost Charge Offset: 0x{:X}", rocketBoostChargeOffset);

    // Unknown
    addr = mem::FindPattern("\xF3\x0F\x11\xB3\x00\x00\x00\x00\x44\x88\x00\x00\x00\x00\x00\x48\x85\xC9",
        "xxxx????xx?????xxx");
    hoverTransformRatioOffset = addr == 0 ? 0 : *(int*)(addr + 4);
    LOG(hoverTransformRatioOffset == 0 ? WARN : DEBUG, "Hover Transform Active Offset: 0x{:X}", hoverTransformRatioOffset);

    //addr = mem::FindPattern("\xF3\x0F\x11\xB3\x00\x00\x00\x00\x44\x88\x00\x00\x00\x00\x00\x48\x85\xC9",
    //    "xxxx????xx?????xxx");
    hoverTransformRatioLerpOffset = addr == 0 ? 0 : *(int*)(addr + 4) + 0x28;
    LOG(hoverTransformRatioLerpOffset == 0 ? WARN : DEBUG, "Hover Transform Ratio Offset: 0x{:X}", hoverTransformRatioLerpOffset);

    addr = mem::FindPattern("\x48\x85\xC0\x74\x3C\x8B\x80\x00\x00\x00\x00\xC1\xE8\x0F", "xxxxxxx????xxx");
    vehicleFlagsOffset = addr == 0 ? 0 : *(int*)(addr + 7);
    LOG(vehicleFlagsOffset == 0 ? WARN : DEBUG, "Vehicle Flags Offset: 0x{:X}", vehicleFlagsOffset);

    addr = mem::FindPattern("\x74\x26\x0F\x57\xC9", "xxxxx");
    fuelLevelOffset = addr == 0 ? 0 : *(int*)(addr + 8);
    LOG(fuelLevelOffset == 0 ? WARN : DEBUG, "Fuel Level Offset: 0x{:X}", fuelLevelOffset);

    // 86C -> bulb
    addr = mem::FindPattern("F6 87 ? ? ? ? 02 75 06 C6 45 80 01");
    lightsBrokenOffset = addr == 0 ? 0 : *(int*)(addr + 2);;
    LOG(lightsBrokenOffset == 0 ? WARN : DEBUG, "Lights Broken Offset: 0x{:X}", lightsBrokenOffset);

    // 874 -> visibly
    lightsBrokenVisuallyOffset = addr == 0 ? 0 : lightsBrokenOffset + 8;
    LOG(lightsBrokenVisuallyOffset == 0 ? WARN : DEBUG, "Lights Visually Broken Offset: 0x{:X}", lightsBrokenVisuallyOffset);

    addr = mem::FindPattern("\x48\x8D\x8F\x00\x00\x00\x00\x4C\x8B\xC3\xF3\x0F\x11\x7C\x24",
        "xxx????xxxxxxxx");
    nextGearOffset = addr == 0 ? 0 : *(int*)(addr + 3);
    LOG(nextGearOffset == 0 ? WARN : DEBUG, "Next Gear Offset: 0x{:X}", nextGearOffset);

    currentGearOffset = addr == 0 ? 0 : *(int*)(addr + 3) + 2;
    LOG(currentGearOffset == 0 ? WARN : DEBUG, "Current Gear Offset: 0x{:X}", currentGearOffset);

    topGearOffset = addr == 0 ? 0 : *(int*)(addr + 3) + 6;
    LOG(topGearOffset == 0 ? WARN : DEBUG, "Top Gear Offset: 0x{:X}", topGearOffset);

    gearRatiosOffset = addr == 0 ? 0 : *(int*)(addr + 3) + 8;
    LOG(gearRatiosOffset == 0 ? WARN : DEBUG, "Gear Ratios Offset: 0x{:X}", gearRatiosOffset);

    if (g_gameVersion >= G_VER_1_0_1604_0_STEAM) {
        addr = mem::FindPattern("\xF3\x0F\x10\x8F\xA4\x08\x00\x00\xF3\x0F\x5E\xF0\x41\x0F\x2F\xCA", "xxxx????xxx?xxx?");
        driveForceOffset = addr == 0 ? 0 : *(int*)(addr + 4);
    }
    else {
        driveForceOffset = addr == 0 ? 0 : *(int*)(addr + 3) + 0x28;
    }
    LOG(driveForceOffset == 0 ? WARN : DEBUG, "Drive Force Offset: 0x{:X}", driveForceOffset);

    initialDriveMaxFlatVelOffset = driveForceOffset == 0 ? 0 : driveForceOffset + 0x04;
    LOG(initialDriveMaxFlatVelOffset == 0 ? WARN : DEBUG, "Initial Drive Max Flat Velocity Offset: 0x{:X}", initialDriveMaxFlatVelOffset);

    driveMaxFlatVelOffset = driveForceOffset == 0 ? 0 : driveForceOffset + 0x08;
    LOG(driveMaxFlatVelOffset == 0 ? WARN : DEBUG, "Drive Max Flat Velocity Offset: 0x{:X}", driveMaxFlatVelOffset);

    addr = mem::FindPattern("\x76\x03\x0F\x28\xF0\xF3\x44\x0F\x10\x93",
        "xxxxxxxxxx");
    currentRPMOffset = addr == 0 ? 0 : *(int*)(addr + 10);
    LOG(currentRPMOffset == 0 ? WARN : DEBUG, "RPM Offset: 0x{:X}", currentRPMOffset);

    clutchOffset = addr == 0 ? 0 : *(int*)(addr + 10) + 0xC;
    LOG(clutchOffset == 0 ? WARN : DEBUG, "Clutch Offset: 0x{:X}", clutchOffset);

    throttleOffset = addr == 0 ? 0 : *(int*)(addr + 10) + 0x10;
    LOG(throttleOffset == 0 ? WARN : DEBUG, "Throttle Offset: 0x{:X}", throttleOffset);

    if (g_gameVersion >= G_VER_1_0_1604_0_STEAM) {
        addr = mem::FindPattern("\xF3\x0F\x10\x9F\xD4\x08\x00\x00\x0F\x2F\xDF\x73\x0A", "xxxx????xxxxx");
    }
    else {
        addr = mem::FindPattern("\xF3\x0F\x10\x8F\x68\x08\x00\x00\x88\x4D\x8C\x0F\x2F\xCF",
            "xxxx????xxx???");
    }
    turboOffset = addr == 0 ? 0 : *(int*)(addr + 4);
    LOG(turboOffset == 0 ? WARN : DEBUG, "Turbo Offset: 0x{:X}", turboOffset);

    if (g_gameVersion >= G_VER_1_0_1604_0_STEAM) {
        // TODO: pattern
        arenaBoostOffset = turboOffset + 0x30;
    }
    else {
        arenaBoostOffset = 0;
    }

    addr = mem::FindPattern("\x3C\x03\x0F\x85\x00\x00\x00\x00\x48\x8B\x41\x20\x48\x8B\x88",
        "xxxx????xxxxxxx");
    handlingOffset = addr == 0 ? 0 : *(int*)(addr + 0x16);
    LOG(handlingOffset == 0 ? WARN : DEBUG, "Handling Offset: 0x{:X}", handlingOffset);

    addr = mem::FindPattern("FD 02 DB 08 98 ? ? ? ? 48 8B 5C 24 30");
    lightStatesOffset = addr == 0 ? 0 : *(int*)(addr - 4) - 1;
    LOG(lightStatesOffset == 0 ? WARN : DEBUG, "Light States Offset: 0x{:X}", lightStatesOffset);
    // Or "8A 96 ? ? ? ? 0F B6 C8 84 D2 41", +10 or something (+31 is the engine starting bit), (0x928 starting addr)

    addr = mem::FindPattern("\x74\x0A\xF3\x0F\x11\xB3\x1C\x09\x00\x00\xEB\x25", "xxxxxx????xx");
    steeringAngleInputOffset = addr == 0 ? 0 : *(int*)(addr + 6);
    LOG(steeringAngleInputOffset == 0 ? WARN : DEBUG, "Steering Input Offset: 0x{:X}", steeringAngleInputOffset);

    steeringAngleOffset = addr == 0 ? 0 : *(int*)(addr + 6) + 8;
    LOG(steeringAngleOffset == 0 ? WARN : DEBUG, "Steering Angle Offset: 0x{:X}", steeringAngleOffset);

    throttlePOffset = addr == 0 ? 0 : *(int*)(addr + 6) + 0x10;
    LOG(throttlePOffset == 0 ? WARN : DEBUG, "ThrottleP Offset: 0x{:X}", throttlePOffset);

    brakePOffset = addr == 0 ? 0 : *(int*)(addr + 6) + 0x14;
    LOG(brakePOffset == 0 ? WARN : DEBUG, "BrakeP Offset: 0x{:X}", brakePOffset);

    if (g_gameVersion >= G_VER_1_0_2060_0_STEAM) {
        addr = mem::FindPattern("8A C2 24 01 C0 E0 04 08 81");
        handbrakeOffset = addr == 0 ? 0 : *(int*)(addr + 19);
    }
    else {
        addr = mem::FindPattern("\x44\x88\xA3\x00\x00\x00\x00\x45\x8A\xF4", "xxx????xxx");
        handbrakeOffset = addr == 0 ? 0 : *(int*)(addr + 3);
    }
    LOG(handbrakeOffset == 0 ? WARN : DEBUG, "Handbrake Offset: 0x{:X}", handbrakeOffset);

    addr = mem::FindPattern("\x0F\x29\x7C\x24\x30\x0F\x85\xE3\x00\x00\x00\xF3\x0F\x10\xB9\x68\x09\x00\x00",
        "xx???xx????xxxx????");
    dirtLevelOffset = addr == 0 ? 0 : *(int*)(addr + 0xF);
    LOG(dirtLevelOffset == 0 ? WARN : DEBUG, "Dirt Level Offset: 0x{:X}", dirtLevelOffset);

    addr = mem::FindPattern("\xF3\x0F\x11\x9B\xDC\x09\x00\x00\x0F\x84\xB1\x00\x00\x00",
        "xxxx????xxx???");
    engineTempOffset = addr == 0 ? 0 : *(int*)(addr + 4);
    LOG(engineTempOffset == 0 ? WARN : DEBUG, "Engine Temperature Offset: 0x{:X}", engineTempOffset);

    addr = mem::FindPattern("\xF3\x0F\x10\x8F\x10\x0A\x00\x00\xF3\x0F\x59\x05\x5E\x30\x8D\x00",
        "xxxx????xxxx????");
    dashSpeedOffset = addr == 0 ? 0 : *(int*)(addr + 4);
    LOG(dashSpeedOffset == 0 ? WARN : DEBUG, "Dashboard Speed Offset: 0x{:X}", dashSpeedOffset);

    addr = mem::FindPattern("\x8B\x83\x38\x0B\x00\x00\x83\xE8\x08\x83\xF8\x02", "xx????xx?xxx");
    modelTypeOffset = addr == 0 ? 0 : *(int*)(addr + 2);
    LOG(modelTypeOffset == 0 ? WARN : DEBUG, "Model Type Offset: 0x{:X}", modelTypeOffset);

    addr = mem::FindPattern("\x3B\xB7\x48\x0B\x00\x00\x7D\x0D", "xx????xx");
    wheelsPtrOffset = addr == 0 ? 0 : *(int*)(addr + 2) - 8;
    LOG(wheelsPtrOffset == 0 ? WARN : DEBUG, "Wheels Pointer Offset: 0x{:X}", wheelsPtrOffset);

    numWheelsOffset = addr == 0 ? 0 : *(int*)(addr + 2);
    LOG(numWheelsOffset == 0 ? WARN : DEBUG, "Wheel Count Offset: 0x{:X}", numWheelsOffset);

    addr = mem::FindPattern("F3 0F 59 BF ? ? ? ? 4D 85 E4 0F 8E ? ? ? ?");
    gravityOffset = addr == 0 ? 0 : *(int*)(addr + 4);
    LOG(gravityOffset == 0 ? WARN : DEBUG, "Gravity Offset: 0x{:X}", gravityOffset);

    // Wheel stuff offset from here

    addr = mem::FindPattern("\x0F\xBA\xAB\xEC\x01\x00\x00\x09\x0F\x2F\xB3\x40\x01\x00\x00\x48\x8B\x83\x20\x01\x00\x00",
        "xx?????xxx???xxxx?????");
    wheelSteerMultOffset = addr == 0 ? 0 : *(int*)(addr + 11);
    LOG(wheelSteerMultOffset == 0 ? WARN : DEBUG, "Wheel Steering Multiplier Offset: 0x{:X}", wheelSteerMultOffset);

    addr = mem::FindPattern("\x45\x0f\x57\xc9\xf3\x0f\x11\x83\x60\x01\x00\x00\xf3\x0f\x5c", "xxx?xxx???xxxxx");
    wheelSuspensionCompressionOffset = addr == 0 ? 0 : *(int*)(addr + 8);
    LOG(wheelSuspensionCompressionOffset == 0 ? WARN : DEBUG, "Wheel Suspension Compression Offset: 0x{:X}", wheelSuspensionCompressionOffset);

    wheelAngularVelocityOffset = addr == 0 ? 0 : (*(int*)(addr + 8)) + 0xc;
    LOG(wheelAngularVelocityOffset == 0 ? WARN : DEBUG, "Wheel Angular Velocity Offset: 0x{:X}", wheelAngularVelocityOffset);

    // angular velocity offset + 0x08
    wheelOverheatOffset = addr == 0 ? 0 : (*(int*)(addr + 8)) + 0xc + 0x08;
    LOG(wheelOverheatOffset == 0 ? WARN : DEBUG, "Wheel Overheat Offset: 0x{:X}", wheelOverheatOffset);

    // Material stuff only tested for b2245
    addr = mem::FindPattern("89 8B ? ? 00 00 E8 ? ? ? ? 0F 57 ?");
    wheelMatTyreGripOffset = addr == 0 ? 0 : (*(int*)(addr + 2));
    LOG(wheelMatTyreGripOffset == 0 ? WARN : DEBUG, "Wheel Material TYRE_GRIP Offset: 0x{:X}", wheelMatTyreGripOffset);

    wheelMatWetGripOffset = addr == 0 ? 0 : (*(int*)(addr + 2) + 4);
    LOG(wheelMatWetGripOffset == 0 ? WARN : DEBUG, "Wheel Material WET_GRIP Offset: 0x{:X}", wheelMatWetGripOffset);

    wheelMatTyreDragOffset = addr == 0 ? 0 : (*(int*)(addr + 2) + 8);
    LOG(wheelMatTyreDragOffset == 0 ? WARN : DEBUG, "Wheel Material TYRE_DRAG Offset: 0x{:X}", wheelMatTyreDragOffset);

    wheelMatTopSpeedMultOffset = addr == 0 ? 0 : (*(int*)(addr + 2) + 12);
    LOG(wheelMatTopSpeedMultOffset == 0 ? WARN : DEBUG, "Wheel Material TOP_SPEED_MULT Offset: 0x{:X}", wheelMatTopSpeedMultOffset);

    if (g_gameVersion >= G_VER_1_0_1737_0_STEAM) {
        addr = mem::FindPattern("\x0F\x2F\x81\xBC\x01\x00\x00" "\x0F\x97\xC0" "\xEB\x00" "\xD1\x00", "xx???xx" "xxx" "x?" "x?");
    }
    else {
        addr = mem::FindPattern("\x0F\x2F\x81\xBC\x01\x00\x00" "\x0F\x97\xC0\xEB\xDA", "xx???xx" "xxxxx");
    }

    wheelTractionVectorLengthOffset = addr == 0 ? 0 : (*(int*)(addr + 3)) - 0x14;
    LOG(wheelTractionVectorLengthOffset == 0 ? WARN : DEBUG, "Wheel Traction Vector Length Offset: 0x{:X}", wheelTractionVectorLengthOffset);

    wheelLoadOffset = addr == 0 ? 0 : *(int*)(addr + 3) - 0x10;
    LOG(wheelLoadOffset == 0 ? WARN : DEBUG, "Wheel Load Offset: 0x{:X}", wheelLoadOffset);

    wheelTractionVectorYOffset = addr == 0 ? 0 : (*(int*)(addr + 3)) - 0x0C;
    LOG(wheelTractionVectorYOffset == 0 ? WARN : DEBUG, "Wheel Traction Vector Y Offset: 0x{:X}", wheelTractionVectorYOffset);

    wheelTractionVectorXOffset = addr == 0 ? 0 : (*(int*)(addr + 3)) - 0x08;
    LOG(wheelTractionVectorXOffset == 0 ? WARN : DEBUG, "Wheel Traction Vector X Offset: 0x{:X}", wheelTractionVectorXOffset);

    wheelSteeringAngleOffset = addr == 0 ? 0 : *(int*)(addr + 3);
    LOG(wheelSteeringAngleOffset == 0 ? WARN : DEBUG, "Wheel Steering Angle Offset: 0x{:X}", wheelSteeringAngleOffset);

    wheelBrakeOffset = addr == 0 ? 0 : (*(int*)(addr + 3)) + 0x4;
    LOG(wheelBrakeOffset == 0 ? WARN : DEBUG, "Wheel Brake Offset: 0x{:X}", wheelBrakeOffset);

    wheelPowerOffset = addr == 0 ? 0 : (*(int*)(addr + 3)) + 0x8;
    LOG(wheelPowerOffset == 0 ? WARN : DEBUG, "Wheel Power Offset: 0x{:X}", wheelPowerOffset);

    addr = mem::FindPattern("\x75\x24\xF3\x0F\x10\x81\xE0\x01\x00\x00\xF3\x0F\x5C\xC1", "xxxxx???xxxx??");
    wheelHealthOffset = addr == 0 ? 0 : *(int*)(addr + 6);
    LOG(wheelHealthOffset == 0 ? WARN : DEBUG, "Wheel Health Offset: 0x{:X}", wheelHealthOffset);

    // wheelHealthOffset + float = tyre health

    addr = mem::FindPattern("\x75\x11\x48\x8b\x01\x8b\x88", "xxxxxxx");
    wheelFlagsOffset = addr == 0 ? 0 : *(int*)(addr + 7);
    LOG(wheelFlagsOffset == 0 ? WARN : DEBUG, "Wheel Flags Offset: 0x{:X}", wheelFlagsOffset);

    addr = mem::FindPattern("88 8B ? ? 00 00 41 0F B6 47 51 66 89 83 ? ? 00 00");
    wheelMatTypeOffset = addr == 0 ? 0 : (*(int*)(addr + 2));
    LOG(wheelMatTypeOffset == 0 ? WARN : DEBUG, "Wheel Material Type Offset: 0x{:X}", wheelMatTypeOffset);

    wheelDownforceOffset = addr == 0 ? 0 : *(int*)(addr + 2) + 0x16;
    LOG(wheelDownforceOffset == 0 ? WARN : DEBUG, "Wheel Downforce Offset: 0x{:X}", wheelDownforceOffset);

    addr = mem::FindPattern("4C 8D 48 ? 80 E1 01");
    StreamRenderGfxPtrOffset = addr == 0 ? 0 : *(uint32_t*)(addr - 4);
    LOG(StreamRenderGfxPtrOffset == 0 ? WARN : DEBUG, "StreamRenderGfxPtrOffset: 0x{:X}", StreamRenderGfxPtrOffset);

    addr = mem::FindPattern("44 0F 2F 43 48 45 8D");
    DrawHandlerPtrOffset = addr == 0 ? 0 : *(uint8_t*)(addr + 4);
    LOG(DrawHandlerPtrOffset == 0 ? WARN : DEBUG, "DrawHandlerPtrOffset: 0x{:X}", DrawHandlerPtrOffset);

    addr = mem::FindPattern("48 89 01 B8 00 00 80 3F 66 44 89 51");
    StreamRenderWheelWidthOffset = addr == 0 ? 0 : *(uint32_t*)(addr + 23);
    LOG(StreamRenderWheelWidthOffset == 0 ? WARN : DEBUG, "StreamRenderWheelWidthOffset: 0x{:X}", StreamRenderWheelWidthOffset);

    StreamRenderWheelSizeOffset = addr == 0 ? 0 : *(uint8_t*)(addr + 20);
    LOG(StreamRenderWheelSizeOffset == 0 ? WARN : DEBUG, "StreamRenderWheelSizeOffset: 0x{:X}", StreamRenderWheelSizeOffset);
}

BYTE* VehicleExtensions::GetAddress(Vehicle handle) {
    return reinterpret_cast<BYTE*>(mem::GetAddressOfEntity(handle));
}

bool VehicleExtensions::GetRocketBoostActive(Vehicle handle) {
    if (rocketBoostActiveOffset == 0) return false;
    return *reinterpret_cast<bool*>(GetAddress(handle) + rocketBoostActiveOffset);
}

void VehicleExtensions::SetRocketBoostActive(Vehicle handle, bool val) {
    if (rocketBoostActiveOffset == 0) return;
    *reinterpret_cast<bool*>(GetAddress(handle) + rocketBoostActiveOffset) = val;
}

float VehicleExtensions::GetRocketBoostCharge(Vehicle handle) {
    if (rocketBoostChargeOffset == 0) return 0.0f;
    return *reinterpret_cast<float*>(GetAddress(handle) + rocketBoostChargeOffset);
}

void VehicleExtensions::SetRocketBoostCharge(Vehicle handle, float value) {
    if (rocketBoostChargeOffset == 0) return;
    *reinterpret_cast<float*>(GetAddress(handle) + rocketBoostChargeOffset) = value;
}

float VehicleExtensions::GetHoverTransformRatio(Vehicle handle) {
    if (hoverTransformRatioOffset == 0) return false;
    return *reinterpret_cast<float*>(GetAddress(handle) + hoverTransformRatioOffset);
}

void VehicleExtensions::SetHoverTransformRatio(Vehicle handle, float value) {
    if (hoverTransformRatioOffset == 0) return;
    *reinterpret_cast<float*>(GetAddress(handle) + hoverTransformRatioOffset) = value;
}

float VehicleExtensions::GetHoverTransformRatioLerp(Vehicle handle) {
    if (hoverTransformRatioLerpOffset == 0) return 0.0f;
    return *reinterpret_cast<float*>(GetAddress(handle) + hoverTransformRatioLerpOffset);
}

void VehicleExtensions::SetHoverTransformRatioLerp(Vehicle handle, float value) {
    if (hoverTransformRatioLerpOffset == 0) return;
    *reinterpret_cast<float*>(GetAddress(handle) + hoverTransformRatioLerpOffset) = value;
}

float VehicleExtensions::GetFuelLevel(Vehicle handle) {
    if (fuelLevelOffset == 0) return 0.0f;
    return *reinterpret_cast<float*>(GetAddress(handle) + fuelLevelOffset);
}

void VehicleExtensions::SetFuelLevel(Vehicle handle, float value) {
    if (fuelLevelOffset == 0) return;
    *reinterpret_cast<float*>(GetAddress(handle) + fuelLevelOffset) = value;
}

uint32_t VehicleExtensions::GetLightsBroken(Vehicle handle) {
    if (lightsBrokenOffset == 0) return 0;
    auto address = GetAddress(handle);
    return *reinterpret_cast<uint32_t*>(address + lightsBrokenOffset);
}

void VehicleExtensions::SetLightsBroken(Vehicle handle, uint32_t value) {
    if (lightsBrokenOffset == 0) return;
    auto address = GetAddress(handle);
    *reinterpret_cast<uint32_t*>(address + lightsBrokenOffset) = value;
}

uint32_t VehicleExtensions::GetLightsBrokenVisual(Vehicle handle) {
    if (lightsBrokenVisuallyOffset == 0) return 0;
    auto address = GetAddress(handle);
    return *reinterpret_cast<uint32_t*>(address + lightsBrokenVisuallyOffset);
}

void VehicleExtensions::SetLightsBrokenVisual(Vehicle handle, uint32_t value) {
    if (lightsBrokenVisuallyOffset == 0) return;
    auto address = GetAddress(handle);
    *reinterpret_cast<uint32_t*>(address + lightsBrokenVisuallyOffset) = value;
}

uint16_t VehicleExtensions::GetGearNext(Vehicle handle) {
    if (nextGearOffset == 0) return 0;
    return *reinterpret_cast<const uint16_t*>(GetAddress(handle) + nextGearOffset);
}

void VehicleExtensions::SetGearNext(Vehicle handle, uint16_t value) {
    if (nextGearOffset == 0) return;
    *reinterpret_cast<uint16_t*>(GetAddress(handle) + nextGearOffset) = value;
}

uint16_t VehicleExtensions::GetGearCurr(Vehicle handle) {
    if (currentGearOffset == 0) return 0;
    return *reinterpret_cast<const uint16_t*>(GetAddress(handle) + currentGearOffset);
}

void VehicleExtensions::SetGearCurr(Vehicle handle, uint16_t value) {
    if (currentGearOffset == 0) return;
    *reinterpret_cast<uint16_t*>(GetAddress(handle) + currentGearOffset) = value;
}

uint8_t VehicleExtensions::GetTopGear(Vehicle handle) {
    if (topGearOffset == 0) return 0;
    return *reinterpret_cast<uint8_t*>(GetAddress(handle) + topGearOffset);
}

void VehicleExtensions::SetTopGear(Vehicle handle, uint8_t value) {
    if (topGearOffset == 0) return;
    *reinterpret_cast<uint8_t*>(GetAddress(handle) + topGearOffset) = value;
}

float* VehicleExtensions::GetGearRatioPtr(Vehicle handle, uint8_t gear) {
    if (gearRatiosOffset == 0) return nullptr;
    return reinterpret_cast<float*>(
        GetAddress(handle) + gearRatiosOffset + gear * sizeof(float));
}

std::vector<float> VehicleExtensions::GetGearRatios(Vehicle handle) {
    if (gearRatiosOffset == 0) return {};
    auto address = GetAddress(handle);
    std::vector<float> ratios(GetTopGear(handle) + 1);
    for (int gear = 0; gear < GetTopGear(handle) + 1; ++gear) {
        ratios[gear] = *reinterpret_cast<float*>(address + gearRatiosOffset + gear * sizeof(float));
    }
    return ratios;
}

void VehicleExtensions::SetGearRatios(Vehicle handle, const std::vector<float>& values) {
    if (gearRatiosOffset == 0) return;
    auto address = GetAddress(handle);
    for (uint8_t gear = 0; gear < values.size(); ++gear) {
        *reinterpret_cast<float*>(address + gearRatiosOffset + gear * sizeof(float)) = values[gear];
    }
}

float VehicleExtensions::GetDriveForce(Vehicle handle) {
    if (driveForceOffset == 0) return 0.0f;
    return *reinterpret_cast<float*>(GetAddress(handle) + driveForceOffset);
}

void VehicleExtensions::SetDriveForce(Vehicle handle, float value) {
    if (driveForceOffset == 0) return;
    *reinterpret_cast<float*>(GetAddress(handle) + driveForceOffset) = value;
}

float VehicleExtensions::GetInitialDriveMaxFlatVel(Vehicle handle) {
    if (initialDriveMaxFlatVelOffset == 0) return 0.0f;
    return *reinterpret_cast<float*>(GetAddress(handle) + initialDriveMaxFlatVelOffset);
}

void VehicleExtensions::SetInitialDriveMaxFlatVel(Vehicle handle, float value) {
    if (initialDriveMaxFlatVelOffset == 0) return;
    *reinterpret_cast<float*>(GetAddress(handle) + initialDriveMaxFlatVelOffset) = value;
}

float VehicleExtensions::GetDriveMaxFlatVel(Vehicle handle) {
    if (driveMaxFlatVelOffset == 0) return 0.0f;
    return *reinterpret_cast<float*>(GetAddress(handle) + driveMaxFlatVelOffset);
}

void VehicleExtensions::SetDriveMaxFlatVel(Vehicle handle, float value) {
    if (driveMaxFlatVelOffset == 0) return;
    *reinterpret_cast<float*>(GetAddress(handle) + driveMaxFlatVelOffset) = value;
}

float VehicleExtensions::GetCurrentRPM(Vehicle handle) {
    if (currentRPMOffset == 0) return 0.0f;
    return *reinterpret_cast<const float*>(GetAddress(handle) + currentRPMOffset);
}

void VehicleExtensions::SetCurrentRPM(Vehicle handle, float value) {
    if (currentRPMOffset == 0) return;
    *reinterpret_cast<float*>(GetAddress(handle) + currentRPMOffset) = value;
}

float VehicleExtensions::GetClutch(Vehicle handle) {
    if (clutchOffset == 0) return 0.0f;
    auto address = GetAddress(handle);
    return address == nullptr ? 0 : *reinterpret_cast<const float*>(address + clutchOffset);
}

void VehicleExtensions::SetClutch(Vehicle handle, float value) {
    if (clutchOffset == 0) return;
    auto address = GetAddress(handle);
    *reinterpret_cast<float*>(address + clutchOffset) = value;
}

float VehicleExtensions::GetThrottle(Vehicle handle) {
    if (throttleOffset == 0) return 0.0f;
    auto address = GetAddress(handle);
    return *reinterpret_cast<float*>(address + throttleOffset);
}

// Seems to just control the sound.
void VehicleExtensions::SetThrottle(Vehicle handle, float value) {
    if (throttleOffset == 0) return;
    auto address = GetAddress(handle);
    *reinterpret_cast<float*>(address + throttleOffset) = value;
}

float VehicleExtensions::GetTurbo(Vehicle handle) {
    if (turboOffset == 0) return 0.0f;
    auto address = GetAddress(handle);
    return address == nullptr ? 0 : *reinterpret_cast<const float*>(address + turboOffset);
}

void VehicleExtensions::SetTurbo(Vehicle handle, float value) {
    if (turboOffset == 0) return;
    auto address = GetAddress(handle);
    *reinterpret_cast<float*>(address + turboOffset) = value;
}

float VehicleExtensions::GetArenaBoost(Vehicle handle) {
    if (arenaBoostOffset == 0) return 0.0f;
    auto address = GetAddress(handle);
    return address == nullptr ? 0 : *reinterpret_cast<const float*>(address + arenaBoostOffset);
}

void VehicleExtensions::SetArenaBoost(Vehicle handle, float value) {
    if (arenaBoostOffset == 0) return;
    auto address = GetAddress(handle);
    *reinterpret_cast<float*>(address + arenaBoostOffset) = value;
}

uint64_t VehicleExtensions::GetHandlingPtr(Vehicle handle) {
    if (handlingOffset == 0) return 0;
    auto address = GetAddress(handle);
    return *reinterpret_cast<uint64_t*>(address + handlingOffset);
}

void VehicleExtensions::SetHandlingPtr(Vehicle handle, uint64_t value) {
    if (handlingOffset == 0) return;
    auto address = GetAddress(handle);
    if (address == 0) return;
    *reinterpret_cast<uint64_t*>(address + handlingOffset) = value;
}

uint32_t VehicleExtensions::GetLightStates(Vehicle handle) {
    if (lightStatesOffset == 0) return 0;
    auto address = GetAddress(handle);
    return *reinterpret_cast<uint32_t*>(address + lightStatesOffset);
}

void VehicleExtensions::SetLightStates(Vehicle handle, uint32_t value) {
    if (lightStatesOffset == 0) return;
    auto address = GetAddress(handle);
    *reinterpret_cast<uint32_t*>(address + lightStatesOffset) = value;
}

bool VehicleExtensions::GetIndicatorHigh(Vehicle handle, int gameTime) {
    if (indicatorTimingOffset == 0) return false;
    auto address = GetAddress(handle);

    auto a = *reinterpret_cast<uint32_t*>(address + indicatorTimingOffset);
    a += (uint32_t)gameTime;
    a = a >> 9;
    a = a & 1;
    return a == 1;
}

float VehicleExtensions::GetGravity(Vehicle handle) {
    if (gravityOffset == 0) return 0.0f;
    auto address = GetAddress(handle);
    return *reinterpret_cast<float*>(address + gravityOffset);
}

void VehicleExtensions::SetGravity(Vehicle handle, float value) {
    if (gravityOffset == 0) return;
    auto address = GetAddress(handle);
    *reinterpret_cast<float*>(address + gravityOffset) = value;
}

float VehicleExtensions::GetSteeringInputAngle(Vehicle handle) {
    if (steeringAngleInputOffset == 0) return 0;
    auto address = GetAddress(handle);
    return *reinterpret_cast<float*>(address + steeringAngleInputOffset);
}

void VehicleExtensions::SetSteeringInputAngle(Vehicle handle, float value) {
    if (steeringAngleInputOffset == 0) return;
    auto address = GetAddress(handle);
    *reinterpret_cast<float*>(address + steeringAngleInputOffset) = value;
}

float VehicleExtensions::GetSteeringAngle(Vehicle handle) {
    if (steeringAngleOffset == 0) return 0;
    auto address = GetAddress(handle);
    return *reinterpret_cast<float*>(address + steeringAngleOffset);
}

void VehicleExtensions::SetSteeringAngle(Vehicle handle, float value) {
    if (steeringAngleOffset == 0) return;
    auto address = GetAddress(handle);
    *reinterpret_cast<float*>(address + steeringAngleOffset) = value;
}

float VehicleExtensions::GetThrottleP(Vehicle handle) {
    if (throttlePOffset == 0) return 0;
    auto address = GetAddress(handle);
    return *reinterpret_cast<float*>(address + throttlePOffset);
}

void VehicleExtensions::SetThrottleP(Vehicle handle, float value) {
    if (throttlePOffset == 0) return;
    auto address = GetAddress(handle);
    *reinterpret_cast<float*>(address + throttlePOffset) = value;
}

float VehicleExtensions::GetBrakeP(Vehicle handle) {
    if (brakePOffset == 0) return 0;
    auto address = GetAddress(handle);
    return *reinterpret_cast<float*>(address + brakePOffset);
}

void VehicleExtensions::SetBrakeP(Vehicle handle, float value) {
    if (brakePOffset == 0) return;
    auto address = GetAddress(handle);
    *reinterpret_cast<float*>(address + brakePOffset) = value;
}

bool VehicleExtensions::GetHandbrake(Vehicle handle) {
    if (handbrakeOffset == 0) return false;
    auto address = GetAddress(handle);
    return *reinterpret_cast<bool*>(address + handbrakeOffset);
}

void VehicleExtensions::SetHandbrake(Vehicle handle, bool value) {
    if (handbrakeOffset == 0) return;
    auto address = GetAddress(handle);
    *reinterpret_cast<bool*>(address + handbrakeOffset) = value;
}

float VehicleExtensions::GetDirtLevel(Vehicle handle) {
    if (dirtLevelOffset == 0) return 0;
    auto address = GetAddress(handle);
    return *reinterpret_cast<float*>(address + dirtLevelOffset);
}

float VehicleExtensions::GetEngineTemp(Vehicle handle) {
    if (engineTempOffset == 0) return 0;
    auto address = GetAddress(handle);
    return *reinterpret_cast<float*>(address + engineTempOffset);
}

float VehicleExtensions::GetDashSpeed(Vehicle handle) {
    if (dashSpeedOffset == 0) return 0;
    auto address = GetAddress(handle);
    return *reinterpret_cast<float*>(address + dashSpeedOffset);
}

int VehicleExtensions::GetModelType(Vehicle handle) {
    if (modelTypeOffset == 0) return 0;
    auto address = GetAddress(handle);
    return *reinterpret_cast<int*>(address + modelTypeOffset);
}

uint64_t VehicleExtensions::GetWheelsPtr(Vehicle handle) {
    if (wheelsPtrOffset == 0) return 0;
    auto address = GetAddress(handle);
    return *reinterpret_cast<uint64_t*>(address + wheelsPtrOffset);
}

uint8_t VehicleExtensions::GetNumWheels(Vehicle handle) {
    if (numWheelsOffset == 0) return 0;
    auto address = GetAddress(handle);
    if (address == 0) return 0;
    return *reinterpret_cast<int*>(address + numWheelsOffset);
}

float VehicleExtensions::GetDriveBiasFront(Vehicle handle) {
    auto address = GetHandlingPtr(handle);
    if (address == 0) return 0.0f;
    return *reinterpret_cast<float*>(address + hOffsets.fDriveBiasFront);
}

float VehicleExtensions::GetDriveBiasRear(Vehicle handle) {
    auto address = GetHandlingPtr(handle);
    if (address == 0) return 0.0f;
    return *reinterpret_cast<float*>(address + hOffsets.fDriveBiasRear);
}

float VehicleExtensions::GetPetrolTankVolume(Vehicle handle) {
    auto address = GetHandlingPtr(handle);
    if (address == 0) return 0.0f;
    return *reinterpret_cast<float*>(address + hOffsets.fPetrolTankVolume);
}

float VehicleExtensions::GetOilVolume(Vehicle handle) {
    auto address = GetHandlingPtr(handle);
    if (address == 0) return 0.0f;
    return *reinterpret_cast<float*>(address + hOffsets.fOilVolume);
}

float VehicleExtensions::GetMaxSteeringAngle(Vehicle handle) {
    auto address = GetHandlingPtr(handle);
    if (address == 0) return 0.0f;
    return *reinterpret_cast<float*>(address + hOffsets.fSteeringLock);
}

Hash VehicleExtensions::GetAIHandling(Vehicle handle) {
    auto address = GetHandlingPtr(handle);
    if (address == 0) return 0;
    auto offset = 0x13C;
    if (offset == 0) return 0;
    return *reinterpret_cast<Hash*>(address + offset);
}

std::vector<uint64_t> VehicleExtensions::GetWheelPtrs(Vehicle handle) {
    auto wheelPtr = GetWheelsPtr(handle);  // pointer to wheel pointers
    auto numWheels = GetNumWheels(handle);
    std::vector<uint64_t> wheelPtrs(numWheels);
    for (auto i = 0; i < numWheels; i++) {
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * i);
        wheelPtrs[i] = wheelAddr;
    }
    return wheelPtrs;
}

float VehicleExtensions::GetVisualHeight(Vehicle handle) {
    auto wheelPtr = GetWheelsPtr(handle);

    auto offset = g_gameVersion >= G_VER_1_0_944_2_STEAM ? 0x080 : 0;
    if (offset == 0)
        return 0.0f;

    return *reinterpret_cast<float*>(wheelPtr + offset);
}

void VehicleExtensions::SetVisualHeight(Vehicle handle, float height) {
    auto wheelPtr = GetWheelsPtr(handle);
    auto offset = g_gameVersion >= G_VER_1_0_944_2_STEAM ? 0x07C : 0;

    if (offset == 0)
        return;

    *reinterpret_cast<float*>(wheelPtr + offset) = height;
}

uint8_t VehicleExtensions::GetWheelIdMem(Vehicle handle, uint8_t index) {
    auto wheelPtr = GetWheelsPtr(handle);
    if (index > GetNumWheels(handle))
        return 0xFF; // Hope this doesn't backfire LMAO

    auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * index);
    return *reinterpret_cast<uint8_t*>(wheelAddr + 0x108); // pray to R* devs this doesn't change
}

std::vector<Vector3> VehicleExtensions::GetWheelBoneVelocity(Vehicle handle) {
    auto wheels = GetWheelPtrs(handle);
    std::vector<Vector3> values;

    values.reserve(wheels.size());
    for (auto wheelAddr : wheels) {
        values.emplace_back(Vector3{
            *reinterpret_cast<float*>(wheelAddr + 0xB0),
            *reinterpret_cast<float*>(wheelAddr + 0xB4),
            *reinterpret_cast<float*>(wheelAddr + 0xB8),
            });
    }

    return values;
}

std::vector<Vector3> VehicleExtensions::GetWheelTractionVector(Vehicle handle) {
    auto wheels = GetWheelPtrs(handle);
    std::vector<Vector3> values;

    values.reserve(wheels.size());
    for (auto wheelAddr : wheels) {
        values.emplace_back(Vector3{
            *reinterpret_cast<float*>(wheelAddr + 0xC0),
            *reinterpret_cast<float*>(wheelAddr + 0xC4),
            *reinterpret_cast<float*>(wheelAddr + 0xC8),
            });
    }

    return values;
}

std::vector<float> VehicleExtensions::GetWheelHealths(Vehicle handle) {
    auto wheelPtr = GetWheelsPtr(handle);
    auto numWheels = GetNumWheels(handle);

    std::vector<float> healths(numWheels);

    if (wheelHealthOffset == 0) return healths;

    for (auto i = 0; i < numWheels; i++) {
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * i);
        healths[i] = *reinterpret_cast<float*>(wheelAddr + wheelHealthOffset);
    }
    return healths;
}

void VehicleExtensions::SetWheelsHealth(Vehicle handle, float health) {
    if (wheelHealthOffset == 0) return;

    auto wheelPtr = GetWheelsPtr(handle);  // pointer to wheel pointers
    auto numWheels = GetNumWheels(handle);

    for (auto i = 0; i < numWheels; i++) {
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * i);
        *reinterpret_cast<float*>(wheelAddr + wheelHealthOffset) = health;
    }
}

std::vector<float> VehicleExtensions::GetWheelSteeringMultipliers(Vehicle handle) {
    auto wheelPtr = GetWheelsPtr(handle);
    std::vector<float> mults(GetNumWheels(handle));

    for (uint8_t i = 0; i < mults.size(); ++i) {
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * i);
        mults[i] = *reinterpret_cast<float*>(wheelAddr + wheelSteerMultOffset);
    }

    return mults;
}

void VehicleExtensions::SetWheelSteeringMultipliers(Vehicle handle, const std::vector<float>& values) {
    auto wheelPtr = GetWheelsPtr(handle);

    for (uint8_t i = 0; i < values.size(); i++) {
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * i);
        *reinterpret_cast<float*>(wheelAddr + wheelSteerMultOffset) = values[i];
    }
}

std::vector<Vector3> VehicleExtensions::GetWheelOffsets(Vehicle handle) {
    auto wheels = GetWheelPtrs(handle);
    std::vector<Vector3> positions;

    int offPosX = 0x20;
    int offPosY = 0x24;
    int offPosZ = 0x28;

    positions.reserve(wheels.size());
    for (auto wheelAddr : wheels) {
        positions.emplace_back(Vector3{
            *reinterpret_cast<float*>(wheelAddr + offPosX),
            *reinterpret_cast<float*>(wheelAddr + offPosY),
            *reinterpret_cast<float*>(wheelAddr + offPosZ),
            });
    }
    return positions;
}

std::vector<Vector3> VehicleExtensions::GetWheelLastContactCoords(Vehicle handle) {
    auto wheels = GetWheelPtrs(handle);
    std::vector<Vector3> positions;
    // 0x40: Last wheel contact coordinates
    // 0x50: Last wheel contact coordinates but centered on the wheel width
    // 0x60: Next probable wheel position? Seems to flutter around a lot, while
    //       position is entirely lost (0.0) when contact is lost. Wheels that
    //       steer emphasise this further, though acceleration/deceleration
    //       will also influence it.

    int offPosX = 0x40;
    int offPosY = 0x44;
    int offPosZ = 0x48;

    positions.reserve(wheels.size());
    for (auto wheelAddr : wheels) {
        positions.emplace_back(Vector3{
            *reinterpret_cast<float*>(wheelAddr + offPosX),
            *reinterpret_cast<float*>(wheelAddr + offPosY),
            *reinterpret_cast<float*>(wheelAddr + offPosZ),
            });
    }
    return positions;
}

std::vector<float> VehicleExtensions::GetWheelCompressions(Vehicle handle) {
    auto wheelPtr = GetWheelsPtr(handle);
    auto numWheels = GetNumWheels(handle);

    std::vector<float> compressions(numWheels);

    if (wheelSuspensionCompressionOffset == 0) return compressions;

    for (auto i = 0; i < numWheels; i++) {
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * i);
        compressions[i] = *reinterpret_cast<float*>(wheelAddr + wheelSuspensionCompressionOffset);
    }
    return compressions;
}

std::vector<float> VehicleExtensions::GetWheelSteeringAngles(Vehicle handle) {
    auto wheelPtr = GetWheelsPtr(handle);
    auto numWheels = GetNumWheels(handle);

    std::vector<float> angles(numWheels);

    if (wheelSteeringAngleOffset == 0) return angles;

    for (auto i = 0; i < numWheels; i++) {
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * i);
        angles[i] = *reinterpret_cast<float*>(wheelAddr + wheelSteeringAngleOffset);
    }
    return angles;
}

std::vector<bool> VehicleExtensions::GetWheelsOnGround(Vehicle handle) {
    auto compressions = GetWheelCompressions(handle);
    std::vector<bool> onGround;
    onGround.reserve(compressions.size());
    for (auto comp : compressions) {
        onGround.push_back(comp != 0.0f);
    }
    return onGround;
}

float VehicleExtensions::GetWheelLargestAngle(Vehicle handle) {
    float largestAngle = 0.0f;
    auto angles = GetWheelSteeringAngles(handle);

    for (auto angle : angles) {
        if (abs(angle) > abs(largestAngle)) {
            largestAngle = angle;
        }
    }
    return largestAngle;
}

float VehicleExtensions::GetWheelAverageAngle(Vehicle handle) {
    auto angles = GetWheelSteeringAngles(handle);
    auto mults = GetWheelSteeringMultipliers(handle);
    float wheelsSteered = 0.0f;
    float avgAngle = 0.0f;

    for (int i = 0; i < GetNumWheels(handle); i++) {
        if (IsWheelSteered(handle, i)) {
            wheelsSteered += 1.0f;

            float angle = angles[i];

            // Flip the sign, otherwise we get 0 average steering
            if (mults[i] < 0.0f)
                angle = -angle;
            avgAngle += angle;
        }
    }

    avgAngle /= wheelsSteered;
    return avgAngle;
}

std::vector<WheelDimensions> VehicleExtensions::GetWheelDimensions(Vehicle handle) {
    auto wheels = GetWheelPtrs(handle);

    std::vector<WheelDimensions> dimensionsSet;
    int offTyreRadius = 0x110;
    int offRimRadius = 0x114;
    int offTyreWidth = 0x118;

    for (auto wheelAddr : wheels) {
        if (!wheelAddr) continue;

        WheelDimensions dimensions;
        dimensions.TyreRadius = *reinterpret_cast<float*>(wheelAddr + offTyreRadius);
        dimensions.RimRadius = *reinterpret_cast<float*>(wheelAddr + offRimRadius);
        dimensions.TyreWidth = *reinterpret_cast<float*>(wheelAddr + offTyreWidth);
        dimensionsSet.push_back(dimensions);
    }
    return dimensionsSet;
}

std::vector<float> VehicleExtensions::GetWheelRotationSpeeds(Vehicle handle) {
    auto wheelPtr = GetWheelsPtr(handle);
    auto numWheels = GetNumWheels(handle);
    std::vector<float> speeds(numWheels);

    if (wheelAngularVelocityOffset == 0) return speeds;

    for (auto i = 0; i < numWheels; i++) {
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * i);
        speeds[i] = -*reinterpret_cast<float*>(wheelAddr + wheelAngularVelocityOffset);
    }
    return speeds;
}

void VehicleExtensions::SetWheelRotationSpeed(Vehicle handle, uint8_t index, float value) {
    if (index > GetNumWheels(handle)) return;
    if (wheelAngularVelocityOffset == 0) return;

    auto wheelPtr = GetWheelsPtr(handle);

    auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * index);
    *reinterpret_cast<float*>(wheelAddr + wheelAngularVelocityOffset) = value;
}

std::vector<float> VehicleExtensions::GetTyreSpeeds(Vehicle handle) {
    int numWheels = GetNumWheels(handle);
    std::vector<float> rotationSpeed = GetWheelRotationSpeeds(handle);
    std::vector<WheelDimensions> dimensionsSet = GetWheelDimensions(handle);
    std::vector<float> wheelSpeeds(numWheels);

    for (int i = 0; i < numWheels; i++) {
        wheelSpeeds[i] = rotationSpeed[i] * dimensionsSet[i].TyreRadius;
    }
    return wheelSpeeds;
}

void VehicleExtensions::SetWheelTractionVectorLength(Vehicle handle, uint8_t index, float value) {
    if (index > GetNumWheels(handle)) return;
    if (wheelTractionVectorLengthOffset == 0) return;

    auto wheelPtr = GetWheelsPtr(handle);

    auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * index);
    *reinterpret_cast<float*>(wheelAddr + wheelTractionVectorLengthOffset) = value;
}

std::vector<float> VehicleExtensions::GetWheelTractionVectorLength(Vehicle handle) {
    auto numWheels = GetNumWheels(handle);
    std::vector<float> values(numWheels);
    auto wheelPtr = GetWheelsPtr(handle);

    if (wheelTractionVectorLengthOffset == 0) return values;

    for (auto i = 0; i < numWheels; i++) {
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * i);
        values[i] = (-*reinterpret_cast<float*>(wheelAddr + wheelTractionVectorLengthOffset));
    }
    return values;
}

std::vector<float> VehicleExtensions::GetWheelTractionVectorY(Vehicle handle) {
    auto numWheels = GetNumWheels(handle);
    std::vector<float> values(numWheels);
    auto wheelPtr = GetWheelsPtr(handle);

    if (wheelTractionVectorYOffset == 0) return values;

    for (auto i = 0; i < numWheels; i++) {
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * i);
        values[i] = (-*reinterpret_cast<float*>(wheelAddr + wheelTractionVectorYOffset));
    }
    return values;
}

std::vector<float> VehicleExtensions::GetWheelTractionVectorX(Vehicle handle) {
    auto numWheels = GetNumWheels(handle);
    std::vector<float> values(numWheels);
    auto wheelPtr = GetWheelsPtr(handle);

    if (wheelTractionVectorXOffset == 0) return values;

    for (auto i = 0; i < numWheels; i++) {
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * i);
        values[i] = (-*reinterpret_cast<float*>(wheelAddr + wheelTractionVectorXOffset));
    }
    return values;
}

std::vector<float> VehicleExtensions::GetTyreGrips(Vehicle handle) {
    auto numWheels = GetNumWheels(handle);
    std::vector<float> values(numWheels);
    auto wheelPtr = GetWheelsPtr(handle);

    if (wheelMatTyreGripOffset == 0) return values;

    for (auto i = 0; i < numWheels; i++) {
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * i);
        values[i] = (*reinterpret_cast<float*>(wheelAddr + wheelMatTyreGripOffset));
    }
    return values;
}

std::vector<float> VehicleExtensions::GetWetGrips(Vehicle handle) {
    auto numWheels = GetNumWheels(handle);
    std::vector<float> values(numWheels);
    auto wheelPtr = GetWheelsPtr(handle);

    if (wheelMatWetGripOffset == 0) return values;

    for (auto i = 0; i < numWheels; i++) {
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * i);
        values[i] = (*reinterpret_cast<float*>(wheelAddr + wheelMatWetGripOffset));
    }
    return values;
}

std::vector<float> VehicleExtensions::GetTyreDrags(Vehicle handle) {
    auto numWheels = GetNumWheels(handle);
    std::vector<float> values(numWheels);
    auto wheelPtr = GetWheelsPtr(handle);

    if (wheelMatTyreDragOffset == 0) return values;

    for (auto i = 0; i < numWheels; i++) {
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * i);
        values[i] = (*reinterpret_cast<float*>(wheelAddr + wheelMatTyreDragOffset));
    }
    return values;
}

std::vector<float> VehicleExtensions::GetTopSpeedMults(Vehicle handle) {
    auto numWheels = GetNumWheels(handle);
    std::vector<float> values(numWheels);
    auto wheelPtr = GetWheelsPtr(handle);

    if (wheelMatTopSpeedMultOffset == 0) return values;

    for (auto i = 0; i < numWheels; i++) {
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * i);
        values[i] = (*reinterpret_cast<float*>(wheelAddr + wheelMatTopSpeedMultOffset));
    }
    return values;
}

std::vector<uint16_t> VehicleExtensions::GetTireContactMaterial(Vehicle handle) {
    auto numWheels = GetNumWheels(handle);
    std::vector<uint16_t> values(numWheels);
    auto wheelPtr = GetWheelsPtr(handle);

    if (wheelMatTypeOffset == 0) return values;

    for (auto i = 0; i < numWheels; i++) {
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * i);
        values[i] = (*reinterpret_cast<uint16_t*>(wheelAddr + wheelMatTypeOffset));
    }
    return values;
}

std::vector<float> VehicleExtensions::GetWheelPower(Vehicle handle) {
    auto numWheels = GetNumWheels(handle);
    auto wheelPtr = GetWheelsPtr(handle);

    std::vector<float> values(numWheels);

    if (wheelPowerOffset == 0) return values;

    for (auto i = 0; i < numWheels; ++i) {
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * i);
        values[i] = *reinterpret_cast<float*>(wheelAddr + wheelPowerOffset);
    }
    return values;
}

void VehicleExtensions::SetWheelPower(Vehicle handle, uint8_t index, float value) {
    if (index > GetNumWheels(handle)) return;
    if (wheelPowerOffset == 0) return;

    auto wheelPtr = GetWheelsPtr(handle);

    auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * index);
    *reinterpret_cast<float*>(wheelAddr + wheelPowerOffset) = value;
}

std::vector<float> VehicleExtensions::GetWheelBrakePressure(Vehicle handle) {
    const auto numWheels = GetNumWheels(handle);
    std::vector<float> values(numWheels);
    auto wheelPtr = GetWheelsPtr(handle);

    if (wheelBrakeOffset == 0) return values;

    for (auto i = 0; i < numWheels; ++i) {
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * i);
        values[i] = *reinterpret_cast<float*>(wheelAddr + wheelBrakeOffset);
    }
    return values;
}

void VehicleExtensions::SetWheelBrakePressure(Vehicle handle, uint8_t index, float value) {
    if (index > GetNumWheels(handle)) return;
    if (wheelBrakeOffset == 0) return;

    auto wheelPtr = GetWheelsPtr(handle);

    auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * index);
    *reinterpret_cast<float*>(wheelAddr + wheelBrakeOffset) = value;
}

bool VehicleExtensions::IsWheelSteered(Vehicle handle, uint8_t index) {
    if (index > GetNumWheels(handle)) return false;
    if (wheelFlagsOffset == 0) return false;

    auto wheelPtr = GetWheelsPtr(handle);
    auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * index);
    auto wheelFlags = *reinterpret_cast<uint32_t*>(wheelAddr + wheelFlagsOffset);
    return wheelFlags & eWheelFlag::FLAG_IS_STEERED;
}

bool VehicleExtensions::IsWheelPowered(Vehicle handle, uint8_t index) {
    if (index > GetNumWheels(handle)) return false;
    if (wheelFlagsOffset == 0) return false;

    auto wheelPtr = GetWheelsPtr(handle);
    auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * index);
    auto wheelFlags = *reinterpret_cast<uint32_t*>(wheelAddr + wheelFlagsOffset);
    return wheelFlags & eWheelFlag::FLAG_IS_DRIVEN;
}

std::vector<uint32_t> VehicleExtensions::GetWheelFlags(Vehicle handle) {
    const auto numWheels = GetNumWheels(handle);
    std::vector<uint32_t> flags(numWheels);
    auto wheelPtr = GetWheelsPtr(handle);

    if (wheelFlagsOffset == 0) return flags;

    for (auto i = 0; i < numWheels; ++i) {
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * i);
        flags[i] = *reinterpret_cast<uint32_t*>(wheelAddr + wheelFlagsOffset);
    }
    return flags;
}

std::vector<float> VehicleExtensions::GetWheelLoads(Vehicle handle) {
    auto wheelPtr = GetWheelsPtr(handle);
    auto numWheels = GetNumWheels(handle);
    std::vector<float> values(numWheels);

    if (wheelLoadOffset == 0) return values;

    for (auto i = 0; i < numWheels; i++) {
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * i);
        values[i] = *reinterpret_cast<float*>(wheelAddr + 0x1BC);
    }
    return values;
}

std::vector<float> VehicleExtensions::GetWheelDownforces(Vehicle handle) {
    auto wheelPtr = GetWheelsPtr(handle);
    auto numWheels = GetNumWheels(handle);
    std::vector<float> dfs(numWheels);

    if (wheelDownforceOffset == 0) return dfs;

    for (auto i = 0; i < numWheels; i++) {
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * i);
        dfs[i] = *reinterpret_cast<float*>(wheelAddr + wheelDownforceOffset);
    }
    return dfs;
}

std::vector<float> VehicleExtensions::GetWheelOverheats(Vehicle handle) {
    auto wheelPtr = GetWheelsPtr(handle);
    auto numWheels = GetNumWheels(handle);
    std::vector<float> vals(numWheels);

    if (wheelOverheatOffset == 0) return vals;

    for (auto i = 0; i < numWheels; i++) {
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * i);
        vals[i] = *reinterpret_cast<float*>(wheelAddr + wheelOverheatOffset);
    }
    return vals;
}

uint64_t VehicleExtensions::GetWheelHandlingPtr(Vehicle handle, uint8_t index) {
    if (handlingOffset == 0) return 0;

    auto wheelPtr = GetWheelsPtr(handle);
    auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * index);

    return *reinterpret_cast<uint64_t*>(wheelAddr + 0x120);
}

void VehicleExtensions::SetWheelHandlingPtr(Vehicle handle, uint8_t index, uint64_t value) {
    if (handlingOffset == 0) return;
    auto address = GetAddress(handle);
    if (address == 0) return;

    auto wheelPtr = GetWheelsPtr(handle);
    auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * index);

    *reinterpret_cast<uint64_t*>(wheelAddr + 0x120) = value;
}

std::vector<uint32_t> VehicleExtensions::GetVehicleFlags(Vehicle handle) {
    auto address = GetAddress(handle);

    if (!address)
        return std::vector<uint32_t>();

    std::vector<uint32_t> offs(6);

    auto pCVehicleModelInfo = *(uint64_t*)(address + vehicleModelInfoOffset);
    for (uint8_t i = 0; i < 6; i++) {
        offs[i] = *(uint32_t*)(pCVehicleModelInfo + vehicleFlagsOffset + sizeof(uint32_t) * i);
    }
    return offs;
}

float VehicleExtensions::GetMaxSteeringWheelAngle(Vehicle handle) {
    auto address = GetAddress(handle);

    if (!address)
        return 109.0f;

    auto pCVehicleModelInfo = *(uint64_t*)(address + vehicleModelInfoOffset);
    return *(float*)(pCVehicleModelInfo + 0x54C);
}

float VehicleExtensions::GetVehicleWheelWidth(Vehicle handle) {
    auto address = GetAddress(handle);

    auto drawHandler = *reinterpret_cast<uint64_t*>((uint64_t)address + DrawHandlerPtrOffset);
    auto streamRenderGfx = *reinterpret_cast<uint64_t*>(drawHandler + StreamRenderGfxPtrOffset);

    if (streamRenderGfx != 0) {
        return *(float*)(streamRenderGfx + StreamRenderWheelWidthOffset);
    }
    return 0.0f;
}

void VehicleExtensions::SetVehicleWheelWidth(Vehicle handle, float value) {
    auto address = GetAddress(handle);

    auto drawHandler = *reinterpret_cast<uint64_t*>((uint64_t)address + DrawHandlerPtrOffset);
    auto streamRenderGfx = *reinterpret_cast<uint64_t*>(drawHandler + StreamRenderGfxPtrOffset);

    if (streamRenderGfx != 0) {
        *(float*)(streamRenderGfx + StreamRenderWheelWidthOffset) = value;
    }
}

float VehicleExtensions::GetVehicleWheelSize(Vehicle handle) {
    auto address = GetAddress(handle);

    auto drawHandler = *reinterpret_cast<uint64_t*>((uint64_t)address + DrawHandlerPtrOffset);
    auto streamRenderGfx = *reinterpret_cast<uint64_t*>(drawHandler + StreamRenderGfxPtrOffset);

    if (streamRenderGfx != 0) {
        return *(float*)(streamRenderGfx + StreamRenderWheelSizeOffset);
    }
    return 0.0f;
}

void VehicleExtensions::SetVehicleWheelSize(Vehicle handle, float value) {
    auto address = GetAddress(handle);

    auto drawHandler = *reinterpret_cast<uint64_t*>((uint64_t)address + DrawHandlerPtrOffset);
    auto streamRenderGfx = *reinterpret_cast<uint64_t*>(drawHandler + StreamRenderGfxPtrOffset);

    if (streamRenderGfx != 0) {
        *(float*)(streamRenderGfx + StreamRenderWheelSizeOffset) = value;
    }
}

// These apply to b1103
// 0x7f8 to 0x814 are gear ratios!
// 0x7f8 - Reverse
// 0x7fc - 1
// 0x800 - 2
// 0x804 - 3
// 0x808 - 4
// 0x80c - 5
// 0x810 - 6
// 0x814 - 7

// 0x818: fDriveForce
// Affected by engine upgrade
// 

// 0x81C: fInitialDriveMaxFlatVel (m/s)
// Affected by no tuning options
// Doesn't influence anything?

// 0x820: final drive speed
// divide this by gear ratio and there's the top speed for the gear

// b1180
// 0x838 - power? Drive Force?
// 0x9E4 - has spoiler? (2nd bit)
// Couldn't find anything torque-related

// wheel+0x1ec - power/brake flags? abs turned off for e-brake with [wheel+0x1ec] & 0xFFFF3FFF

// b2245
// Wheel
// 0x198 - TYRE_GRIP
// 0x19C - 1.0 - (WET_GRIP * Wetness)
// 0x1A0 - TYRE_DRAG
// 0x1A4 - TOP_SPEED_MULT
// 0x208 - Entity type? Bone?
// 0x20A - Material type (uint16_t)
