#include "MainController.h"
#include "../Utils/TimeUtils.h"
#include <format>
#include <iomanip>
#include <iostream>
#include <string>

void MainController::run() {
    loadData();

    bool running = true;
    while (running) {
        showMainMenu();

        std::string input;
        std::getline(std::cin, input);

        if (input.empty()) continue;

        int choice = -1;
        try { choice = std::stoi(input); } catch (...) {}

        switch (choice) {
            case 1: sampleCtrl_.run();     break;
            case 2: orderCtrl_.run();      break;
            case 3: approvalCtrl_.run();   break;
            case 4: monitorCtrl_.run();    break;
            case 5: productionCtrl_.run(); break;
            case 6: releaseCtrl_.run();    break;
            case 0:
                running = false;
                break;
            default:
                std::cout << "  잘못된 입력입니다. 다시 선택하세요.\n";
                break;
        }
    }

    saveData();
    std::cout << "\n  데이터를 저장했습니다. 시스템을 종료합니다.\n";
}

void MainController::loadData() {
    SampleStore ss(SAMPLE_FILE);
    ss.load(sampleModel_);
    OrderStore os(ORDER_FILE);
    os.load(orderModel_);
}

void MainController::saveData() const {
    SampleStore ss(SAMPLE_FILE);
    ss.save(sampleModel_);
    OrderStore os(ORDER_FILE);
    os.save(orderModel_);
}

void MainController::showMainMenu() const {
    int totalStock   = sampleModel_.totalStock();
    int sampleCount  = sampleModel_.count();
    int orderCount   = orderModel_.totalCount();
    int lineWaiting  = productionLine_.pendingCount() + (productionLine_.isRunning() ? 1 : 0);

    // 천 단위 구분자 포맷
    auto fmtNum = [](int n) -> std::string {
        std::string s = std::to_string(n);
        int insertPos = static_cast<int>(s.size()) - 3;
        while (insertPos > 0) {
            s.insert(static_cast<std::size_t>(insertPos), ",");
            insertPos -= 3;
        }
        return s;
    };

    std::string timeStr = TimeUtils::nowDateTime();

    std::cout << "\n============================================================\n";
    std::cout << "   반도체 시료 생산주문관리 시스템\n";
    std::cout << "============================================================\n";
    std::cout << std::format(" 시스템 현황  {}\n", timeStr);
    std::cout << "------------------------------------------------------------\n";
    std::cout << std::format(" 등록 시료  {:>5}종    총 재고  {:>8} ea\n",
        sampleCount, fmtNum(totalStock));
    std::cout << std::format(" 전체 주문  {:>5}건    생산라인  {:>5}건 대기\n",
        orderCount, lineWaiting);
    std::cout << "------------------------------------------------------------\n";
    std::cout << " [1] 시료 관리        [2] 시료 주문\n";
    std::cout << " [3] 주문 승인/거절   [4] 모니터링\n";
    std::cout << " [5] 생산라인 조회    [6] 출고 처리\n";
    std::cout << " [0] 종료\n";
    std::cout << "------------------------------------------------------------\n";
    std::cout << "선택 > ";
}
