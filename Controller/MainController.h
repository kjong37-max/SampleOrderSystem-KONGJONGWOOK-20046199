#pragma once
#define NOMINMAX
#include <Windows.h>
#include "../Model/SampleModel.h"
#include "../Model/OrderModel.h"
#include "../Model/ProductionLine.h"
#include "../View/SampleView.h"
#include "../View/OrderView.h"
#include "../View/ProductionView.h"
#include "../View/MonitorView.h"
#include "../Controller/SampleController.h"
#include "../Controller/OrderController.h"
#include "../Controller/ApprovalController.h"
#include "../Controller/ProductionController.h"
#include "../Controller/ReleaseController.h"
#include "../Controller/MonitorController.h"
#include "../Data/SampleStore.h"
#include "../Data/OrderStore.h"
#include <string>

class MainController {
public:
    void run();

private:
    void loadData();
    void saveData() const;
    void showMainMenu() const;

    // Models
    SampleModel    sampleModel_;
    OrderModel     orderModel_;
    ProductionLine productionLine_;

    // Views
    SampleView     sampleView_;
    OrderView      orderView_;
    ProductionView productionView_;
    MonitorView    monitorView_;

    // Sub-Controllers (멤버 초기화 순서 주의: Models/Views 먼저)
    SampleController     sampleCtrl_{     sampleModel_, sampleView_ };
    OrderController      orderCtrl_{      orderModel_,  sampleModel_, orderView_ };
    ApprovalController   approvalCtrl_{   orderModel_,  sampleModel_,
                                          productionLine_, orderView_ };
    ProductionController productionCtrl_{ orderModel_,  sampleModel_,
                                          productionLine_, productionView_ };
    ReleaseController    releaseCtrl_{    orderModel_,  sampleModel_, orderView_ };
    MonitorController    monitorCtrl_{    orderModel_,  sampleModel_, monitorView_ };

    static constexpr const char* SAMPLE_FILE = "Data/samples.txt";
    static constexpr const char* ORDER_FILE  = "Data/orders.txt";
};
