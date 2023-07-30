#include "Script.hpp"

#include "Config.hpp"
#include "Constants.hpp"
#include "ScriptSettings.hpp"
#include "StanceScript.hpp"

#include "Memory/VehicleExtensions.hpp"
#include "Memory/VehicleFlags.hpp"
#include "Patching/SuspensionPatch.hpp"
#include "Util/Game.hpp"
#include "Util/Logger.hpp"
#include "Util/Math.hpp"
#include "Util/Paths.hpp"
#include "Util/ScriptUtils.hpp"
#include "Util/String.hpp"
#include "Util/Timer.hpp"

#include <inc/main.h>
#include <inc/natives.h>
#include <memory>
#include <filesystem>

using VExt = VehicleExtensions;

namespace {
    std::shared_ptr<CScriptSettings> settings;
    std::vector<std::shared_ptr<CStanceScript>> vehicleScripts;
    std::unique_ptr<CScriptMenu<CStanceScript>> scriptMenu;

    std::vector<CConfig> configs;

    bool initialized = false;
    CTimer collectionUpdateTimer(5000);
}

namespace VStancer {
    void scriptInit();
    void scriptTick();

    void updateScriptCollection();
    std::shared_ptr<CStanceScript> updateScripts();

    void updateActiveConfigs();
}

void VStancer::ScriptMain() {
    if (!initialized) {
        LOG(INFO, "Script started");
        scriptInit();
        initialized = true;
    }
    else {
        LOG(INFO, "Script restarted");
    }
    try {
        scriptTick();
    }
    catch (const std::exception& ex) {
        LOG(FATAL, "Script crashed: {}", ex.what());
    }
    catch (...) {
        LOG(FATAL, "Script crashed due to unknown reason");
    }
}

void VStancer::scriptInit() {
    const auto settingsGeneralPath = Paths::GetModPath() / "settings_general.ini";
    const auto settingsMenuPath = Paths::GetModPath() / "settings_menu.ini";

    settings = std::make_shared<CScriptSettings>(settingsGeneralPath.string());
    settings->Load();
    LOG(INFO, "Settings loaded");

    VStancer::LoadConfigs();

    VehicleExtensions::Init();

    scriptMenu = std::make_unique<CScriptMenu<CStanceScript>>(settingsMenuPath.string(),
        []() {
            // OnInit
            settings->Load();
            VStancer::LoadConfigs();
        },
        []() {
            // OnExit
            settings->Save();
            VStancer::SaveConfigs();
        },
        BuildMenu()
    );
}

void VStancer::scriptTick() {
    collectionUpdateTimer.Reset(settings->Main.UpdateIntervalMs);
    while (true) {
        if (collectionUpdateTimer.Expired()) {
            updateScriptCollection();
            collectionUpdateTimer.Reset();
        }

        std::shared_ptr<CStanceScript> playerScriptInst = updateScripts();
        scriptMenu->Tick(playerScriptInst);
        WAIT(0);
    }
}

// Updates the list of script-supported vehicles.
// Also manages patching/unpatching for incompatible vehicles.
void VStancer::updateScriptCollection() {
    std::vector<Vehicle> allVehicles(1024);
    int actualSize = worldGetAllVehicles(allVehicles.data(), 1024);
    allVehicles.resize(actualSize);

    bool unpatch = false;
    bool anyIncompatibleVehicleFound = false;
    Vector3 playerCoords{};
    if (settings->Patch.PatchMode == 2) {
        playerCoords = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), false);
    }

    for (const auto& vehicle : allVehicles) {
        if (settings->Patch.PatchMode == 2 && !anyIncompatibleVehicleFound) {
            float dstSq = settings->Patch.UnpatchDistance * settings->Patch.UnpatchDistance;
            Vector3 vehCoords = ENTITY::GET_ENTITY_COORDS(vehicle, false);
            if (!IsCompatibleNow(vehicle) &&
                DistanceSq(playerCoords, vehCoords) < dstSq) {
                anyIncompatibleVehicleFound = true;
            }
        }

        if (!IsSupportedClass(vehicle))
            continue;

        if (!IsCompatible(vehicle))
            continue;

        auto it = std::find_if(vehicleScripts.begin(), vehicleScripts.end(), [vehicle](const auto& inst) {
            return inst->GetVehicle() == vehicle;
            });

        if (it == vehicleScripts.end()) {
            vehicleScripts.push_back(std::make_shared<CStanceScript>(vehicle, configs));
            vehicleScripts.back()->UpdateActiveConfig();
        }
    }

    // PatchMode -1 always unpatches
    if (settings->Patch.PatchMode == -1) {
        unpatch = true;
    }
    // PatchMode 0 always patches
    else if (settings->Patch.PatchMode == 0) {
        unpatch = false;
    }
    // PatchMode 1: Only disable if player actively is in incompat veh
    else if (settings->Patch.PatchMode == 1) {
        auto playerVehicle = PED::GET_VEHICLE_PED_IS_IN(PLAYER::PLAYER_PED_ID(), true);
        if (ENTITY::DOES_ENTITY_EXIST(playerVehicle) && !IsCompatibleNow(playerVehicle))
            unpatch = true;
    }
    // PatchMode 2: Disable if vehicles in the vicinity are incompatible
    else if (settings->Patch.PatchMode == 2 && anyIncompatibleVehicleFound) {
        unpatch = true;
    }

    if (unpatch)
        VStancer::UnpatchHeightReset();
    else
        VStancer::PatchHeightReset();
}

// Returns player script, if player was in any vehicle.
// Also cleans up the stale vehicles.
std::shared_ptr<CStanceScript> VStancer::updateScripts() {
    std::shared_ptr<CStanceScript> playerScript = nullptr;
    std::vector<std::shared_ptr<CStanceScript>> instsToDelete;

    for (const auto& inst : vehicleScripts) {
        if (!ENTITY::DOES_ENTITY_EXIST(inst->GetVehicle())) {
            instsToDelete.push_back(inst);
            continue;
        }
        if (!playerScript &&
            Util::VehicleAvailable(inst->GetVehicle(), PLAYER::PLAYER_PED_ID(), false)) {
            playerScript = inst;
        }
        inst->Tick();
    }

    for (const auto& inst : instsToDelete) {
        vehicleScripts.erase(std::remove(vehicleScripts.begin(), vehicleScripts.end(), inst), vehicleScripts.end());
    }

    return playerScript;
}

void VStancer::updateActiveConfigs() {
    for (const auto& inst : vehicleScripts) {
        inst->UpdateActiveConfig();
    }
}

CScriptSettings* VStancer::GetSettings() {
    return settings == nullptr ? nullptr : settings.get();
}

const std::vector<CConfig>& VStancer::GetConfigs() {
    return configs;
}

uint32_t VStancer::LoadConfigs() {
    namespace fs = std::filesystem;

    const auto configsPath = Paths::GetModPath() / "Configs";

    LOG(DEBUG, "Clearing and reloading configs");

    configs.clear();

    if (!(fs::exists(configsPath) && fs::is_directory(configsPath))) {
        LOG(ERROR, "Directory [{}] not found!", configsPath.string());
        return 0;
    }

    for (const auto& file : fs::directory_iterator(configsPath)) {
        if (Util::to_lower(fs::path(file).extension().string()) != ".ini") {
            LOG(DEBUG, "Skipping [{}] - not .ini", file.path().stem().string());
            continue;
        }

        CConfig config = CConfig::Read(fs::path(file).string());

        configs.push_back(config);
        LOG(DEBUG, "Loaded vehicle config [{}]", config.Name);
    }

    LOG(INFO, "Configs loaded: {}", configs.size());

    VStancer::updateActiveConfigs();
    return static_cast<unsigned>(configs.size());
}

void VStancer::SaveConfigs() {
    namespace fs = std::filesystem;

    const auto configsPath = Paths::GetModPath() / "Configs";

    LOG(DEBUG, "Saving all configs");

    if (!(fs::exists(configsPath) && fs::is_directory(configsPath))) {
        LOG(ERROR, "Directory [{}] not found!", configsPath.string());
        return;
    }

    for (auto& config : configs) {
        if (!config.ModelName.empty()) {
            config.Write();
        }
    }
}
