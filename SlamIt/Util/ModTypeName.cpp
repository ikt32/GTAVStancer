#include "ModTypeName.hpp"

#include <inc/natives.h>
#include <format>
#include <chrono>

namespace {
    bool LoadLabels() {
        auto start = std::chrono::steady_clock::now();
        HUD::REQUEST_ADDITIONAL_TEXT("mod_mnu", 19);
        while (!HUD::HAS_ADDITIONAL_TEXT_LOADED(19)) {
            WAIT(0);

            if (std::chrono::steady_clock::now() > start + std::chrono::seconds(10))
                return false;
        }
        return true;
    }

    std::string GetModTypeNameAlt(Vehicle vehicle, eVehicleMod modType) {
        if (!LoadLabels()) {
            return {};
        }

        switch (modType) {
            case eVehicleMod::VehicleModEngine: return HUD::GET_FILENAME_FOR_AUDIO_CONVERSATION("CMOD_MOD_ENG");
            case eVehicleMod::VehicleModBrakes: return HUD::GET_FILENAME_FOR_AUDIO_CONVERSATION("CMOD_MOD_BRA");
            case eVehicleMod::VehicleModTransmission: return HUD::GET_FILENAME_FOR_AUDIO_CONVERSATION("CMOD_MOD_TRN");
            case eVehicleMod::VehicleModSuspension: return HUD::GET_FILENAME_FOR_AUDIO_CONVERSATION("CMOD_MOD_SUS");
            case eVehicleMod::VehicleModArmor: return HUD::GET_FILENAME_FOR_AUDIO_CONVERSATION("CMOD_MOD_ARM");
            case eVehicleMod::VehicleModTurbo: return HUD::GET_FILENAME_FOR_AUDIO_CONVERSATION("CMOD_MOD_TUR");
            case eVehicleMod::VehicleModWindows: return HUD::GET_FILENAME_FOR_AUDIO_CONVERSATION("MOD_WINDOWS");
            case eVehicleMod::VehicleModLivery: return HUD::GET_FILENAME_FOR_AUDIO_CONVERSATION("CMM_MOD_S23");
            case eVehicleMod::VehicleModHorns: return HUD::GET_FILENAME_FOR_AUDIO_CONVERSATION("CMOD_MOD_HRN");
            case eVehicleMod::VehicleModXenonLights: return HUD::GET_FILENAME_FOR_AUDIO_CONVERSATION("CMOD_MOD_LGT");
            case eVehicleMod::VehicleModFrontWheels: return HUD::GET_FILENAME_FOR_AUDIO_CONVERSATION("CMOD_MOD_WHEM");
            default:
            {
                auto sMod = static_cast<int32_t>(modType) - static_cast<int32_t>(eVehicleMod::VehicleModPlateholder);

                if (sMod >= 0 && sMod <= 20) {
                    return HUD::GET_FILENAME_FOR_AUDIO_CONVERSATION(std::format("CMM_MOD_S{}", sMod).c_str());
                }
                else {
                    auto label = VEHICLE::GET_MOD_SLOT_NAME(vehicle, static_cast<int>(modType));

                    if (label) {
                        if (auto text = HUD::GET_FILENAME_FOR_AUDIO_CONVERSATION(label);
                            text != "NULL") {
                            return text;
                        }
                        else {
                            return label;
                        }
                    }

                    return {};
                }
            }
            break;
        }
    }
}

std::string VStancer::GetModTypeName(Vehicle vehicle, eVehicleMod modType) {
    if (auto name = GetModTypeNameAlt(vehicle, modType);
        !name.empty()) {
        return name;
    }
    switch (modType) {
        case VehicleModSpoilers: return "Spoiler";
        case VehicleModFrontBumper: return "Front bumper";
        case VehicleModRearBumper: return "Rear bumper";
        case VehicleModSideSkirt: return "Side skirt";
        case VehicleModExhaust: return "Exhaust";
        case VehicleModFrame: return "Frame";
        case VehicleModGrille: return "Grille";
        case VehicleModHood: return "Bonnet";
        case VehicleModFender: return "Left fender";
        case VehicleModRightFender: return "Right fender";
        case VehicleModRoof: return "Roof";
        default: return std::format("UNK{}", static_cast<int>(modType));
    }
}
