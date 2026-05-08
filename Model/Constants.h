#pragma once

namespace SemiConst {
    // 생산량 보정 비율 (PRD 고정 상수)
    // 실 생산량 = ceil(부족분 / (수율 × PRODUCTION_CORRECTION))
    constexpr double PRODUCTION_CORRECTION = 0.9;

    // 시료 목록 페이지 크기
    constexpr int PAGE_SIZE = 5;

    // 모니터링 재고 바 그래프 너비 (칸 수)
    constexpr int BAR_WIDTH = 20;
}
