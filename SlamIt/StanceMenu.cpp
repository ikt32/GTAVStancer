#include "ScriptMenu.hpp"
#include "Script.hpp"
#include "Constants.hpp"

#include "Util/ModTypeName.hpp"
#include "Util/UI.hpp"
#include "Memory/SuspensionOffsets.hpp"
#include "Memory/VehicleExtensions.hpp"
#include "StanceMenuUtils.hpp"

#include <GTAVMenuBase/menu.h>

#include <format>

using VExt = VehicleExtensions;

namespace VStancer {
    std::vector<std::string> FormatConfig(CStanceScript& context, const CConfig& config);
    std::vector<std::string> FormatModAdjust(CStanceScript& context, const CConfig::SModAdjustment& adjust);
    bool PromptSave(CStanceScript& context, Hash model, std::string plate);

    std::string FormatTyreDims(const CConfig::SWheelColliderParams& wheelCol) {
    // {width_mm}/{aspect_ratio}R{rim_diameter_inch}
        return std::format("{:.0f}/{:.0f}R{:.0f}", wheelCol.TyreWidth * 1000.0f,
            100.0f * ((wheelCol.TyreRadius - wheelCol.RimRadius) / wheelCol.TyreWidth),
            2.0f * wheelCol.RimRadius * 39.3701f);
    };
}

std::vector<CScriptMenu<CStanceScript>::CSubmenu> VStancer::BuildMenu() {
    std::vector<CScriptMenu<CStanceScript>::CSubmenu> submenus;
    
    /* mainmenu */
    submenus.emplace_back("mainmenu",
        [](NativeMenu::Menu& mbCtx, std::shared_ptr<CStanceScript> context) {
            mbCtx.Title(Constants::ScriptName);
            mbCtx.Subtitle(std::string("~b~") + Constants::DisplayVersion);

            if (mbCtx.MenuOption("Manage configurations", "configsmenu",
                { "Create a new configuration, or delete one." })) {
                VStancer::LoadConfigs();
            }

            Ped playerPed = PLAYER::PLAYER_PED_ID();
            Vehicle playerVehicle = PED::GET_VEHICLE_PED_IS_IN(playerPed, false);

            if (!playerVehicle || !ENTITY::DOES_ENTITY_EXIST(playerVehicle)) {
                mbCtx.Option("No active configuration",
                    { "Get in a vehicle to edit its configuration." });
            }
            else {
                mbCtx.MenuOption("Suspension", "suspensionmenu",
                    { "Change camber, height, track width and visual ride height." });

                mbCtx.MenuOption("Wheels", "wheelmenu",
                    { "Change wheel and tire sizes." });

                mbCtx.MenuOption("Misc", "miscmenu",
                    { "Other options." });

                mbCtx.MenuOption("Modifications", "modsmenu",
                    { "Display how modifications change suspension geometry." });
            }

            mbCtx.MenuOption("NPC options", "npcmenu");

            mbCtx.MenuOption("Developer options", "developermenu");
        });

    /* mainmenu -> configsmenu */
    submenus.emplace_back("configsmenu",
        [](NativeMenu::Menu& mbCtx, std::shared_ptr<CStanceScript> context) {
            mbCtx.Title("Manage configurations");

            CConfig* config = context ? context->ActiveConfig() : nullptr;
            mbCtx.Subtitle(std::format("Current: {}", config ? config->Name : "None"));

            if (!context) {
                mbCtx.Option("No script instance");
                return;
            }

            if (mbCtx.Option("Create configuration",
                { "Create a new configuration file from the current settings.",
                  "Changes made within a configuration are saved to that configuration only.",
                  "The submenu subtitles indicate which configuration is being edited." })) {
                VStancer::PromptSave(*context,
                    ENTITY::GET_ENTITY_MODEL(context->GetVehicle()),
                    VEHICLE::GET_VEHICLE_NUMBER_PLATE_TEXT(context->GetVehicle()));
            }

            if (VStancer::GetConfigs().empty()) {
                mbCtx.Option("No saved configs");
            }

            for (const auto& config : VStancer::GetConfigs()) {
                bool selected;
                bool triggered = mbCtx.OptionPlus(config.Name, {}, &selected);

                if (selected) {
                    mbCtx.OptionPlusPlus(FormatConfig(*context, config), config.Name);
                }

                if (triggered) {
                    context->ApplyConfig(config, true, true, true);
                    UI::Notify(std::format("Applied config {}.", config.Name), true);
                }
            }
        });

    /* mainmenu -> suspensionmenu */
    submenus.emplace_back("suspensionmenu",
        [](NativeMenu::Menu& mbCtx, std::shared_ptr<CStanceScript> context) {
            mbCtx.Title("Suspension settings");

            CConfig* config = context ? context->ActiveConfig() : nullptr;
            Vehicle vehicle = context ? context->GetVehicle() : 0;

            mbCtx.Subtitle(std::format("Current: {}", config ? config->Name : "None"));

            if (config == nullptr ||
                vehicle == 0) {
                mbCtx.Option("No active vehicle/configuration");
                return;
            }

            bool triggered = false;

            triggered |= mbCtx.FloatOptionCb("Front camber", config->Suspension.Front.Camber, -2.0f, 2.0f, 0.01f, GetKbEntryFloat);
            triggered |= mbCtx.FloatOptionCb("Front track width", config->Suspension.Front.TrackWidth, -2.0f, 2.0f, 0.01f, GetKbEntryFloat);
            triggered |= mbCtx.FloatOptionCb("Front height", config->Suspension.Front.Height, -2.0f, 2.0f, 0.01f, GetKbEntryFloat);

            triggered |= mbCtx.FloatOptionCb("Rear camber", config->Suspension.Rear.Camber, -2.0f, 2.0f, 0.01f, GetKbEntryFloat);
            triggered |= mbCtx.FloatOptionCb("Rear track width", config->Suspension.Rear.TrackWidth, -2.0f, 2.0f, 0.01f, GetKbEntryFloat);
            triggered |= mbCtx.FloatOptionCb("Rear height", config->Suspension.Rear.Height, -2.0f, 2.0f, 0.01f, GetKbEntryFloat);

            triggered |= mbCtx.FloatOptionCb("Visual height adjustment", config->Suspension.VisualHeight, -0.5f, 0.5f, 0.01f, GetKbEntryFloat);

            if (triggered) {
                context->ApplyConfig(*config, true, true, false);
            }
        });

    /* mainmenu -> wheelmenu */
    submenus.emplace_back("wheelmenu",
        [](NativeMenu::Menu& mbCtx, std::shared_ptr<CStanceScript> context) {
            mbCtx.Title("Wheel settings");

            CConfig* config = context ? context->ActiveConfig() : nullptr;
            Vehicle vehicle = context ? context->GetVehicle() : 0;
            mbCtx.Subtitle(std::format("Current: {}", config ? config->Name : "None"));

            if (config == nullptr ||
                vehicle == 0) {
                mbCtx.Option("No active vehicle/configuration");
                return;
            }

            mbCtx.MenuOption("Physical sizes", "wheelsizemenu",
                { "Change the physical dimensions of your wheels.",
                  "These aren't connected to the visual sizes." });

            auto wheelSize = VExt::GetVehicleWheelSize(vehicle);
            auto wheelWidth = VExt::GetVehicleWheelWidth(vehicle);

            if (wheelSize == 0.0f || wheelWidth == 0.0f) {
                mbCtx.Option("Unavailable", { "Changing visual wheel size/width is unavailable on stock wheels." });
                return;
            }

            auto wheelType = VEHICLE::GET_VEHICLE_WHEEL_TYPE(vehicle);
            auto wheelMod = VEHICLE::GET_VEHICLE_MOD(vehicle, eVehicleMod::VehicleModFrontWheels);

            std::vector<std::string> wheelInfo = {
                "Visual size and width are tied to the installed wheels.",
                std::format("Wheel type: {}", wheelType),
                std::format("Wheel mod: {}", wheelMod),
            };

            mbCtx.OptionPlus("Explanation", wheelInfo);

            config->Wheels.Visual = context->GetWheelRenderParams();

            bool triggered = false;
            triggered |= mbCtx.FloatOptionCb("Visual size", config->Wheels.Visual.WheelSize, 0.0f, 5.0f, 0.01f, GetKbEntryFloat);
            triggered |= mbCtx.FloatOptionCb("Visual width", config->Wheels.Visual.WheelWidth, 0.0f, 5.0f, 0.01f, GetKbEntryFloat);

            if (triggered) {
                context->ApplyConfig(*config, true, true, false);
            }
        });

    /* mainmenu -> wheelmenu -> wheelsizemenu */
    submenus.emplace_back("wheelsizemenu",
        [](NativeMenu::Menu& mbCtx, std::shared_ptr<CStanceScript> context) {
            mbCtx.Title("Wheel sizes");

            CConfig* config = context ? context->ActiveConfig() : nullptr;
            mbCtx.Subtitle(std::format("Current: {}", config ? config->Name : "None"));

            if (config == nullptr) {
                mbCtx.Option("No active vehicle/configuration");
                return;
            }

            std::vector<std::string> dimsFmt = {
                "Resulting tyre dimensions:",
                std::format("Front: {}", FormatTyreDims(config->Wheels.Front)),
                std::format("Rear: {}", FormatTyreDims(config->Wheels.Rear))
            };

            bool triggered = false;

            triggered |= mbCtx.FloatOptionCb(std::format("Front tyre radius"), config->Wheels.Front.TyreRadius, 0.0f, 10.0f, 0.01f, GetKbEntryFloat, { dimsFmt });
            triggered |= mbCtx.FloatOptionCb(std::format("Front tyre width"), config->Wheels.Front.TyreWidth, 0.0f, 10.0f, 0.01f, GetKbEntryFloat, { dimsFmt });
            triggered |= mbCtx.FloatOptionCb(std::format("Front rim radius"), config->Wheels.Front.RimRadius, 0.0f, 10.0f, 0.01f, GetKbEntryFloat, { dimsFmt });

            triggered |= mbCtx.FloatOptionCb(std::format("Rear tyre radius"), config->Wheels.Rear.TyreRadius, 0.0f, 10.0f, 0.01f, GetKbEntryFloat, { dimsFmt });
            triggered |= mbCtx.FloatOptionCb(std::format("Rear tyre width"), config->Wheels.Rear.TyreWidth, 0.0f, 10.0f, 0.01f, GetKbEntryFloat, { dimsFmt });
            triggered |= mbCtx.FloatOptionCb(std::format("Rear rim radius"), config->Wheels.Rear.RimRadius, 0.0f, 10.0f, 0.01f, GetKbEntryFloat, { dimsFmt });

            if (triggered) {
                context->ApplyConfig(*config, true, true, false);
            }
        });

    /* mainmenu -> miscmenu */
    submenus.emplace_back("miscmenu",
        [](NativeMenu::Menu& mbCtx, std::shared_ptr<CStanceScript> context) {
            mbCtx.Title("Misc settings");

            CConfig* config = context ? context->ActiveConfig() : nullptr;
            mbCtx.Subtitle(std::format("Current: {}", config ? config->Name : "None"));

            if (config == nullptr) {
                mbCtx.Option("No active vehicle/configuration");
                return;
            }

            if (mbCtx.IntOption("Damage", config->Misc.DamageLoweringLevel, -1, 2, 1,
                { "Damages the suspension, causing the vehicle to drop.",
                  "Effect is automatically applied if set to 0, 1 or 2.",
                  "-1 disables it." })) {
                context->ApplyConfig(*config, true, true, false);
                PAD::SET_CONTROL_VALUE_NEXT_FRAME(0, ControlVehicleAccelerate, 0.3f);
            }
        });

    /* mainmenu -> modsmenu */
    submenus.emplace_back("modsmenu",
        [](NativeMenu::Menu& mbCtx, std::shared_ptr<CStanceScript> context) {
            mbCtx.Title("Modifications");

            CConfig* config = context ? context->ActiveConfig() : nullptr;
            mbCtx.Subtitle(std::format("Current: {}", config ? config->Name : "None"));

            if (config == nullptr) {
                mbCtx.Option("No active vehicle/configuration");
                return;
            }

            // TODO: Create a config. Probably not possible with the current tools.
            // Authors will have to do this themselves in the .ini

            if (config->ModAdjustments.empty()) {
                mbCtx.Option("No modification adjustments", { "Create them manually in the config .ini." });
            }

            for (const auto& adjustment : config->ModAdjustments) {
                std::string modSlotName = GetModTypeName(context->GetVehicle(), (eVehicleMod)adjustment.ModSlot);

                const char* modIndexNameRaw = VEHICLE::GET_MOD_TEXT_LABEL(context->GetVehicle(), adjustment.ModSlot, adjustment.ModIndex);
                std::string modIndexName = HUD::GET_FILENAME_FOR_AUDIO_CONVERSATION(modIndexNameRaw);
                modIndexName = modIndexName != "NULL" ? modIndexName : modIndexNameRaw;

                std::string modName = std::format("{}_{}: {} - {}", adjustment.ModSlot, adjustment.ModIndex, modSlotName, modIndexName);

                bool selected;
                mbCtx.OptionPlus(modName, {}, &selected);

                if (selected) {
                    mbCtx.OptionPlusPlus(FormatModAdjust(*context, adjustment), modName);
                }
            }
        });

    /* mainmenu -> npcmenu */
    submenus.emplace_back("npcmenu",
        [](NativeMenu::Menu& mbCtx, std::shared_ptr<CStanceScript> context) {
            mbCtx.Title("NPC settings");

            CConfig* config = context ? context->ActiveConfig() : nullptr;
            mbCtx.Subtitle(std::format("Current: {}", config ? config->Name : "None"));

            if (config == nullptr) {
                mbCtx.Option("No active vehicle/configuration");
                return;
            }

            mbCtx.Option("Placeholder");

        });

    /* mainmenu -> developermenu */
    submenus.emplace_back("developermenu",
        [](NativeMenu::Menu& mbCtx, std::shared_ptr<CStanceScript> context) {
            mbCtx.Title("Developer settings");

            CConfig* config = context ? context->ActiveConfig() : nullptr;
            mbCtx.Subtitle(std::format("Current: {}", config ? config->Name : "None"));

            if (config == nullptr) {
                mbCtx.Option("No active vehicle/configuration");
                return;
            }

            mbCtx.Option("Placeholder");

        });
    return submenus;
}

std::vector<std::string> VStancer::FormatConfig(CStanceScript& context, const CConfig& config) {
    std::vector<std::string> extras{
        std::format("Name: {}", config.Name),
        std::format("Model: {}", config.ModelName.empty() ? "None (Generic)" : config.ModelName),
        std::format("Plate: {}", config.Plate.empty() ? "None" : std::format("[{}]", config.Plate)),

        "Suspension:",
        std::format("Camber F: {:.2f}, R: {:.2f}", config.Suspension.Front.Camber, config.Suspension.Rear.Camber),
        std::format("Track width F: {:.2f}, R: {:.2f}", config.Suspension.Front.TrackWidth, config.Suspension.Rear.TrackWidth),
        std::format("Ride height F: {:.2f}, R: {:.2f}", config.Suspension.Front.Height, config.Suspension.Rear.Height),
        std::format("Visual height: {:.2f}", config.Suspension.VisualHeight),

        "Wheels (collision):",
        std::format("Front: {}", FormatTyreDims(config.Wheels.Front)),
        std::format("Tyre radius: {:.2f}", config.Wheels.Front.TyreRadius),
        std::format("Tyre width: {:.2f}", config.Wheels.Front.TyreWidth),
        std::format("Rim radius: {:.2f}", config.Wheels.Front.RimRadius),

        std::format("Rear: {}", FormatTyreDims(config.Wheels.Rear)),
        std::format("Tyre radius: {:.2f}", config.Wheels.Rear.TyreRadius),
        std::format("Tyre width: {:.2f}", config.Wheels.Rear.TyreWidth),
        std::format("Rim radius: {:.2f}", config.Wheels.Rear.RimRadius),
    };
    return extras;
}

std::vector<std::string> VStancer::FormatModAdjust(CStanceScript& context, const CConfig::SModAdjustment& adjust) {
    std::string modSlotName = GetModTypeName(context.GetVehicle(), (eVehicleMod)adjust.ModSlot);

    const char* modIndexNameRaw = VEHICLE::GET_MOD_TEXT_LABEL(context.GetVehicle(), adjust.ModSlot, adjust.ModIndex);
    std::string modIndexName = HUD::GET_FILENAME_FOR_AUDIO_CONVERSATION(modIndexNameRaw);

    if (modIndexNameRaw == nullptr || modIndexName.empty() || modIndexName == "NULL")
        modIndexName = std::format("{}", adjust.ModIndex);

    std::vector<std::string> extras{
        std::format("Slot {} : {}", adjust.ModSlot, modSlotName),
        std::format("Modification {} : {}", adjust.ModIndex, modIndexName),

        "Adjustments:",
        std::format("Camber F: {:.2f}, R: {:.2f}", adjust.FrontAdjust.Camber, adjust.RearAdjust.Camber),
        std::format("Track width F: {:.2f}, R: {:.2f}", adjust.FrontAdjust.TrackWidth, adjust.RearAdjust.TrackWidth),
        std::format("Ride height F: {:.2f}, R: {:.2f}", adjust.FrontAdjust.Height, adjust.RearAdjust.Height),
    };
    return extras;
}

bool VStancer::PromptSave(CStanceScript& context, Hash model, std::string plate) {
    UI::ShowHelpText("Enter new configuration name.");
    std::string newName = UI::GetKeyboardResult();

    if (newName.empty()) {
        UI::ShowHelpText("No configuration name entered, save cancelled.");
        return false;
    }

    UI::ShowHelpText("Enter '1' for a generic model configuration.\n"
        "Enter '2' for a model and plate matched configuration.");
    std::string configMode = UI::GetKeyboardResult();

    CConfig::ESaveType saveType;
    if (configMode == "1") {
        saveType = CConfig::ESaveType::GenericModel;
    }
    else if (configMode == "2") {
        saveType = CConfig::ESaveType::Specific;
    }
    else {
        UI::ShowHelpText("No supported configuration type entered, save cancelled.");
        return false;
    }

    if (context.ActiveConfig()->Write(newName, model, plate, saveType))
        UI::Notify("New configuration saved.", true);
    else
        UI::Notify("~r~An error occurred~s~, failed to save new configuration.\n"
            "Check the log file for further details.", true);
    VStancer::LoadConfigs();

    return true;
}
