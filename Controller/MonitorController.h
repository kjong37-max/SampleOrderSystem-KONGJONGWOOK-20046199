#pragma once
#include "../Model/OrderModel.h"
#include "../Model/SampleModel.h"
#include "../View/MonitorView.h"

class MonitorController {
public:
    MonitorController(OrderModel&  orderModel,
                      SampleModel& sampleModel,
                      MonitorView& view);
    void run();

private:
    void handleOrderStats();
    void handleStockStats();
    StockStatus calcStockStatus(const Sample& sample) const;

    OrderModel&  orderModel_;
    SampleModel& sampleModel_;
    MonitorView& view_;
};
