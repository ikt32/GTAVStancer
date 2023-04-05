#pragma once
#include "Config.hpp"
#include <inc/types.h>
#include <memory>
#include <unordered_map>
#include <vector>

class CStanceScript {
public:
    CStanceScript(Vehicle vehicle, std::vector<CConfig>& configs);
    ~CStanceScript();
    void Tick();

    CConfig* ActiveConfig() const {
        return mActiveConfig;
    }

    void UpdateActiveConfig();
    void ApplyConfig(const CConfig& config, bool applyWheelMods, bool applyVisualHeight);

    Vehicle GetVehicle() {
        return mVehicle;
    }

    CConfig::SWheelRenderParams GetWheelRenderParams();
private:
    void initializeDefaultConfig();
    void initializeMods();

    void update();
    void updateMods();

    CConfig::SSuspensionTuning readSuspensionData();
    CConfig::SWheelTuning readWheelData();


    CConfig mDefaultConfig;
    std::vector<CConfig>& mConfigs;

    Vehicle mVehicle;
    CConfig* mActiveConfig = nullptr;

    // Slot, Index
    std::unordered_map<int, int> mCurrentMods;
    std::vector<CConfig::SModAdjustment> mActiveAdjustments;

    CConfig::SSuspensionParams mTotalFrontAdjust{};
    CConfig::SSuspensionParams mTotalRearAdjust{};
};
