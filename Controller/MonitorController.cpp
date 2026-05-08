#include "MonitorController.h"
#include <algorithm>
#include <vector>

MonitorController::MonitorController(OrderModel&  orderModel,
                                     SampleModel& sampleModel,
                                     MonitorView& view)
    : orderModel_(orderModel), sampleModel_(sampleModel), view_(view) {}

void MonitorController::run() {
    while (true) {
        view_.showMenu();
        switch (view_.promptMenuChoice()) {
        case 1: handleOrderStats(); break;
        case 2: handleStockStats(); break;
        case 0: return;
        default: view_.showMessage("[오류] 잘못된 선택입니다.");
        }
    }
}

void MonitorController::handleOrderStats() {
    int reserved  = static_cast<int>(orderModel_.findByStatus(OrderStatus::RESERVED).size());
    int confirmed = static_cast<int>(orderModel_.findByStatus(OrderStatus::CONFIRMED).size());
    int producing = static_cast<int>(orderModel_.findByStatus(OrderStatus::PRODUCING).size());
    int released  = static_cast<int>(orderModel_.findByStatus(OrderStatus::RELEASED).size());
    view_.showOrderStats(reserved, confirmed, producing, released);
}

void MonitorController::handleStockStats() {
    auto allSamples = sampleModel_.all();
    std::vector<Sample> samples(allSamples.begin(), allSamples.end());
    std::vector<StockStatus> statuses;
    for (const auto& s : samples)
        statuses.push_back(calcStockStatus(s));
    view_.showStockStats(samples, statuses);
}

StockStatus MonitorController::calcStockStatus(const Sample& s) const {
    if (s.stock == 0) return StockStatus::DEPLETED;

    int demand = 0;
    for (const auto& o : orderModel_.findByStatus(OrderStatus::CONFIRMED))
        if (o.sampleId == s.id) demand += o.quantity;
    for (const auto& o : orderModel_.findByStatus(OrderStatus::RESERVED))
        if (o.sampleId == s.id) demand += o.quantity;

    return (demand > s.stock) ? StockStatus::SHORT : StockStatus::SURPLUS;
}
