#include "Config.hpp"
#include "SettingsCommon.hpp"

#include "Constants.hpp"
#include "Util/AddonSpawnerCache.hpp"
#include "Util/Paths.hpp"
#include "Util/Logger.hpp"
#include "Util/String.hpp"

#include <simpleini/SimpleIni.h>
#include <filesystem>
#include <format>
#include <sstream>

#define CHECK_LOG_SI_ERROR(result, operation, file) \
    if ((result) < 0) { \
        LOG(ERROR, "[Config] {} Failed to {}, SI_Error [{}]", \
        file, operation, result); \
    }

#define SAVE_VAL(section, key, option) \
    { \
        SetValue(ini, section, key, option); \
    }

#define LOAD_VAL(section, key, option) \
    { \
        option = (GetValue(ini, section, key, option)); \
    }

CConfig CConfig::Read(const std::string & configFile) {
    CConfig config{};
    config.Path = configFile;

    CSimpleIniA ini;
    ini.SetUnicode();
    ini.SetMultiLine(true);
    SI_Error result = ini.LoadFile(configFile.c_str());
    CHECK_LOG_SI_ERROR(result, "load", configFile);

    config.Name = std::filesystem::path(configFile).stem().string();

    // [ID]
    std::string modelNamesAll = ini.GetValue("ID", "Models", "");

    std::string modelHashStr = ini.GetValue("ID", "ModelHash", "");
    std::string modelName = ini.GetValue("ID", "ModelName", "");


    if (modelHashStr.empty() && modelName.empty()) {
        // This is a no-vehicle config. Nothing to be done.
    }
    else if (modelHashStr.empty()) {
        // This config only has a model name.
        config.ModelHash = Util::joaat(modelName.c_str());
        config.ModelName = modelName;
    }
    else {
        // This config only has a hash.
        Hash modelHash = 0;
        int found = sscanf_s(modelHashStr.c_str(), "%X", &modelHash);

        if (found == 1) {
            config.ModelHash = modelHash;

            auto& asCache = ASCache::Get();
            auto it = asCache.find(modelHash);
            std::string modelName = it == asCache.end() ? std::string() : it->second;
            config.ModelName = modelName;
        }
    }

    std::string plate = ini.GetValue("ID", "Plate", "");
    const std::string plateFormatError = "Incorrect plate format, expect 8 chars in brackets: '[........]'";
    if (plate.size() == 10) {
        if (plate[0] == '[' &&
            plate[9] == ']') {
            config.Plate = plate.substr(1, 8);
        }
        else {
            LOG(ERROR, "[Config] {} {}, got '{}'", configFile, plateFormatError, plate);
        }
    }
    else if (!plate.empty()) {
        LOG(ERROR, "[Config] {} {}, got '{}'", configFile, plateFormatError, plate);
    }
    // Else: Empty plate is fine

    // [Suspension]
    LOAD_VAL("Suspension", "FrontCamber", config.Suspension.Front.Camber);
    LOAD_VAL("Suspension", "FrontHeight", config.Suspension.Front.Height);
    LOAD_VAL("Suspension", "FrontTrackWidth", config.Suspension.Front.TrackWidth);

    LOAD_VAL("Suspension", "RearCamber", config.Suspension.Rear.Camber);
    LOAD_VAL("Suspension", "RearHeight", config.Suspension.Rear.Height);
    LOAD_VAL("Suspension", "RearTrackWidth", config.Suspension.Rear.TrackWidth);

    LOAD_VAL("Suspension", "VisualHeight", config.Suspension.VisualHeight);

    // [Wheels]
    LOAD_VAL("Wheels", "FrontTyreRadius", config.Wheels.Front.TyreRadius);
    LOAD_VAL("Wheels", "FrontTyreWidth", config.Wheels.Front.TyreWidth);
    LOAD_VAL("Wheels", "FrontRimRadius", config.Wheels.Front.RimRadius);

    LOAD_VAL("Wheels", "RearTyreRadius", config.Wheels.Rear.TyreRadius);
    LOAD_VAL("Wheels", "RearTyreWidth", config.Wheels.Rear.TyreWidth);
    LOAD_VAL("Wheels", "RearRimRadius", config.Wheels.Rear.RimRadius);

    LOAD_VAL("Wheels", "VisualWheelSize", config.Wheels.Visual.WheelSize);
    LOAD_VAL("Wheels", "VisualWheelWidth", config.Wheels.Visual.WheelWidth);
    LOAD_VAL("Wheels", "VisualWheelType", config.Wheels.Visual.WheelType);
    LOAD_VAL("Wheels", "VisualModIndex", config.Wheels.Visual.ModIndex);

    // [Misc]
    LOAD_VAL("Misc", "DamageLoweringLevel", config.Misc.DamageLoweringLevel);

    // [Modifications]
    // e.g. "1_2,1_3,2_1"
    const char* modIdentifiersRaw = ini.GetValue("Modifications", "Identifiers");
    if (modIdentifiersRaw) {
        auto modIdentifiers = Util::split(modIdentifiersRaw, ',');

        for (const auto& modIdentifier : modIdentifiers) {
            const std::string frontCamber = std::format("{}_FrontCamber", modIdentifier);
            const std::string frontTrackWidth = std::format("{}_FrontTrackWidth", modIdentifier);
            const std::string frontHeight = std::format("{}_FrontHeight", modIdentifier);

            const std::string rearCamber = std::format("{}_RearCamber", modIdentifier);
            const std::string rearTrackWidth = std::format("{}_RearTrackWidth", modIdentifier);
            const std::string rearHeight = std::format("{}_RearHeight", modIdentifier);

            // Error checking is overrated. Crash and burn on wrong entry <3
            int modSlot, modIndex;
            sscanf_s(modIdentifier.c_str(), "%d_%d", &modSlot, &modIndex);
            config.ModAdjustments.push_back({
                .ModSlot = modSlot,
                .ModIndex = modIndex,
                .FrontAdjust = {
                    .Camber = (float)ini.GetDoubleValue("Modifications", frontCamber.c_str(), 0.0),
                    .TrackWidth = (float)ini.GetDoubleValue("Modifications", frontTrackWidth.c_str(), 0.0),
                    .Height = (float)ini.GetDoubleValue("Modifications", frontHeight.c_str(), 0.0),
                },
                .RearAdjust = {
                    .Camber = (float)ini.GetDoubleValue("Modifications", rearCamber.c_str(), 0.0),
                    .TrackWidth = (float)ini.GetDoubleValue("Modifications", rearTrackWidth.c_str(), 0.0),
                    .Height = (float)ini.GetDoubleValue("Modifications", rearHeight.c_str(), 0.0),
                }
                });
        }
    }

    return config;
}

void CConfig::Write() {
    auto saveType = Plate.empty() ?
        CConfig::ESaveType::GenericModel :
        CConfig::ESaveType::Specific;
    Write(Name, 0, Plate, saveType, false);
}

CConfig::ESaveResult CConfig::Write(const std::string& newName, Hash model, std::string plate, ESaveType saveType, bool newFile) {
    const auto configsPath = Paths::GetModPath() / "Configs";
    const auto configFile = configsPath / std::format("{}.ini", newName);
    if (newFile && std::filesystem::exists(configFile)) {
        LOG(ERROR, "[Config] Failed to write new config '{}': already exists", configFile.string());
        return ESaveResult::FailExists;
    }

    CSimpleIniA ini;
    ini.SetUnicode();
    ini.SetMultiLine(true);

    // This here MAY fail on first save, in which case, it can be ignored.
    // _Not_ having this just nukes the entire file, including any comments.
    SI_Error result = ini.LoadFile(configFile.c_str());
    if (result < 0) {
        LOG(WARN, "[Config] {} Failed to load, SI_Error [{}]. (No problem if no file exists yet)",
            configFile.string(), (int)result);
    }

    // [ID]
    if (saveType != ESaveType::GenericNone) {
        if (model != 0) {
            ModelHash = model;
        }

        ini.SetValue("ID", "ModelHash", std::format("{:X}", ModelHash).c_str());

        auto& asCache = ASCache::Get();
        auto it = asCache.find(ModelHash);
        std::string modelName = it == asCache.end() ? std::string() : it->second;
        if (!modelName.empty()) {
            ModelName = modelName;
            ini.SetValue("ID", "ModelName", modelName.c_str());
        }

        if (saveType == ESaveType::Specific) {
            Plate = plate;
            ini.SetValue("ID", "Plate", std::format("[{}]", plate).c_str());
        }
    }

    // [Suspension]
    SAVE_VAL("Suspension", "FrontCamber", Suspension.Front.Camber);
    SAVE_VAL("Suspension", "FrontHeight", Suspension.Front.Height);
    SAVE_VAL("Suspension", "FrontTrackWidth", Suspension.Front.TrackWidth);

    SAVE_VAL("Suspension", "RearCamber", Suspension.Rear.Camber);
    SAVE_VAL("Suspension", "RearHeight", Suspension.Rear.Height);
    SAVE_VAL("Suspension", "RearTrackWidth", Suspension.Rear.TrackWidth);

    SAVE_VAL("Suspension", "VisualHeight", Suspension.VisualHeight);

    // [Wheels]
    SAVE_VAL("Wheels", "FrontTyreRadius", Wheels.Front.TyreRadius);
    SAVE_VAL("Wheels", "FrontTyreWidth", Wheels.Front.TyreWidth);
    SAVE_VAL("Wheels", "FrontRimRadius", Wheels.Front.RimRadius);

    SAVE_VAL("Wheels", "RearTyreRadius", Wheels.Rear.TyreRadius);
    SAVE_VAL("Wheels", "RearTyreWidth", Wheels.Rear.TyreWidth);
    SAVE_VAL("Wheels", "RearRimRadius", Wheels.Rear.RimRadius);

    SAVE_VAL("Wheels", "VisualWheelSize", Wheels.Visual.WheelSize);
    SAVE_VAL("Wheels", "VisualWheelWidth", Wheels.Visual.WheelWidth);
    SAVE_VAL("Wheels", "VisualWheelType", Wheels.Visual.WheelType);
    SAVE_VAL("Wheels", "VisualModIndex", Wheels.Visual.ModIndex);

    // [Misc]
    SAVE_VAL("Misc", "DamageLoweringLevel", Misc.DamageLoweringLevel);

    result = ini.SaveFile(configFile.c_str());
    CHECK_LOG_SI_ERROR(result, "save", configFile.string());
    if (result < 0)
        return ESaveResult::FailOther;
    return ESaveResult::Success;
}

CConfig::SSuspensionParams operator+(const CConfig::SSuspensionParams& a, const CConfig::SSuspensionParams& b) {
    return {
        .Camber = a.Camber + b.Camber,
        .TrackWidth = a.TrackWidth + b.TrackWidth,
        .Height = a.Height + b.Height,
    };
}
