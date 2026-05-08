#include "ProductionController.h"
#include <iostream>
#include <string>

ProductionController::ProductionController(OrderModel&     orderModel,
                                           SampleModel&    sampleModel,
                                           ProductionLine& productionLine,
                                           ProductionView& view)
    : orderModel_(orderModel)
    , sampleModel_(sampleModel)
    , productionLine_(productionLine)
    , view_(view)
{}

void ProductionController::run() {
    while (true) {
        view_.showMenu();
        switch (view_.promptMenuChoice()) {
        case 1: handleStatus();   break;
        case 2: handleComplete(); break;
        case 0: return;
        default: view_.showError("잘못된 선택입니다.");
        }
    }
}

void ProductionController::handleStatus() {
    view_.showProductionStatus(
        productionLine_.currentJob(),
        productionLine_.pendingJobs());
}

void ProductionController::handleComplete() {
    auto current = productionLine_.currentJob();
    if (!current) {
        view_.showMessage("현재 처리 중인 생산 작업이 없습니다.");
        return;
    }

    view_.showProductionStatus(current, productionLine_.pendingJobs());

    std::cout << "현재 작업을 완료 처리하시겠습니까? (Y/N) > ";
    std::string ans;
    std::getline(std::cin, ans);
    if (ans.empty() || (ans[0] != 'Y' && ans[0] != 'y')) return;

    // 재고 보충
    sampleModel_.updateStock(current->sampleId, current->actualQty);

    // 주문 상태 → CONFIRMED
    orderModel_.updateStatus(current->orderId, OrderStatus::CONFIRMED);

    // 다음 작업으로 이동
    auto done = productionLine_.completeCurrentJob();
    if (done) view_.showCompleteResult(*done);
}
