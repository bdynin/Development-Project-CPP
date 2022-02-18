#pragma once

#include <codecvt>
#include <locale>
#include <string>

#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING 1

namespace iti {

inline std::string WstrToStr(const std::wstring &source) {
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
    std::string dest = convert.to_bytes((std::u16string &)source);
    return dest;
}

inline std::wstring StrToWstr(const std::string &source) {
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
    std::u16string dest = convert.from_bytes(source);
    return (std::wstring &)dest;
}
} // namespace iti
 