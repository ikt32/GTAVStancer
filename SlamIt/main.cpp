#include "script.h"
#include "keyboard.h"
#include "Util/Paths.h"
#include "Util/Logger.hpp"
#include "Util/Versions.h"
#include "Constants.hpp"


BOOL APIENTRY DllMain(HMODULE hInstance, DWORD reason, LPVOID lpReserved) {
    std::string logFile = Paths::GetModuleFolder(hInstance) + Constants::ModDir +
        "\\" + Paths::GetModuleNameWithoutExtension(hInstance) + ".log";
    logger.SetFile(logFile);
    Paths::SetOurModuleHandle(hInstance);
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        scriptRegister(hInstance, ScriptMain);
        logger.Clear();
        logger.Write(INFO, "VStancer " + std::string(Constants::DisplayVersion));
        logger.Write(INFO, "Game version " + eGameVersionToString(getGameVersion()));
        if (getGameVersion() < G_VER_1_0_944_2_STEAM) {
            logger.Write(WARN, "Incompatible game version. Update your game!");
        }
        logger.Write(INFO, "Script registered");
        break;
    case DLL_PROCESS_DETACH:
        scriptUnregister(hInstance);
        break;
    }
    return TRUE;
}
