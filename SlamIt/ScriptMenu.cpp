#include "script.h"
#include "GTAVMenuBase/menu.h"
#include "Util/Versions.h"
#include "settings.h"
#include "../../GTAVManualTransmission/Gears/Memory/VehicleExtensions.hpp"
#include "Util/Util.hpp"
#include "../../GTAVManualTransmission/Gears/Memory/NativeMemory.hpp"
#include "Offsets.h"

extern NativeMenu::Menu menu;
extern Hash model;
extern Vehicle vehicle;
extern VehicleExtensions ext;
extern int prevNotification;
extern std::vector<Preset> presets;
extern std::vector<Preset> saved;
extern Settings settings;
extern float g_frontCamber;
extern float g_frontTrackWidth;
extern float g_frontHeight;
extern float g_rearCamber;
extern float g_rearTrackWidth;
extern float g_rearHeight;
extern float g_visualHeight;
extern int slamLevel;
extern bool showOnlyCompatible;
extern uint32_t offVisualWidth;

void choosePresetMenu(std::string title, std::vector<Preset> whichPresets) {
    menu.Title(title);
    menu.Subtitle(title);

    std::string currentName = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(model);
    std::string compatibleString = "Show only " + currentName;
    menu.BoolOption(compatibleString, showOnlyCompatible);
    for (auto preset : whichPresets) {
        if (showOnlyCompatible) {
            if (preset.Name() != currentName) {
                continue;
            }
        }
        std::string label = preset.Name() + " " + preset.Plate();
        std::vector<std::string> info;
        info.push_back("Press RIGHT to delete preset");
        info.push_back("Front Camber\t\t" + std::to_string(preset.FrontSuspension.Camber));
        info.push_back("Front Track width\t" + std::to_string(preset.FrontSuspension.TrackWidth));
        info.push_back("Front Height\t\t" + std::to_string(preset.FrontSuspension.Height));
        info.push_back("Rear Camber\t\t" + std::to_string(preset.RearSuspension.Camber));
        info.push_back("Rear Track width\t" + std::to_string(preset.RearSuspension.TrackWidth));
        info.push_back("Rear Height\t\t" + std::to_string(preset.RearSuspension.Height));
        std::string heightDisplay = preset.VisualHeight == -1337.0f ? "Missing Value" : std::to_string(preset.VisualHeight);
        info.push_back("Visual height\t\t" + heightDisplay);

        if (menu.OptionPlus(label, info, nullptr, std::bind(deletePreset, preset, whichPresets), nullptr, "Preset data")) {
            applyPreset(preset);
            getStats(vehicle);
            showNotification("Applied preset!", &prevNotification);
        }
    }
}

void update_mainmenu() {
    menu.Title("VStancer");
    menu.Subtitle(DISPLAY_VERSION);

    if (getGameVersion() < G_VER_1_0_944_2_STEAM) {
        menu.Option("Unsupported game version!", NativeMenu::solidRed, { "You need v1.0.944.2 or newer." });
    }
    
    if (menu.BoolOption("Enable mod", settings.enableMod, { "Enables or disables the entire mod." })) {
        settings.SaveSettings();
        if (settings.enableMod && settings.enableHeight) {
            patchHeightReset();
        }
        else {
            unloadPatch();
        }
    }
    if (menu.BoolOption("Auto-apply", settings.autoApply, { "Automatically apply the car-specific preset if "
                            "the licence plate and car model match." })) {
        settings.SaveSettings();
    }

    menu.MenuOption("Suspension menu", "suspensionmenu", { "Change camber, height, track width and overall height." });
    menu.MenuOption("Tyres", "tyremenu", { "View and edit vehicle tyre settings." });

    menu.MenuOption("Load a preset", "presetmenu", { "Load and manage a generic preset." });
    menu.MenuOption("List car configs", "carsmenu", { "Show and manage car-specific presets." });
    if (menu.Option("Save as car", { "Save as car-specific preset. This loads the current setting when you get in "
                        "this car with this licence plate." })) {
        savePreset(false, "");
    }
    if (menu.Option("Save as preset", { "Save as generic preset." })) {
        savePreset(true, "");
    }

    menu.MenuOption("Other stuff", "othermenu", { "\"Slam It\" is here at the moment." });

}

void update_suspensionmenu() {
    menu.Title("Suspension menu");
    menu.Subtitle("");

    menu.FloatOption("Front Camber\t\t", g_frontCamber, -2.0f, 2.0f, 0.01f);
    menu.FloatOption("Front Track Width", g_frontTrackWidth, -2.0f, 2.0f, 0.01f);
    if (menu.FloatOption("Front Height\t\t", g_frontHeight, -2.0f, 2.0f, 0.01f)) {
        ENTITY::APPLY_FORCE_TO_ENTITY_CENTER_OF_MASS(vehicle, 1, 0.0f, 0.1f, 0.0f, true, true, true, true);
    }

    menu.FloatOption("Rear Camber\t\t", g_rearCamber, -2.0f, 2.0f, 0.01f);
    menu.FloatOption("Rear Track Width", g_rearTrackWidth, -2.0f, 2.0f, 0.01f);
    if (menu.FloatOption("Rear Height\t\t", g_rearHeight, -2.0f, 2.0f, 0.01f)) {
        ENTITY::APPLY_FORCE_TO_ENTITY_CENTER_OF_MASS(vehicle, 1, 0.0f, 0.1f, 0.0f, true, true, true, true);
    }

    if (menu.FloatOption("Visual Lowering", g_visualHeight, -0.5f, 0.5f, 0.01f, { "This changes the same value tuning the suspension in mod shops does." })) {
        ext.SetVisualHeight(vehicle, g_visualHeight);
    }
}

void update_othermenu() {
    menu.Title("Other options");
    menu.Subtitle("");
    if (menu.IntOption("Slam", slamLevel, 0, 2, 1, { "This damages the suspension/wheel so the car drops. Effect is removed upon vehicle repair." })) {
        oldSlam(vehicle, slamLevel);
        CONTROLS::_SET_CONTROL_NORMAL(0, ControlVehicleAccelerate, 0.3f);
    }
    if (menu.BoolOption("Enable Height tuning", settings.enableHeight, { "Enabling the car height setting is not compatible with lowriders "
    "and cars that have moving/transforming wheels."})) {
        settings.SaveSettings();
        if (settings.enableMod && settings.enableHeight) {
            patchHeightReset();
        }
        else {
            unloadPatch();
        }
    }
}

void update_tyremenu() {
    menu.Title("Tyres/Wheels");
    menu.Subtitle("");

    std::string physDetails = "Change the physical dimensions of your wheels";
    menu.MenuOption("Front", "wheelsizefrontmenu", { physDetails });
    menu.MenuOption("Rear", "wheelsizerearmenu", { physDetails });

    auto CVeh_0x48 = *(uint64_t *)(ext.GetAddress(vehicle) + 0x48);
    auto CVeh_0x48_0x370 = *(uint64_t *)(CVeh_0x48 + 0x370);
    if (CVeh_0x48_0x370 == 0) {
        menu.Option("Unavailable", { "Changing visual wheel size/width is unavailable on stock wheels." });
        return;
    }
    std::string extraInfo = "Visual property. Applies to all wheels. Doesn't work with stock wheels.";
    menu.FloatOption("Wheel size", *(float *)(CVeh_0x48_0x370 + 0x8), 0.0f, 5.0f, 0.01f, { extraInfo });
    menu.FloatOption("Wheel width", *(float *)(CVeh_0x48_0x370 + offVisualWidth), 0.0f, 5.0f, 0.01f, { extraInfo });
}

void update_wheelsizefrontmenu() {
    menu.Title("Front");
    menu.Subtitle("");

    int numWheels = ext.GetNumWheels(vehicle);
    if (numWheels < 4) {
        menu.Option("Vehicle has < 4 wheels", { "That's " + std::to_string(numWheels) + " wheels." });
        return;
    }

    auto wheels = ext.GetWheelPtrs(vehicle);

    float oldTyreRadius = *reinterpret_cast<float *>(wheels[0] + offTyreRadius);
    float oldRimRadius = *reinterpret_cast<float *>(wheels[0] + offRimRadius);
    float oldTyreWidth = *reinterpret_cast<float *>(wheels[0] + offTyreWidth);

    if (menu.FloatOption("Tyre radius", oldTyreRadius, 0.0f, 10.0f, 0.01f)) {
        *reinterpret_cast<float *>(wheels[0] + offTyreRadius) = oldTyreRadius;
        *reinterpret_cast<float *>(wheels[1] + offTyreRadius) = oldTyreRadius;
    }

    if (menu.FloatOption("Rim radius", oldRimRadius, 0.0f, 10.0f, 0.01f)) {

        *reinterpret_cast<float *>(wheels[0] + offRimRadius) = oldRimRadius;
        *reinterpret_cast<float *>(wheels[1] + offRimRadius) = oldRimRadius;
    }
        
    if (menu.FloatOption("Tyre width", oldTyreWidth, 0.0f, 10.0f, 0.01f)) {

        *reinterpret_cast<float *>(wheels[0] + offTyreWidth) = oldTyreWidth;
        *reinterpret_cast<float *>(wheels[1] + offTyreWidth) = oldTyreWidth;
    }
}

void update_wheelsizerearmenu() {
    menu.Title("Rear");
    menu.Subtitle("");

    int numWheels = ext.GetNumWheels(vehicle);
    if (numWheels < 4) {
        menu.Option("Vehicle has < 4 wheels", { "That's " + std::to_string(numWheels) + " wheels." });
        return;
    }

    auto wheels = ext.GetWheelPtrs(vehicle);

    float oldTyreRadius = *reinterpret_cast<float *>(wheels[2] + offTyreRadius);
    float oldRimRadius = *reinterpret_cast<float *>(wheels[2] + offRimRadius);
    float oldTyreWidth = *reinterpret_cast<float *>(wheels[2] + offTyreWidth);

    if (menu.FloatOption("Tyre radius", oldTyreRadius, 0.0f, 10.0f, 0.01f)) {
        for (int i = 2; i < numWheels; i++) {
            *reinterpret_cast<float *>(wheels[i] + offTyreRadius) = oldTyreRadius;
        }
    }

    if (menu.FloatOption("Rim radius", oldRimRadius, 0.0f, 10.0f, 0.01f)) {
        for (int i = 2; i < numWheels; i++) {
            *reinterpret_cast<float *>(wheels[i] + offRimRadius) = oldRimRadius;
        }
    }

    if (menu.FloatOption("Tyre width", oldTyreWidth, 0.0f, 10.0f, 0.01f)) {
        for (int i = 2; i < numWheels; i++) {
            *reinterpret_cast<float *>(wheels[i] + offTyreWidth) = oldTyreWidth;
        }
    }
}

void update_menu() {
    menu.CheckKeys();

    if (menu.CurrentMenu("mainmenu")) {
        update_mainmenu();
    }

    if (menu.CurrentMenu("suspensionmenu")) {
        update_suspensionmenu();
    }

    if (menu.CurrentMenu("presetmenu")) {
        choosePresetMenu("Load preset", presets);
    }

    if (menu.CurrentMenu("carsmenu")) {
        choosePresetMenu("Car overview", saved);
    }

    if (menu.CurrentMenu("othermenu")) {
        update_othermenu();
    }

    if (menu.CurrentMenu("tyremenu")) {
        update_tyremenu();
    }

    if (menu.CurrentMenu("wheelsizefrontmenu")) {
        update_wheelsizefrontmenu();
    }

    if (menu.CurrentMenu("wheelsizerearmenu")) {
        update_wheelsizerearmenu();
    }


    menu.EndMenu();
}
