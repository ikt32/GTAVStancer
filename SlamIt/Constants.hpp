#pragma once
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define VERSION_MAJOR 0
#define VERSION_MINOR 3
#define VERSION_PATCH 3

namespace Constants {
    static const char* const DisplayVersion = "v" STR(VERSION_MAJOR) "."  STR(VERSION_MINOR) "." STR(VERSION_PATCH);

    static const char* const ScriptName = "VStancer";
    static const char* const NotificationPrefix = "~b~VStancer~w~";
    static const char* const appdataFolder = "ikt";
    static const char* const ScriptFolder = "VStancer";
}
