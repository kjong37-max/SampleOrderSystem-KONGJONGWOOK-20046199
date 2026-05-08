#include "OrderView.h"
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

Order OrderView::promptOrderInput(std::span<const Sample> samples) const {
    while (true) {
        std::cout << "============================================================\n";
        std::cout << " [2] 시료 주문\n";
        std::cout << "------------------------------------------------------------\n";
        std::cout << " 등록된 시료 목록\n";
        std::cout << " " << std::left
                  << std::setw(9)  << "ID"
                  << std::setw(22) << "시료명"
                  << "재고\n";

        for (const auto& s : samples) {
            std::cout << " " << std::left
                      << std::setw(9)  << s.id
                      << std::setw(22) << s.name
                      << s.stock << " ea\n";
        }
        std::cout << "------------------------------------------------------------\n";

        // 시료 ID 입력
        std::string sampleId;
        while (true) {
            std::cout << " 시료 ID   > ";
            std::getline(std::cin, sampleId);
            bool found = false;
            for (const auto& s : samples) {
                if (s.id == sampleId) { found = true; break; }
            }
            if (found) break;
            std::cout << " [오류] 존재하지 않는 시료 ID입니다. 다시 입력하세요.\n";
        }

        // 고객명 입력
        std::string customer;
        while (true) {
            std::cout << " 고객명    > ";
            std::getline(std::cin, customer);
            if (!customer.empty()) break;
            std::cout << " [오류] 고객명을 입력하세요.\n";
        }

        // 주문 수량 입력
        int quantity = 0;
        while (true) {
            std::cout << " 주문 수량 > ";
            std::string line;
            std::getline(std::cin, line);
            try {
                int val = std::stoi(line);
                if (val >= 1) { quantity = val; break; }
            } catch (...) {}
            std::cout << " [오류] 1 이상의 정수를 입력하세요.\n";
        }

        // 시료명 조회
        std::string sampleName;
        for (const auto& s : samples) {
            if (s.id == sampleId) { sampleName = s.name; break; }
        }

        // 입력 확인 화면
        std::cout << "\n --- 입력 확인 ---\n";
        std::cout << " 시료   : " << sampleName << " (" << sampleId << ")\n";
        std::cout << " 고객   : " << customer << "\n";
        std::cout << " 수량   : " << quantity << " ea\n";
        std::cout << "\n [Y] 예약 접수   [N] 취소\n";
        std::cout << " 선택 > ";

        std::string choice;
        std::getline(std::cin, choice);

        if (!choice.empty() && (choice[0] == 'Y' || choice[0] == 'y')) {
            Order o;
            o.sampleId = sampleId;
            o.customer = customer;
            o.quantity = quantity;
            return o;
        }
        // N이면 처음부터 재입력
    }
}

void OrderView::showOrderResult(const Order& order) const {
    std::cout << "------------------------------------------------------------\n";
    std::cout << " 예약 접수 완료.\n\n";
    std::cout << " 주문번호  : " << order.orderId   << "\n";
    std::cout << " 시료 ID  : " << order.sampleId  << "\n";
    std::cout << " 고객명   : " << order.customer  << "\n";
    std::cout << " 수량     : " << order.quantity  << " ea\n";

    std::string statusStr;
    switch (order.status) {
        case OrderStatus::RESERVED:  statusStr = "RESERVED";  break;
        case OrderStatus::REJECTED:  statusStr = "REJECTED";  break;
        case OrderStatus::PRODUCING: statusStr = "PRODUCING"; break;
        case OrderStatus::CONFIRMED: statusStr = "CONFIRMED"; break;
        case OrderStatus::RELEASED:  statusStr = "RELEASED";  break;
    }
    std::cout << " 현재 상태 : " << statusStr       << "\n";
    std::cout << " 접수 일시 : " << order.createdAt << "\n";
    std::cout << "\n ※ 재고 확인 및 승인은 [3] 주문 승인/거절 메뉴에서 진행하세요.\n";
    std::cout << "------------------------------------------------------------\n";
}

void OrderView::showReservedList(const std::vector<Order>& orders,
                                  std::span<const Sample>   samples) const {
    if (orders.empty()) {
        std::cout << " 대기 중인 주문이 없습니다.\n";
        return;
    }

    std::cout << "------------------------------------------------------------\n";
    std::cout << " " << std::left
              << std::setw(4)  << "No."
              << std::setw(20) << "주문번호"
              << std::setw(20) << "고객"
              << std::setw(22) << "시료명"
              << std::setw(8)  << "수량"
              << "상태\n";
    std::cout << "------------------------------------------------------------\n";

    int no = 1;
    for (const auto& o : orders) {
        std::string sampleName;
        for (const auto& s : samples) {
            if (s.id == o.sampleId) { sampleName = s.name; break; }
        }

        std::string statusStr;
        switch (o.status) {
            case OrderStatus::RESERVED:  statusStr = "RESERVED";  break;
            case OrderStatus::REJECTED:  statusStr = "REJECTED";  break;
            case OrderStatus::PRODUCING: statusStr = "PRODUCING"; break;
            case OrderStatus::CONFIRMED: statusStr = "CONFIRMED"; break;
            case OrderStatus::RELEASED:  statusStr = "RELEASED";  break;
        }

        std::cout << " " << std::left
                  << std::setw(4)  << no++
                  << std::setw(20) << o.orderId
                  << std::setw(20) << o.customer
                  << std::setw(22) << sampleName
                  << std::setw(8)  << (std::to_string(o.quantity) + " ea")
                  << statusStr << "\n";
    }
    std::cout << "------------------------------------------------------------\n";
}

void OrderView::showMessage(const std::string& msg) const {
    std::cout << " " << msg << "\n";
}

void OrderView::showError(const std::string& msg) const {
    std::cout << " [오류] " << msg << "\n";
}

int OrderView::promptMenuChoice() const {
    std::string line;
    std::getline(std::cin, line);
    try {
        return std::stoi(line);
    } catch (...) {
        return -1;
    }
}
