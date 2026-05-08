#pragma once
#include <string>

struct ProductionJob {
    std::string orderId;
    std::string sampleId;
    int         shortage;      // 부족분
    int         actualQty;     // ceil(부족분 / (수율 × 0.9))
    double      totalMinutes;  // avgProdTime × actualQty
};
