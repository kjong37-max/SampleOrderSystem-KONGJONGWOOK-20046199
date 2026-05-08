#define NOMINMAX
#include <Windows.h>
#include "TimeUtils.h"
#include <format>

namespace TimeUtils {

std::string nowDateTime() {
    SYSTEMTIME st{};
    GetLocalTime(&st);
    return std::format("{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond);
}

std::string nowDate() {
    SYSTEMTIME st{};
    GetLocalTime(&st);
    return std::format("{:04d}{:02d}{:02d}",
        st.wYear, st.wMonth, st.wDay);
}

} // namespace TimeUtils
