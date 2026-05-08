#include "ProductionView.h"
#include <iomanip>
#include <iostream>
#include <sstream>

void ProductionView::showMenu() const {
    std::cout << "============================================================\n"
              << " [5] 생산라인 조회\n"
              << "------------------------------------------------------------\n"
              << " [1] 생산 현황 조회   [2] 생산 완료 처리   [0] 위로\n"
              << "선택 > ";
}

void ProductionView::showProductionStatus(
    const std::optional<ProductionJob>& current,
    const std::vector<ProductionJob>&   pending) const
{
    std::cout << "============================================================\n"
              << " 생산라인 현황\n"
              << "------------------------------------------------------------\n"
              << " ■ 현재 처리 중\n";

    if (!current) {
        std::cout << "  현재 처리 중인 작업이 없습니다.\n";
    } else {
        std::cout << "   주문번호   : " << current->orderId      << "\n"
                  << "   시료 ID   : " << current->sampleId     << "\n"
                  << "   부족분    : " << current->shortage      << " ea\n"
                  << "   실 생산량  : " << current->actualQty    << " ea\n"
                  << "   총 생산시간: "
                  << std::fixed << std::setprecision(1)
                  << current->totalMinutes << " min\n";
    }

    std::cout << "------------------------------------------------------------\n"
              << " ■ 대기 중인 주문  (FIFO 순)\n";

    if (pending.empty()) {
        std::cout << "  대기 중인 작업이 없습니다.\n";
    } else {
        std::cout << " 순서  주문번호              시료 ID  부족분    실생산량  총생산시간\n";
        int idx = 1;
        for (const auto& job : pending) {
            std::cout << "  [" << idx << "]"
                      << "  " << std::left << std::setw(20) << job.orderId
                      << "  " << std::setw(5) << job.sampleId
                      << "  " << std::right << std::setw(3) << job.shortage << " ea"
                      << "   " << std::setw(3) << job.actualQty << " ea"
                      << "   "
                      << std::fixed << std::setprecision(1)
                      << job.totalMinutes << " min\n";
            ++idx;
        }
    }

    std::cout << "------------------------------------------------------------\n"
              << " 대기 건수: " << pending.size() << "건\n"
              << "============================================================\n";
}

void ProductionView::showCompleteResult(const ProductionJob& job) const {
    std::cout << "------------------------------------------------------------\n"
              << " 생산 완료 처리되었습니다.\n"
              << "\n"
              << " 주문번호  : " << job.orderId  << "\n"
              << " 시료 ID  : " << job.sampleId  << "\n"
              << " 생산 수량 : " << job.actualQty << " ea  (재고 보충)\n"
              << " 상태 변경 : PRODUCING → CONFIRMED\n"
              << "------------------------------------------------------------\n";
}

void ProductionView::showMessage(const std::string& msg) const {
    std::cout << msg << "\n";
}

void ProductionView::showError(const std::string& msg) const {
    std::cout << "[오류] " << msg << "\n";
}

int ProductionView::promptMenuChoice() const {
    std::string line;
    std::getline(std::cin, line);
    try {
        return std::stoi(line);
    } catch (...) {
        return -1;
    }
}
