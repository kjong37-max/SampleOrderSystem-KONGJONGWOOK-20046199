#pragma once
#define NOMINMAX
#include <Windows.h>
#include "../Model/Constants.h"
#include "../Model/Sample.h"
#include "../Model/Order.h"
#include <string>
#include <vector>

enum class StockStatus { SURPLUS, SHORT, DEPLETED };

class MonitorView {
public:
    MonitorView();
    virtual ~MonitorView();

    virtual void showMenu() const;
    virtual void showOrderStats(int reserved, int confirmed,
                                int producing, int released) const;
    virtual void showStockStats(const std::vector<Sample>& samples,
                                const std::vector<StockStatus>& statuses) const;
    virtual void showMessage(const std::string& msg) const;
    virtual int  promptMenuChoice() const;

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
    void printBar(int stock, int maxStock,
                  int width = SemiConst::BAR_WIDTH) const;
};
