#pragma once
#include <string>

struct Sample {
    std::string id;           // 고유 식별자. 형식: "S-NNN"
    std::string name;         // 시료명
    double      avgProdTime;  // 평균 생산시간 (min/ea)
    double      yield;        // 수율 (0.0 ~ 1.0)
    int         stock;        // 현재 재고 수량 (ea)
};
