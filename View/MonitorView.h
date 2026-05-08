#pragma once
#define NOMINMAX
#include <Windows.h>
#include "../Model/Sample.h"
#include "../Model/Order.h"
#include <string>
#include <vector>

enum class StockStatus { SURPLUS, SHORT, DEPLETED };

class MonitorView {
public:
    MonitorView();
    ~MonitorView();

    void showMenu() const;
    void showOrderStats(int reserved, int confirmed,
                        int producing, int released) const;
    void showStockStats(const std::vector<Sample>& samples,
                        const std::vector<StockStatus>& statuses) const;
    void showMessage(const std::string& msg) const;
    int  promptMenuChoice() const;

private:
    HANDLE hOut_;
    WORD   defaultAttr_;

    enum class C : WORD {
        White  = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
        Gray   = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
        Green  = FOREGROUND_GREEN | FOREGROUND_INTENSITY,
        Yellow = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
        Red    = FOREGROUND_RED | FOREGROUND_INTENSITY,
        Cyan   = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
    };

    void setColor(C c) const;
    void resetColor()  const;
    void printBar(int stock, int maxStock, int width = 20) const;
};
