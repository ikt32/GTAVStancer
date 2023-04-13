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
#include "Util/Paths.hpp"
#include "Util/String.hpp"

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
}

namespace VStancer {
    void scriptInit();
    void scriptTick();

    std::shared_ptr<CStanceScript> updateScripts();

    void updateActiveConfigs();
    bool isSupportedModel(Vehicle vehicle);
}

void VStancer::ScriptMain() {
    if (!initialized) {
        LOG(INFO, "Script started");
        scriptInit();
        initialized = true;
        VStancer::PatchHeightReset();
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
    while (true) {
        std::shared_ptr<CStanceScript> playerScriptInst = updateScripts();
        scriptMenu->Tick(playerScriptInst);
        WAIT(0);
    }
}

// Returns player script, if player was in any vehicle.
std::shared_ptr<CStanceScript> VStancer::updateScripts() {
    std::shared_ptr<CStanceScript> playerScript = nullptr;
    std::vector<std::shared_ptr<CStanceScript>> instsToDelete;

    std::vector<Vehicle> allVehicles(1024);
    int actualSize = worldGetAllVehicles(allVehicles.data(), 1024);
    allVehicles.resize(actualSize);

    for (const auto& vehicle : allVehicles) {
        if (!isSupportedModel(vehicle))
            continue;

        auto it = std::find_if(vehicleScripts.begin(), vehicleScripts.end(), [vehicle](const auto& inst) {
            return inst->GetVehicle() == vehicle;
            });

        if (it == vehicleScripts.end()) {
            vehicleScripts.push_back(std::make_shared<CStanceScript>(vehicle, configs));
            vehicleScripts.back()->UpdateActiveConfig();
        }
    }

    for (const auto& inst : vehicleScripts) {
        if (!playerScript &&
            Util::VehicleAvailable(inst->GetVehicle(), PLAYER::PLAYER_PED_ID(), false)) {
            playerScript = inst;
        }

        if (!ENTITY::DOES_ENTITY_EXIST(inst->GetVehicle())) {
            instsToDelete.push_back(inst);
        }
        else {
            if (settings->Main.EnableNPC ||
                inst == playerScript) {
                inst->Tick();
            }
        }
    }

    for (const auto& inst : instsToDelete) {
        vehicleScripts.erase(std::remove(vehicleScripts.begin(), vehicleScripts.end(), inst), vehicleScripts.end());
    }

    if (instsToDelete.size() > 0) {
        // TODO: Cleanup
    }

    return playerScript;
}

void VStancer::updateActiveConfigs() {
    for (const auto& inst : vehicleScripts) {
        inst->UpdateActiveConfig();
    }
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
            auto saveType = config.Plate.empty() ?
                CConfig::ESaveType::GenericModel :
                CConfig::ESaveType::Specific;
            config.Write(saveType);
        }
    }
}

bool VStancer::isSupportedModel(Vehicle vehicle) {
    bool hydraulics = false;
    auto flags = VExt::GetVehicleFlags(vehicle);
    if (flags[3] & eVehicleFlag4::FLAG_HAS_LOWRIDER_HYDRAULICS ||
        flags[3] & eVehicleFlag4::FLAG_HAS_LOWRIDER_DONK_HYDRAULICS) {
        hydraulics = true;
    }
    

    auto model = ENTITY::GET_ENTITY_MODEL(vehicle);
    bool isSupportedClass =
        VEHICLE::IS_THIS_MODEL_A_CAR(model) ||
        VEHICLE::IS_THIS_MODEL_A_QUADBIKE(model);
    
    return isSupportedClass && !hydraulics;
}
