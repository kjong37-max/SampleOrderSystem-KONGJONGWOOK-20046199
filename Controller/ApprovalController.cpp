#include "ApprovalController.h"
#include "../Model/Constants.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

ApprovalController::ApprovalController(OrderModel&     orderModel,
                                       SampleModel&    sampleModel,
                                       ProductionLine& productionLine,
                                       OrderView&      view)
    : orderModel_(orderModel)
    , sampleModel_(sampleModel)
    , productionLine_(productionLine)
    , view_(view)
{}

void ApprovalController::run() {
    while (true) {
        auto reserved = orderModel_.findByStatus(OrderStatus::RESERVED);
        view_.showReservedList(reserved, sampleModel_.all());

        if (reserved.empty()) {
            std::cout << "계속하려면 Enter...";
            std::string dummy; std::getline(std::cin, dummy);
            return;
        }

        std::cout << "처리할 번호 (0: 위로) > ";
        std::string line; std::getline(std::cin, line);
        int idx = -1;
        try { idx = std::stoi(line); } catch (...) {}

        if (idx == 0) return;
        if (idx < 1 || idx > static_cast<int>(reserved.size())) {
            std::cout << "[오류] 올바른 번호를 입력하세요.\n";
            continue;
        }

        const Order& selected = reserved[idx - 1];

        std::cout << "[1] 승인   [2] 거절   [0] 취소 > ";
        std::getline(std::cin, line);

        if (line == "1")      handleApproval(selected);
        else if (line == "2") {
            orderModel_.updateStatus(selected.orderId, OrderStatus::REJECTED);
            std::cout << "거절 처리되었습니다. RESERVED → REJECTED\n";
        }
        // 0 또는 기타 → continue
    }
}

void ApprovalController::handleApproval(const Order& order) {
    auto sampleOpt = sampleModel_.findById(order.sampleId);
    if (!sampleOpt) {
        std::cout << "[오류] 시료를 찾을 수 없습니다.\n";
        return;
    }
    Sample sample = *sampleOpt;
    int effectiveStock = sampleModel_.effectiveStock(order.sampleId, orderModel_);

    // 재고 정보 표시
    std::cout << "------------------------------------------------------------\n";
    std::cout << "  시료     : " << sample.name << "  (" << sample.id << ")\n";
    std::cout << "  주문 수량 : " << order.quantity << " ea\n";
    std::cout << "  현재 재고 : " << sample.stock << " ea\n";
    std::cout << "  유효 재고 : " << effectiveStock << " ea  (CONFIRMED 대기분 차감 후)\n";
    std::cout << "------------------------------------------------------------\n";

    if (effectiveStock >= order.quantity) {
        // 재고 충분
        sampleModel_.updateStock(order.sampleId, -order.quantity);
        orderModel_.updateStatus(order.orderId, OrderStatus::CONFIRMED);
        std::cout << "[승인 완료] RESERVED → CONFIRMED\n";
    } else {
        // 재고 부족
        int shortage  = order.quantity - std::max(effectiveStock, 0);
        int actualQty = static_cast<int>(std::ceil(
                            static_cast<double>(shortage) /
                            (sample.yield * SemiConst::PRODUCTION_CORRECTION)));
        double totalMin = sample.avgProdTime * actualQty;

        std::cout << "  재고 부족.  부족분: " << shortage << " ea\n";
        std::cout << "  실 생산량 : " << actualQty << " ea"
                  << "   총 생산시간: " << totalMin << " min\n";
        std::cout << "[Y] 승인(생산 등록)   [N] 거절 > ";

        std::string ans; std::getline(std::cin, ans);
        if (!ans.empty() && (ans[0] == 'Y' || ans[0] == 'y')) {
            if (effectiveStock > 0)
                sampleModel_.updateStock(order.sampleId, -effectiveStock);
            ProductionJob job{ order.orderId, order.sampleId,
                               shortage, actualQty, totalMin };
            productionLine_.enqueue(job);
            orderModel_.updateStatus(order.orderId, OrderStatus::PRODUCING);
            std::cout << "[승인 완료] RESERVED → PRODUCING  (생산라인 등록)\n";
        } else {
            orderModel_.updateStatus(order.orderId, OrderStatus::REJECTED);
            std::cout << "[거절 처리] RESERVED → REJECTED\n";
        }
    }
}
