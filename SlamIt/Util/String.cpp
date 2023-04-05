#include "String.hpp"

#include <sstream>
#include <algorithm>

namespace {
    template<typename Out>
    void split(const std::string& s, char delim, Out result) {
        std::stringstream ss;
        ss.str(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            *(result++) = item;
        }
    }
}

namespace Util {
    std::string to_lower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    }

    std::vector<std::string> split(const std::string& s, char delim) {
        std::vector<std::string> elems;
        ::split(s, delim, std::back_inserter(elems));
        return elems;
    }

    std::string ByteArrayToString(uint8_t* byteArray, size_t length) {
        std::string instructionBytes;
        for (int i = 0; i < length; ++i) {
            char buff[4];
            snprintf(buff, 4, "%02X ", byteArray[i]);
            instructionBytes += buff;
        }
        return instructionBytes;
    }

    // trim from start (in place)
    void ltrim(std::string& s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
            }));
    }

    // trim from end (in place)
    void rtrim(std::string& s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
            }).base(), s.end());
    }

    // trim from both ends (in place)
    void trim(std::string& s) {
        ltrim(s);
        rtrim(s);
    }

    // trim from start (copying)
    std::string ltrim_copy(std::string s) {
        ltrim(s);
        return s;
    }

    // trim from end (copying)
    std::string rtrim_copy(std::string s) {
        rtrim(s);
        return s;
    }

    // trim from both ends (copying)
    std::string trim_copy(std::string s) {
        trim(s);
        return s;
    }

    bool strcmpwi(std::string a, std::string b) {
        trim(a);
        trim(b);
        return to_lower(a) == to_lower(b);
    }
}
