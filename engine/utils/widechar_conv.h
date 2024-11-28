#pragma once

#if defined(_WIN32)
#include <windows.h>
#include <string>

namespace cortex::wc {

inline std::string WstringToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) {
        return std::string();
    }

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), NULL, 0, NULL, NULL);
    if (size_needed <= 0) {
        throw std::runtime_error("WideCharToMultiByte() failed: " + std::to_string(GetLastError()));
    }

    std::string result(size_needed, 0);
    int bytes_written = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), &result[0], size_needed, NULL, NULL);
    if (bytes_written <= 0) {
        throw std::runtime_error("WideCharToMultiByte() failed: " + std::to_string(GetLastError()));
    }

    return result;
}

inline std::wstring Utf8ToWstring(const std::string& str) {
    if (str.empty()) {
        return std::wstring();
    }

    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), NULL, 0);
    if (size_needed <= 0) {
        throw std::runtime_error("MultiByteToWideChar() failed: " + std::to_string(GetLastError()));
    }

    std::wstring result(size_needed, 0);
    int chars_written = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &result[0], size_needed);
    if (chars_written <= 0) {
        throw std::runtime_error("MultiByteToWideChar() failed: " + std::to_string(GetLastError()));
    }

    return result;
}

};

#endif