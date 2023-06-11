#include "ScriptUtils.hpp"

#include "../Memory/VehicleExtensions.hpp"
#include "../Memory/VehicleFlags.hpp"

#include <inc/natives.h>

using VExt = VehicleExtensions;

enum eHandlingType {
    HANDLING_TYPE_BIKE,
    HANDLING_TYPE_FLYING,
    HANDLING_TYPE_VERTICAL_FLYING,
    HANDLING_TYPE_BOAT,
    HANDLING_TYPE_SEAPLANE,
    HANDLING_TYPE_SUBMARINE,
    HANDLING_TYPE_TRAIN,
    HANDLING_TYPE_TRAILER,
    HANDLING_TYPE_CAR,
    HANDLING_TYPE_WEAPON,
    HANDLING_TYPE_SPECIALFLIGHT,
    HANDLING_TYPE_MAX_TYPES
};

class CHandlingObject {
public:
    virtual ~CHandlingObject() = default;
    virtual void* parser_GetStructure() = 0; //ret rage::parStructure
};

class CBaseSubHandlingData : public CHandlingObject {
public:
    virtual eHandlingType GetHandlingType() = 0;
    virtual void OnPostLoad() = 0;
};

// Very, very lazy CSpecialFlightHandlingData strFlags & _SF_PERMANENTLY_HOVERING check
bool specialFlightCheck(Vehicle vehicle) {
    auto pHandling = reinterpret_cast<uint8_t*>(VExt::GetHandlingPtr(vehicle));
    if (pHandling) {
        auto shdArrayAddress = pHandling + 0x158;
        for (uint16_t idx = 0; idx < *reinterpret_cast<uint16_t*>(shdArrayAddress + 0x8); ++idx) {
            CBaseSubHandlingData** baseShds = *reinterpret_cast<CBaseSubHandlingData***>(shdArrayAddress);
            CBaseSubHandlingData* baseShd = baseShds[idx];

            if (!baseShd)
                continue;

            auto type = baseShd->GetHandlingType();
            if (baseShd->GetHandlingType() == HANDLING_TYPE_SPECIALFLIGHT) {
                uint32_t specialFlightFlags = *reinterpret_cast<uint32_t*>(baseShd + 0xB8);
                if ((specialFlightFlags & 0x80) != 0) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool VStancer::IsSupportedClass(Vehicle vehicle) {
    auto model = ENTITY::GET_ENTITY_MODEL(vehicle);
    bool isSupportedClass =
        VEHICLE::IS_THIS_MODEL_A_CAR(model) ||
        VEHICLE::IS_THIS_MODEL_A_QUADBIKE(model);

    return isSupportedClass;
}

bool VStancer::IsIncompatible(Vehicle vehicle) {
    if (!IsSupportedClass(vehicle))
        return true;

    // Flight mode, Deluxo
    if (VExt::GetHoverTransformRatio(vehicle) > 0.0f)
        return true;

    // Hydraulics
    auto flags = VExt::GetVehicleFlags(vehicle);
    if (flags[3] & eVehicleFlag4::FLAG_HAS_LOWRIDER_HYDRAULICS ||
        flags[3] & eVehicleFlag4::FLAG_HAS_LOWRIDER_DONK_HYDRAULICS) {
        return true;
    }

    if (specialFlightCheck(vehicle))
        return true;

    return false;
}
