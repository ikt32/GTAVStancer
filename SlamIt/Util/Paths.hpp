#pragma once
#include <Windows.h>
#include <filesystem>
#include <string>

namespace Paths {
    std::filesystem::path GetLocalAppDataPath();

    void SetModPath(std::filesystem::path path);
    std::filesystem::path GetModPath();
    std::filesystem::path GetInitialModPath();

    void SetModPathChanged();
    bool GetModPathChanged();

    std::filesystem::path GetRunningExecutablePath();

    std::filesystem::path GetModuleFolder(HMODULE module);
    std::string	GetModuleName(HMODULE module);
    std::string	GetModuleNameWithoutExtension(HMODULE module);

    void SetOurModuleHandle(HMODULE module);
    HMODULE GetOurModuleHandle();

    std::wstring GetDocumentsFolder();
}
