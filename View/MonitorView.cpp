#include "MonitorView.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>

MonitorView::MonitorView() : hOut_(GetStdHandle(STD_OUTPUT_HANDLE)) {
    CONSOLE_SCREEN_BUFFER_INFO info{};
    GetConsoleScreenBufferInfo(hOut_, &info);
    defaultAttr_ = info.wAttributes;
}

MonitorView::~MonitorView() { resetColor(); }

void MonitorView::setColor(C c) const {
    SetConsoleTextAttribute(hOut_, static_cast<WORD>(c));
}

void MonitorView::resetColor() const {
    SetConsoleTextAttribute(hOut_, defaultAttr_);
}

void MonitorView::showMenu() const {
    std::cout << "============================================================\n";
    std::cout << " [4] 모니터링\n";
    std::cout << "------------------------------------------------------------\n";
    std::cout << " [1] 주문량 확인   [2] 재고량 확인   [0] 위로\n";
    std::cout << "선택 > ";
}

void MonitorView::showOrderStats(int reserved, int confirmed,
                                  int producing, int released) const {
    std::cout << "============================================================\n";
    std::cout << " 주문 현황  (상태별)\n";
    std::cout << "------------------------------------------------------------\n";

    setColor(C::White);
    std::cout << "  RESERVED   : " << std::setw(4) << reserved << "건\n";
    resetColor();

    setColor(C::Green);
    std::cout << "  CONFIRMED  : " << std::setw(4) << confirmed << "건\n";
    resetColor();

    setColor(C::Yellow);
    std::cout << "  PRODUCING  : " << std::setw(4) << producing << "건\n";
    resetColor();

    setColor(C::Gray);
    std::cout << "  RELEASED   : " << std::setw(4) << released << "건\n";
    resetColor();

    std::cout << "------------------------------------------------------------\n";
    int total = reserved + confirmed + producing + released;
    std::cout << "  유효 주문 합계: " << total << "건  (REJECTED 제외)\n";
    std::cout << "============================================================\n";
}

void MonitorView::showStockStats(const std::vector<Sample>& samples,
                                  const std::vector<StockStatus>& statuses) const {
    std::cout << "============================================================\n";
    std::cout << " 재고 현황  (시료별)\n";
    std::cout << "------------------------------------------------------------\n";
    std::cout << "  시료명                  재고      상태   잔여율\n";

    int maxStock = 0;
    for (const auto& s : samples) {
        if (s.stock > maxStock) maxStock = s.stock;
    }
    if (maxStock == 0) maxStock = 1;

    for (std::size_t i = 0; i < samples.size(); ++i) {
        const auto& s = samples[i];
        StockStatus st = statuses[i];

        std::cout << "  " << std::left << std::setw(20) << s.name
                  << std::right << std::setw(6) << s.stock << " ea   ";

        if (st == StockStatus::SURPLUS) {
            setColor(C::Green);
            std::cout << "여유";
        } else if (st == StockStatus::SHORT) {
            setColor(C::Yellow);
            std::cout << "부족";
        } else {
            setColor(C::Red);
            std::cout << "고갈";
        }
        resetColor();

        std::cout << "   ";
        printBar(s.stock, maxStock);

        int pct = s.stock * 100 / maxStock;
        std::cout << " " << std::setw(3) << pct << "%\n";
    }

    std::cout << "------------------------------------------------------------\n";
    std::cout << "============================================================\n";
}

void MonitorView::printBar(int stock, int maxStock, int width) const {
    int filled = (maxStock > 0) ? (stock * width / maxStock) : 0;
    setColor(C::Green);
    for (int i = 0; i < filled; ++i) std::cout << "\xe2\x96\x88"; // █
    setColor(C::Gray);
    for (int i = filled; i < width; ++i) std::cout << "\xe2\x96\x91"; // ░
    resetColor();
}

void MonitorView::showMessage(const std::string& msg) const {
    std::cout << msg << "\n";
}

int MonitorView::promptMenuChoice() const {
    std::string line;
    std::getline(std::cin, line);
    try {
        return std::stoi(line);
    } catch (...) {
        return -1;
    }
}
