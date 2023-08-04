#include "StanceScript.hpp"

#include "Memory/SuspensionOffsets.hpp"
#include "Memory/VehicleExtensions.hpp"

#include "Util/AddonSpawnerCache.hpp"
#include "Util/ScriptUtils.hpp"
#include "Util/String.hpp"
#include <inc/enums.h>
#include <inc/natives.h>

using VExt = VehicleExtensions;

CStanceScript::CStanceScript(Vehicle vehicle, std::vector<CConfig>& configs)
    : mConfigs(configs)
    , mVehicle(vehicle) {
    initializeDefaultConfig();
}

CStanceScript::~CStanceScript() = default;

void CStanceScript::Tick() {
    if (!ENTITY::DOES_ENTITY_EXIST(mVehicle) || !mActiveConfig) {
        mActiveConfig = nullptr;
        return;
    }

    if (!VStancer::IsCompatibleNow(mVehicle))
        return;

    update();
}

void CStanceScript::UpdateActiveConfig() {
    if (!ENTITY::DOES_ENTITY_EXIST(mVehicle)) {
        mActiveConfig = nullptr;
        return;
    }

    Hash model = ENTITY::GET_ENTITY_MODEL(mVehicle);
    std::string plate = VEHICLE::GET_VEHICLE_NUMBER_PLATE_TEXT(mVehicle);

    // First pass - match model and plate
    auto foundConfig = std::find_if(mConfigs.begin(), mConfigs.end(), [&](const CConfig& config) {
        bool modelMatch = config.ModelHash == model;
        bool plateMatch = Util::strcmpwi(config.Plate, plate);
        return modelMatch && plateMatch;
        });

    // second pass - match model with any plate
    if (foundConfig == mConfigs.end()) {
        foundConfig = std::find_if(mConfigs.begin(), mConfigs.end(), [&](const CConfig& config) {
            bool modelMatch = config.ModelHash == model;
            bool plateMatch = config.Plate.empty();
            return modelMatch && plateMatch;
            });
    }

    // third pass - use default
    if (foundConfig == mConfigs.end()) {
        mActiveConfig = &mDefaultConfig;
    }
    else {
        mActiveConfig = &*foundConfig;
        updateMods();
        ApplyConfig(*mActiveConfig, true, true, false);
    }

    initializeMods();
}

void CStanceScript::ApplyConfig(const CConfig& config, bool applyWheelMods, bool applyVisualHeight, bool replaceConfig) {
    auto wheels = VehicleExtensions::GetWheelPtrs(mVehicle);
    auto wheelOffsets = VehicleExtensions::GetWheelOffsets(mVehicle);

    std::vector<int> frontWheelIdxs;
    std::vector<int> rearWheelIdxs;
    for (uint32_t i = 0; i < wheels.size(); ++i) {
        if (wheelOffsets[i].y > 0.0f) {
            frontWheelIdxs.push_back(i);
        }
        else {
            rearWheelIdxs.push_back(i);
        }
    }

    auto wheelPtr = VExt::GetWheelsPtr(mVehicle);

    auto applySuspension = [](const CConfig::SSuspensionParams& suspension, uint64_t wheelPtr, uint8_t index) {
        float flip = index % 2 == 0 ? 1.0f : -1.0f; // cuz the wheels on the other side
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * index);
        *reinterpret_cast<float*>(wheelAddr + SuspensionOffsets::Camber) = suspension.Camber * flip;
        *reinterpret_cast<float*>(wheelAddr + SuspensionOffsets::CamberInv) = -suspension.Camber * flip;
        *reinterpret_cast<float*>(wheelAddr + SuspensionOffsets::TrackWidth) = -suspension.TrackWidth * flip;
        *reinterpret_cast<float*>(wheelAddr + SuspensionOffsets::Height) = suspension.Height;
    };

    auto applyWheel = [](const CConfig::SWheelColliderParams& wheelCol, uint64_t wheelPtr, uint8_t index) {
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * index);
        *reinterpret_cast<float*>(wheelAddr + SuspensionOffsets::TyreRadius) = wheelCol.TyreRadius;
        *reinterpret_cast<float*>(wheelAddr + SuspensionOffsets::TyreWidth) = wheelCol.TyreWidth;
        *reinterpret_cast<float*>(wheelAddr + SuspensionOffsets::RimRadius) = wheelCol.RimRadius;
    };

    for (auto wheelIndex : frontWheelIdxs) {
        applySuspension(config.Suspension.Front + mTotalFrontAdjust, wheelPtr, wheelIndex);
        applyWheel(config.Wheels.Front, wheelPtr, wheelIndex);
    }

    for (auto wheelIndex : rearWheelIdxs) {
        applySuspension(config.Suspension.Rear + mTotalRearAdjust, wheelPtr, wheelIndex);
        applyWheel(config.Wheels.Rear, wheelPtr, wheelIndex);
    }

    if (applyVisualHeight)
        VExt::SetVisualHeight(mVehicle, config.Suspension.VisualHeight);

    const std::unordered_map<int, float> dmgMap = {
        { 0, 1000.0f },
        { 1,  400.0f },
        { 2,    0.0f },
    };

    auto dmgValIt = dmgMap.find(config.Misc.DamageLoweringLevel);

    if (dmgValIt != dmgMap.end())
        VExt::SetWheelsHealth(mVehicle, dmgValIt->second);

    if (config.Wheels.Visual.WheelType != -1 && config.Wheels.Visual.ModIndex != -1) {
        if (applyWheelMods) {
            VEHICLE::SET_VEHICLE_MOD_KIT(mVehicle, 0);
            auto customtyres = VEHICLE::GET_VEHICLE_MOD_VARIATION(mVehicle, eVehicleMod::VehicleModFrontWheels);
            VEHICLE::SET_VEHICLE_WHEEL_TYPE(mVehicle, config.Wheels.Visual.WheelType);
            VEHICLE::SET_VEHICLE_MOD(mVehicle, VehicleModFrontWheels, config.Wheels.Visual.ModIndex, customtyres);
        
            for (auto wheelIndex : frontWheelIdxs) {
                applyWheel(config.Wheels.Front, wheelPtr, wheelIndex);
            }

            for (auto wheelIndex : rearWheelIdxs) {
                applyWheel(config.Wheels.Rear, wheelPtr, wheelIndex);
            }
        }

        VExt::SetVehicleWheelSize(mVehicle, config.Wheels.Visual.WheelSize);
        VExt::SetVehicleWheelWidth(mVehicle, config.Wheels.Visual.WheelWidth);
    }

    if (replaceConfig) {
        // Copy config into current
        mActiveConfig->Suspension = config.Suspension;
        mActiveConfig->Wheels = config.Wheels;
        mActiveConfig->Misc = config.Misc;
        mActiveConfig->ModAdjustments = config.ModAdjustments;
    }
}

void CStanceScript::initializeDefaultConfig() {
    Hash model = ENTITY::GET_ENTITY_MODEL(mVehicle);
    std::string plate = VEHICLE::GET_VEHICLE_NUMBER_PLATE_TEXT(mVehicle);
    std::string modelName;
    auto asCacheNameIt = ASCache::Get().find(model);
    if (asCacheNameIt != ASCache::Get().end())
        modelName = asCacheNameIt->second;

    mDefaultConfig.Name = "Default";
    mDefaultConfig.ModelName = modelName;
    mDefaultConfig.ModelHash = model;
    mDefaultConfig.Plate = plate;

    auto suspensionData = readSuspensionData();
    mDefaultConfig.Suspension = suspensionData;

    auto wheelData = readWheelData();
    mDefaultConfig.Wheels = wheelData;
}

void CStanceScript::initializeMods() {
    if (!mActiveConfig)
        return;

    mCurrentMods.clear();
    for (auto& adjustment : mActiveConfig->ModAdjustments)
        mCurrentMods[adjustment.ModSlot] = VEHICLE::GET_VEHICLE_MOD(mVehicle, adjustment.ModSlot);
}

void CStanceScript::update() {
    if (!mActiveConfig)
        return;

    updateMods();
    ApplyConfig(*mActiveConfig, false, false, false);
}

void CStanceScript::updateMods() {
    if (!mActiveConfig)
        return;

    bool updateActiveAdjustments = false;
    for (auto& [modSlot, modIndex] : mCurrentMods) {
        auto newModIndex = VEHICLE::GET_VEHICLE_MOD(mVehicle, modSlot);
        if (newModIndex != modIndex) {
            modIndex = newModIndex;
            updateActiveAdjustments = true;
        }
    }

    if (!updateActiveAdjustments)
        return;

    mActiveAdjustments.clear();

    for (const auto& [modSlot, modIndex] : mCurrentMods) {
        auto adjustIt = std::find_if(mActiveConfig->ModAdjustments.begin(), mActiveConfig->ModAdjustments.end(),
            [modIndex](const auto& adjust) {
                return modIndex == adjust.ModIndex;
            });

        if (adjustIt != mActiveConfig->ModAdjustments.end()) {
            mActiveAdjustments.push_back(*adjustIt);
        }
    }

    mTotalFrontAdjust = {};
    mTotalRearAdjust = {};

    for (const auto& adjustment : mActiveAdjustments) {
        mTotalFrontAdjust = mTotalFrontAdjust + adjustment.FrontAdjust;
        mTotalRearAdjust = mTotalRearAdjust + adjustment.RearAdjust;
    }
}

CConfig::SSuspensionTuning CStanceScript::readSuspensionData() {
    CConfig::SSuspensionTuning suspensionData{};
    auto wheels = VehicleExtensions::GetWheelPtrs(mVehicle);
    auto wheelOffsets = VehicleExtensions::GetWheelOffsets(mVehicle);

    std::vector<int> frontWheelIdxs;
    std::vector<int> rearWheelIdxs;
    for (uint32_t i = 0; i < wheels.size(); ++i) {
        if (wheelOffsets[i].y > 0.0f) {
            frontWheelIdxs.push_back(i);
        }
        else {
            rearWheelIdxs.push_back(i);
        }
    }

    auto wheelPtr = VExt::GetWheelsPtr(mVehicle);

    if (!frontWheelIdxs.empty()) {
        float flip = frontWheelIdxs[0] % 2 ? 1.0f : -1.0f;
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * frontWheelIdxs[0]);
        suspensionData.Front.Camber = *reinterpret_cast<float*>(wheelAddr + SuspensionOffsets::Camber);
        suspensionData.Front.TrackWidth = flip * *reinterpret_cast<float*>(wheelAddr + SuspensionOffsets::TrackWidth);
        suspensionData.Front.Height = *reinterpret_cast<float*>(wheelAddr + SuspensionOffsets::Height);
    }

    if (!rearWheelIdxs.empty()) {
        float flip = rearWheelIdxs[0] % 2 ? 1.0f : -1.0f;
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * rearWheelIdxs[0]);
        suspensionData.Rear.Camber = *reinterpret_cast<float*>(wheelAddr + SuspensionOffsets::Camber);
        suspensionData.Rear.TrackWidth = flip * *reinterpret_cast<float*>(wheelAddr + SuspensionOffsets::TrackWidth);
        suspensionData.Rear.Height = *reinterpret_cast<float*>(wheelAddr + SuspensionOffsets::Height);
    }

    suspensionData.VisualHeight = VExt::GetVisualHeight(mVehicle);

    return suspensionData;
}

CConfig::SWheelTuning CStanceScript::readWheelData() {
    CConfig::SWheelTuning wheelData{};
    auto wheels = VehicleExtensions::GetWheelPtrs(mVehicle);
    auto wheelOffsets = VehicleExtensions::GetWheelOffsets(mVehicle);

    std::vector<int> frontWheelIdxs;
    std::vector<int> rearWheelIdxs;
    for (uint32_t i = 0; i < wheels.size(); ++i) {
        if (wheelOffsets[i].y > 0.0f) {
            frontWheelIdxs.push_back(i);
        }
        else {
            rearWheelIdxs.push_back(i);
        }
    }

    auto wheelPtr = VExt::GetWheelsPtr(mVehicle);

    if (!frontWheelIdxs.empty()) {
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * frontWheelIdxs[0]);
        wheelData.Front.TyreRadius = *reinterpret_cast<float*>(wheelAddr + SuspensionOffsets::TyreRadius);
        wheelData.Front.TyreWidth = *reinterpret_cast<float*>(wheelAddr + SuspensionOffsets::TyreWidth);
        wheelData.Front.RimRadius = *reinterpret_cast<float*>(wheelAddr + SuspensionOffsets::RimRadius);
    }

    if (!rearWheelIdxs.empty()) {
        auto wheelAddr = *reinterpret_cast<uint64_t*>(wheelPtr + 0x008 * rearWheelIdxs.back());
        wheelData.Rear.TyreRadius = *reinterpret_cast<float*>(wheelAddr + SuspensionOffsets::TyreRadius);
        wheelData.Rear.TyreWidth = *reinterpret_cast<float*>(wheelAddr + SuspensionOffsets::TyreWidth);
        wheelData.Rear.RimRadius = *reinterpret_cast<float*>(wheelAddr + SuspensionOffsets::RimRadius);
    }

    wheelData.Visual = GetWheelRenderParams();

    return wheelData;
}

CConfig::SWheelRenderParams CStanceScript::GetWheelRenderParams() {
    CConfig::SWheelRenderParams visual{};

    auto wheelSize = VExt::GetVehicleWheelSize(mVehicle);
    auto wheelWidth = VExt::GetVehicleWheelWidth(mVehicle);

    if (wheelSize != 0.0f && wheelWidth != 0.0f) {
        visual.WheelSize = wheelSize;
        visual.WheelWidth = wheelWidth;
        visual.WheelType = VEHICLE::GET_VEHICLE_WHEEL_TYPE(mVehicle);
        visual.ModIndex = VEHICLE::GET_VEHICLE_MOD(mVehicle, eVehicleMod::VehicleModFrontWheels);
    }
    else {
        visual.WheelType = -1;
        visual.ModIndex = -1;
    }

    return visual;
}
