#pragma once
#include <string>

namespace TimeUtils {
    // "YYYY-MM-DD HH:MM:SS" 형식 현재 시각 반환
    std::string nowDateTime();

    // "YYYYMMDD" 형식 현재 날짜 반환
    std::string nowDate();
}
