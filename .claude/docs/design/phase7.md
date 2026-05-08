# Phase 7 — 모니터링 설계

> 참고: [PLAN.md](../PLAN.md) | [PRD.md](../PRD.md)  
> 구현 전 **사용자 검토 후 commit** 규칙 준수

---

## 목표

담당자가 시스템 전체 상태를 한눈에 파악할 수 있도록 모니터링 기능을 구현한다.  
DataMonitor 참고 저장소의 컬러 출력·바 그래프 패턴을 참고하여 가독성을 높인다.

---

## 1. 생성 및 수정 파일 목록

| 구분 | 파일 | 작업 |
|------|------|------|
| View | `View/MonitorView.h` | 수정 (빈 선언 → 실제 선언) |
| View | `View/MonitorView.cpp` | 신규 |
| Controller | `Controller/MonitorController.h` | 수정 (빈 선언 → 실제 선언) |
| Controller | `Controller/MonitorController.cpp` | 신규 |
| vcxproj | `SampleOrderSystem_project.vcxproj` | ClCompile 2개 추가 |
| vcxproj.filters | `SampleOrderSystem_project.vcxproj.filters` | 소스 파일 필터 2개 추가 |

---

## 2. 주문량 확인 — 상태별 집계

**집계 대상 상태:** `RESERVED` / `CONFIRMED` / `PRODUCING` / `RELEASED`  
**제외:** `REJECTED` (유효 주문이 아니므로 모니터링 무시)

```cpp
// 집계 방법
int reserved  = orderModel_.findByStatus(OrderStatus::RESERVED).size();
int confirmed = orderModel_.findByStatus(OrderStatus::CONFIRMED).size();
int producing = orderModel_.findByStatus(OrderStatus::PRODUCING).size();
int released  = orderModel_.findByStatus(OrderStatus::RELEASED).size();
```

---

## 3. 재고량 확인 — 재고 상태 판정

| 상태 | 판정 조건 |
|------|-----------|
| 고갈 | `stock == 0` |
| 부족 | `stock > 0` && CONFIRMED + RESERVED 주문 수량 합 > stock |
| 여유 | 그 외 (처리 가능한 재고 보유) |

```cpp
enum class StockStatus { SURPLUS, SHORT, DEPLETED };

StockStatus calcStockStatus(const Sample& s,
                             const OrderModel& orderModel) {
    if (s.stock == 0) return StockStatus::DEPLETED;

    int demand = 0;
    for (const auto& o : orderModel.findByStatus(OrderStatus::CONFIRMED))
        if (o.sampleId == s.id) demand += o.quantity;
    for (const auto& o : orderModel.findByStatus(OrderStatus::RESERVED))
        if (o.sampleId == s.id) demand += o.quantity;

    return (demand > s.stock) ? StockStatus::SHORT : StockStatus::SURPLUS;
}
```

---

## 4. MonitorView 설계

### View/MonitorView.h

```cpp
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

    // 서브 메뉴 출력
    void showMenu() const;

    // 상태별 주문량 표시
    void showOrderStats(int reserved, int confirmed,
                        int producing, int released) const;

    // 재고량 표시 (시료별 재고 + 상태 + 바 그래프)
    void showStockStats(const std::vector<Sample>& samples,
                        const std::vector<StockStatus>& statuses) const;

    // 공통
    void showMessage(const std::string& msg) const;
    int  promptMenuChoice()                  const;

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
```

### View/MonitorView.cpp — 화면 구성 명세

**생성자/소멸자:**
```cpp
MonitorView::MonitorView()
    : hOut_(GetStdHandle(STD_OUTPUT_HANDLE)) {
    CONSOLE_SCREEN_BUFFER_INFO info{};
    GetConsoleScreenBufferInfo(hOut_, &info);
    defaultAttr_ = info.wAttributes;
}
MonitorView::~MonitorView() { resetColor(); }
```

**`setColor()` / `resetColor()`:**
```cpp
void MonitorView::setColor(C c) const {
    SetConsoleTextAttribute(hOut_, static_cast<WORD>(c));
}
void MonitorView::resetColor() const {
    SetConsoleTextAttribute(hOut_, defaultAttr_);
}
```

**`showMenu()`** 출력:
```
============================================================
 [4] 모니터링
------------------------------------------------------------
 [1] 주문량 확인   [2] 재고량 확인   [0] 위로
선택 > 
```

**`showOrderStats()`** 출력:
```
============================================================
 주문 현황  (상태별)
------------------------------------------------------------
  RESERVED   :   3건      ← 흰색
  CONFIRMED  :   8건      ← 초록색
  PRODUCING  :   3건      ← 노란색
  RELEASED   :  18건      ← 회색
------------------------------------------------------------
  유효 주문 합계: 32건  (REJECTED 제외)
============================================================
```
- 상태명에 Win32 콘솔 컬러 적용:
  - `RESERVED` → White, `CONFIRMED` → Green, `PRODUCING` → Yellow, `RELEASED` → Gray

**`showStockStats()`** 출력:
```
============================================================
 재고 현황  (시료별)
------------------------------------------------------------
  시료명                  재고      상태   잔여율
  실리콘 웨이퍼-8인치    480 ea   여유   ████████████████░░░░  80%
  GaN 에피택셀-4인치     220 ea   여유   █████████░░░░░░░░░░░  44%
  SiC 파워기판-6인치      30 ea   부족   █░░░░░░░░░░░░░░░░░░░   6%
  산화막 웨이퍼-SiO2       0 ea   고갈   ░░░░░░░░░░░░░░░░░░░░   0%
------------------------------------------------------------
============================================================
```
- 상태 컬러:
  - `여유` → Green, `부족` → Yellow, `고갈` → Red
- 바 그래프 (`printBar()`):
  - 전체 바 길이 20칸
  - 채워진 칸: `█` (U+2588), 빈 칸: `░` (U+2591)
  - `filledCount = (stock * 20) / maxStock`
  - `maxStock`: 해당 시료의 최대 재고 기준 (전체 시료 중 최대 stock 값 사용)

**`printBar()`:**
```cpp
void MonitorView::printBar(int stock, int maxStock, int width) const {
    int filled = (maxStock > 0) ? (stock * width / maxStock) : 0;
    setColor(C::Green);
    for (int i = 0; i < filled; ++i) std::cout << "█";
    setColor(C::Gray);
    for (int i = filled; i < width; ++i) std::cout << "░";
    resetColor();
}
```

---

## 5. MonitorController 설계

### Controller/MonitorController.h

```cpp
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
```

### Controller/MonitorController.cpp — 흐름 명세

**`run()`:**
```cpp
while (true) {
    view_.showMenu();
    switch (view_.promptMenuChoice()) {
    case 1: handleOrderStats(); break;
    case 2: handleStockStats(); break;
    case 0: return;
    default: view_.showMessage("[오류] 잘못된 선택입니다.");
    }
}
```

**`handleOrderStats()`:**
```cpp
int reserved  = static_cast<int>(orderModel_.findByStatus(OrderStatus::RESERVED).size());
int confirmed = static_cast<int>(orderModel_.findByStatus(OrderStatus::CONFIRMED).size());
int producing = static_cast<int>(orderModel_.findByStatus(OrderStatus::PRODUCING).size());
int released  = static_cast<int>(orderModel_.findByStatus(OrderStatus::RELEASED).size());
view_.showOrderStats(reserved, confirmed, producing, released);
```

**`handleStockStats()`:**
```cpp
auto allSamples = sampleModel_.all();
std::vector<Sample> samples(allSamples.begin(), allSamples.end());
std::vector<StockStatus> statuses;
for (const auto& s : samples)
    statuses.push_back(calcStockStatus(s));
view_.showStockStats(samples, statuses);
```

**`calcStockStatus()`:**
```cpp
StockStatus MonitorController::calcStockStatus(const Sample& s) const {
    if (s.stock == 0) return StockStatus::DEPLETED;

    int demand = 0;
    for (const auto& o : orderModel_.findByStatus(OrderStatus::CONFIRMED))
        if (o.sampleId == s.id) demand += o.quantity;
    for (const auto& o : orderModel_.findByStatus(OrderStatus::RESERVED))
        if (o.sampleId == s.id) demand += o.quantity;

    return (demand > s.stock) ? StockStatus::SHORT : StockStatus::SURPLUS;
}
```

---

## 6. vcxproj 등록

### ClCompile 추가

```xml
<ClCompile Include="View\MonitorView.cpp" />
<ClCompile Include="Controller\MonitorController.cpp" />
```

### vcxproj.filters 추가

```xml
<ClCompile Include="View\MonitorView.cpp">
  <Filter>소스 파일</Filter>
</ClCompile>
<ClCompile Include="Controller\MonitorController.cpp">
  <Filter>소스 파일</Filter>
</ClCompile>
```

---

## 7. 주의사항

| 항목 | 내용 |
|------|------|
| REJECTED 제외 | 주문량 집계 및 재고 상태 판정 모두 REJECTED 무시 |
| stdout 모드 | Win32 컬러 API 사용 시 `_setmode` 불필요 — `SetConsoleTextAttribute` 직접 사용 |
| 바 그래프 기준 | `maxStock`은 전체 시료 중 최대 `stock` 값 사용 (0이면 나누기 방지) |
| StockStatus 위치 | `MonitorView.h`에 정의. `MonitorController.h`에서 include하여 사용 |

---

## 8. Phase 7 완료 조건

- [ ] `MonitorView`: 주문량·재고량 화면 구현, Win32 컬러 출력, 바 그래프 구현 완료
- [ ] `MonitorController`: handleOrderStats / handleStockStats 구현 완료
- [ ] 재고 상태 판정 (여유/부족/고갈) 정확성 확인
- [ ] REJECTED 주문 집계 제외 확인
- [ ] vcxproj / vcxproj.filters 등록 완료
- [ ] 빌드 성공 (경고 없음)
- [ ] 사용자 검토 완료 후 commit
