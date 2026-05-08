#include "OrderStore.h"

OrderStore::OrderStore(const std::string& filePath)
    : filePath_(filePath)
{
}

void OrderStore::load(OrderModel& model) {
    DataStore store(filePath_);
    store.load();

    // 채번 상태 복원
    model.setLastDate(store.get("order.lastDate", ""));
    try { model.setDailySeq(std::stoi(store.get("order.dailySeq", "0"))); } catch (...) {}

    int count = 0;
    try { count = std::stoi(store.get("order.count", "0")); } catch (...) {}
    for (int i = 0; i < count; ++i) {
        std::string prefix = "order." + std::to_string(i);
        Order o;
        o.orderId   = store.get(prefix + ".orderId");
        o.sampleId  = store.get(prefix + ".sampleId");
        o.customer  = store.get(prefix + ".customer");
        o.status    = strToOrderStatus(store.get(prefix + ".status", "RESERVED"));
        o.createdAt = store.get(prefix + ".createdAt");
        try { o.quantity = std::stoi(store.get(prefix + ".quantity", "0")); } catch (...) {}
        if (!o.orderId.empty()) model.addDirect(o);
    }
}

void OrderStore::save(const OrderModel& model) const {
    DataStore store(filePath_);
    store.set("order.lastDate", model.lastDate());
    store.set("order.dailySeq", std::to_string(model.dailySeq()));

    auto orders = model.all();
    store.set("order.count", std::to_string(orders.size()));
    for (int i = 0; i < static_cast<int>(orders.size()); ++i) {
        std::string prefix = "order." + std::to_string(i);
        store.set(prefix + ".orderId",   orders[i].orderId);
        store.set(prefix + ".sampleId",  orders[i].sampleId);
        store.set(prefix + ".customer",  orders[i].customer);
        store.set(prefix + ".quantity",  std::to_string(orders[i].quantity));
        store.set(prefix + ".status",    orderStatusToStr(orders[i].status));
        store.set(prefix + ".createdAt", orders[i].createdAt);
    }
    store.save();
}
