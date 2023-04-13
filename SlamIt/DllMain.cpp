#include "Script.hpp"
#include "Constants.hpp"

#include "Util/FileVersion.hpp"
#include "Util/Logger.hpp"
#include "Util/Paths.hpp"
#include "Patching/SuspensionPatch.hpp"
#include "Memory/Versions.hpp"
#include "Memory/VehicleExtensions.hpp"

#include <inc/main.h>

#include <filesystem>

namespace fs = std::filesystem;

void resolveVersion() {
    int shvVersion = getGameVersion();

    LOG(INFO, "SHV API Game version: {} ({})", eGameVersionToString(shvVersion), shvVersion);
    SVersion exeVersion = getExeInfo();

    // Version we *explicitly* support
    std::vector<int> exeVersionsSupp = findNextLowest(ExeVersionMap, exeVersion);
    if (exeVersionsSupp.empty() || exeVersionsSupp.size() == 1 && exeVersionsSupp[0] == -1) {
        LOG(ERROR, "Failed to find a corresponding game version.");
        LOG(WARN, "    Using SHV API version [{}] ({})",
            eGameVersionToString(shvVersion), shvVersion);
        VehicleExtensions::SetVersion(shvVersion);
        return;
    }

    int highestSupportedVersion = *std::max_element(std::begin(exeVersionsSupp), std::end(exeVersionsSupp));
    if (shvVersion > highestSupportedVersion) {
        LOG(WARN, "Game newer than last supported version");
        LOG(WARN, "    You might experience instabilities or crashes");
        LOG(WARN, "    Using SHV API version [{}] ({})",
            eGameVersionToString(shvVersion), shvVersion);
        VehicleExtensions::SetVersion(shvVersion);
        return;
    }

    int lowestSupportedVersion = *std::min_element(std::begin(exeVersionsSupp), std::end(exeVersionsSupp));
    if (shvVersion < lowestSupportedVersion) {
        LOG(WARN, "SHV API reported lower version than actual EXE version.");
        LOG(WARN, "    EXE version     [{}] ({})",
            eGameVersionToString(lowestSupportedVersion), lowestSupportedVersion);
        LOG(WARN, "    SHV API version [{}] ({})",
            eGameVersionToString(shvVersion), shvVersion);
        LOG(WARN, "    Using EXE version, or highest supported version [{}] ({})",
            eGameVersionToString(lowestSupportedVersion), lowestSupportedVersion);
        VehicleExtensions::SetVersion(lowestSupportedVersion);
        return;
    }

    LOG(DEBUG, "Using offsets based on SHV API version [{}] ({})",
        eGameVersionToString(shvVersion), shvVersion);
    VehicleExtensions::SetVersion(shvVersion);
}

void initializePaths(HMODULE hInstance) {
    Paths::SetOurModuleHandle(hInstance);

    auto localAppDataPath = Paths::GetLocalAppDataPath();
    auto localAppDataModPath = localAppDataPath / Constants::appdataFolder / Constants::ScriptFolder;
    auto originalModPath = Paths::GetModuleFolder(hInstance) / Constants::ScriptFolder;
    Paths::SetModPath(originalModPath);

    bool useAltModPath = false;
    if (fs::exists(localAppDataModPath)) {
        useAltModPath = true;
    }

    fs::path modPath;
    fs::path logFile;

    // Use LocalAppData if it already exists.
    if (useAltModPath) {
        modPath = localAppDataModPath;
        logFile = localAppDataModPath / (Paths::GetModuleNameWithoutExtension(hInstance) + ".log");
    }
    else {
        modPath = originalModPath;
        logFile = modPath / (Paths::GetModuleNameWithoutExtension(hInstance) + ".log");
    }

    Paths::SetModPath(modPath);

    if (!fs::is_directory(modPath) || !fs::exists(modPath)) {
        fs::create_directories(modPath);
    }

    g_Logger.SetFile(logFile.string());
    g_Logger.Clear();

    if (g_Logger.Error()) {
        modPath = localAppDataModPath;
        logFile = localAppDataModPath / (Paths::GetModuleNameWithoutExtension(hInstance) + ".log");
        fs::create_directories(modPath);

        Paths::SetModPath(modPath);
        g_Logger.SetFile(logFile.string());

        fs::copy(originalModPath, localAppDataModPath,
            fs::copy_options::update_existing | fs::copy_options::recursive);

        std::vector<std::string> messages;

        // Fix perms
        for (auto& path : fs::recursive_directory_iterator(localAppDataModPath)) {
            try {
                fs::permissions(path, fs::perms::all);
            }
            catch (std::exception& e) {
                messages.push_back(
                    std::format("Failed to set permissions on [{}]: {}",
                        path.path().string(), e.what()));
            }
        }

        g_Logger.ClearError();
        g_Logger.Clear();

        LOG(WARN, "Copied to [{}] from [{}] due to read/write issues.",
            modPath.string(), originalModPath.string());

        if (!messages.empty()) {
            LOG(WARN, "Encountered issues while updating permissions:");
            for (const auto& message : messages) {
                LOG(WARN, "{}", message);
            }
        }
    }
}

BOOL APIENTRY DllMain(HMODULE hInstance, DWORD reason, LPVOID lpReserved) {
    switch (reason) {
        case DLL_PROCESS_ATTACH: {
            g_Logger.SetMinLevel(DEBUG);
            initializePaths(hInstance);
            LOG(INFO, "{} {} (built {} {})", Constants::ScriptName, Constants::DisplayVersion, __DATE__, __TIME__);
            resolveVersion();
            LOG(INFO, "Data path: {}", Paths::GetModPath().string());

            scriptRegister(hInstance, VStancer::ScriptMain);
            LOG(INFO, "Script registered");
            break;
        }
        case DLL_PROCESS_DETACH: {
            VStancer::UnpatchHeightReset();
            scriptUnregister(hInstance);
            break;
        }
        default: {
            break;
        }
    }
    return TRUE;
}
