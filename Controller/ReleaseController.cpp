#define NOMINMAX
#include <Windows.h>
#include "ReleaseController.h"
#include <format>
#include <iomanip>
#include <iostream>
#include <string>

ReleaseController::ReleaseController(OrderModel& orderModel,
                                     SampleModel& sampleModel,
                                     OrderView& view)
    : orderModel_(orderModel), sampleModel_(sampleModel), view_(view) {}

void ReleaseController::run() {
    while (true) {
        auto confirmed = orderModel_.findByStatus(OrderStatus::CONFIRMED);

        if (confirmed.empty()) {
            std::cout << "============================================================\n"
                      << " [6] 출고 처리\n"
                      << "------------------------------------------------------------\n"
                      << " 출고 가능한 주문이 없습니다.\n"
                      << "------------------------------------------------------------\n"
                      << "계속하려면 Enter...";
            std::string dummy; std::getline(std::cin, dummy);
            return;
        }

        showConfirmedList(confirmed);

        std::cout << "출고할 번호 (0: 위로) > ";
        std::string line; std::getline(std::cin, line);
        int idx = -1;
        try { idx = std::stoi(line); } catch (...) {}

        if (idx == 0) return;
        if (idx < 1 || idx > static_cast<int>(confirmed.size())) {
            std::cout << "[오류] 올바른 번호를 입력하세요.\n";
            continue;
        }

        const Order& selected = confirmed[idx - 1];

        // 출고 처리 (재고 차감 없음 — Phase 4에서 이미 차감됨)
        orderModel_.updateStatus(selected.orderId, OrderStatus::RELEASED);

        std::cout << "------------------------------------------------------------\n"
                  << " 출고 처리 완료.\n\n"
                  << " 주문번호  : " << selected.orderId  << "\n"
                  << " 고객명   : " << selected.customer  << "\n"
                  << " 시료 ID  : " << selected.sampleId  << "\n"
                  << " 출고 수량 : " << selected.quantity  << " ea\n"
                  << " 처리 일시 : " << getCurrentDateTime() << "\n"
                  << " 상태 변경 : CONFIRMED → RELEASED\n"
                  << "------------------------------------------------------------\n";
    }
}

void ReleaseController::showConfirmedList(const std::vector<Order>& orders) const {
    std::cout << "============================================================\n"
              << " [6] 출고 처리\n"
              << "------------------------------------------------------------\n"
              << " 출고 가능 주문  (CONFIRMED)\n"
              << std::left
              << std::setw(6)  << " 번호"
              << std::setw(22) << "주문번호"
              << std::setw(18) << "고객"
              << std::setw(10) << "시료 ID"
              << "수량\n"
              << "------------------------------------------------------------\n";

    for (int i = 0; i < static_cast<int>(orders.size()); ++i) {
        const auto& o = orders[i];
        std::cout << std::left
                  << "  [" << std::setw(2) << (i + 1) << "] "
                  << std::setw(22) << o.orderId
                  << std::setw(18) << o.customer
                  << std::setw(10) << o.sampleId
                  << o.quantity << " ea\n";
    }
    std::cout << "------------------------------------------------------------\n";
}

std::string ReleaseController::getCurrentDateTime() const {
    SYSTEMTIME st{};
    GetLocalTime(&st);
    return std::format("{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond);
}
