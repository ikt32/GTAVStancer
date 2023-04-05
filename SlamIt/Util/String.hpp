#pragma once
#include <algorithm> 
#include <cctype>
#include <locale>
#include <vector>

namespace Util {
    constexpr unsigned long joaat(const char* s) {
        unsigned long hash = 0;
        for (; *s != '\0'; ++s) {
            auto c = *s;
            if (c >= 0x41 && c <= 0x5a) {
                c += 0x20;
            }
            hash += c;
            hash += hash << 10;
            hash ^= hash >> 6;
        }
        hash += hash << 3;
        hash ^= hash >> 11;
        hash += hash << 15;
        return hash;
    }

    std::string to_lower(std::string s);

    std::vector<std::string> split(const std::string& s, char delim);

    std::string ByteArrayToString(uint8_t* byteArray, size_t length);

    // trim from start (in place)
    void ltrim(std::string& s);

    // trim from end (in place)
    void rtrim(std::string& s);

    // trim from both ends (in place)
    void trim(std::string& s);

    // trim from start (copying)
    std::string ltrim_copy(std::string s);

    // trim from end (copying)
    std::string rtrim_copy(std::string s);

    // trim from both ends (copying)
    std::string trim_copy(std::string s);

    bool strcmpwi(std::string a, std::string b);
}
