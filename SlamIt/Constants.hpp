#pragma once
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define VERSION_MAJOR 0
#define VERSION_MINOR 2
#define VERSION_PATCH 6

namespace Constants {
    static const char* const DisplayVersion = "v" STR(VERSION_MAJOR) "."  STR(VERSION_MINOR) "." STR(VERSION_PATCH);
    static const char* const ModDir = "\\VStancer";
    static const char* const NotificationPrefix = "~b~VStancer~w~";
}
