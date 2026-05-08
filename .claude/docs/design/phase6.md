# Phase 6 — 출고 처리 설계

> 참고: [PLAN.md](../PLAN.md) | [PRD.md](../PRD.md)  
> 구현 전 **사용자 검토 후 commit** 규칙 준수

---

## 목표

`CONFIRMED` 상태 주문에 대해 출고를 실행하고 `RELEASED`로 전환한다.  
재고 차감은 Phase 4 승인 시점에 이미 처리되었으므로 **출고 시 재고 추가 차감 없음**.

---

## 1. 생성 및 수정 파일 목록

| 구분 | 파일 | 작업 |
|------|------|------|
| Controller | `Controller/ReleaseController.h` | 수정 (빈 선언 → 실제 선언) |
| Controller | `Controller/ReleaseController.cpp` | 신규 |
| vcxproj | `SampleOrderSystem_project.vcxproj` | ClCompile 1개 추가 |
| vcxproj.filters | `SampleOrderSystem_project.vcxproj.filters` | 소스 파일 필터 1개 추가 |

> 출고 처리 전용 View 클래스는 별도로 만들지 않는다.  
> Phase 3에서 구현된 `OrderView`의 메서드를 재사용하고,  
> 출고 전용 출력은 `ReleaseController` 내부에서 직접 `std::cout`으로 처리한다.

---

## 2. 출고 처리 로직

```
출고 처리 흐름:
1. OrderModel.findByStatus(CONFIRMED) → 출고 가능 주문 목록 취득
2. 목록이 비어있으면 "출고 가능한 주문이 없습니다." 출력 후 반환
3. 번호/주문번호/고객/시료ID/수량 표 형식으로 목록 출력
4. 출고할 번호 입력 (0: 위로)
5. 선택한 주문에 대해:
   - OrderModel.updateStatus(orderId, RELEASED)
6. 출고 완료 결과 출력 (주문번호 / 출고수량 / 처리일시 / 상태 변경)
```

**재고 처리 원칙:**
- 출고 시 재고 차감 없음
- 재고는 Phase 4 승인 시점에 이미 차감됨
- Phase 5 생산 완료 시 `actualQty` 만큼 재고 보충됨

---

## 3. ReleaseController 설계

### Controller/ReleaseController.h

```cpp
#pragma once
#include "../Model/OrderModel.h"
#include "../Model/SampleModel.h"
#include "../View/OrderView.h"

class ReleaseController {
public:
    ReleaseController(OrderModel&  orderModel,
                      SampleModel& sampleModel,
                      OrderView&   view);
    void run();

private:
    void showConfirmedList(const std::vector<Order>& orders) const;
    std::string getCurrentDateTime() const;

    OrderModel&  orderModel_;
    SampleModel& sampleModel_;
    OrderView&   view_;
};
```

### Controller/ReleaseController.cpp — 흐름 명세

**includes 필요:**
```cpp
#define NOMINMAX
#include <Windows.h>
#include "ReleaseController.h"
#include <format>
#include <iostream>
#include <string>
```

**`run()`**:
```cpp
while (true) {
    auto confirmed = orderModel_.findByStatus(OrderStatus::CONFIRMED);

    if (confirmed.empty()) {
        std::cout << "============================================================\n";
        std::cout << " [6] 출고 처리\n";
        std::cout << "------------------------------------------------------------\n";
        std::cout << " 출고 가능한 주문이 없습니다.\n";
        std::cout << "------------------------------------------------------------\n";
        std::cout << "계속하려면 Enter...";
        std::string dummy; std::getline(std::cin, dummy);
        return;
    }

    showConfirmedList(confirmed);

    std::cout << "출고할 번호 (0: 위로) > ";
    std::string line; std::getline(std::cin, line);
    int idx = -1;
    try { idx = std::stoi(line); } catch (...) {}

    if (idx == 0) return;
    if (idx < 1 || idx > static_cast<int>(confirmed.size())) {
        std::cout << "[오류] 올바른 번호를 입력하세요.\n";
        continue;
    }

    const Order& selected = confirmed[idx - 1];

    // 출고 처리 (재고 차감 없음)
    orderModel_.updateStatus(selected.orderId, OrderStatus::RELEASED);

    // 결과 출력
    std::cout << "------------------------------------------------------------\n";
    std::cout << " 출고 처리 완료.\n\n";
    std::cout << " 주문번호  : " << selected.orderId  << "\n";
    std::cout << " 고객명   : " << selected.customer  << "\n";
    std::cout << " 시료 ID  : " << selected.sampleId  << "\n";
    std::cout << " 출고 수량 : " << selected.quantity  << " ea\n";
    std::cout << " 처리 일시 : " << getCurrentDateTime() << "\n";
    std::cout << " 상태 변경 : CONFIRMED → RELEASED\n";
    std::cout << "------------------------------------------------------------\n";
}
```

**`showConfirmedList()`**:
```
============================================================
 [6] 출고 처리
------------------------------------------------------------
 출고 가능 주문  (CONFIRMED)
 번호  주문번호              고객               시료 ID   수량
  [1]  ORD-20260416-0042    SK하이닉스         S-001     150 ea
  [2]  ORD-20260416-0035    DB하이닉스         S-004     400 ea
------------------------------------------------------------
```

**`getCurrentDateTime()`**:
- `GetLocalTime()`으로 현재 시각 취득
- `std::format("{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}", ...)` 형식 반환
- 예: `"2026-04-16 09:34:02"`

---

## 4. 화면 구성 명세

### 출고 가능 목록 화면
```
============================================================
 [6] 출고 처리
------------------------------------------------------------
 출고 가능 주문  (CONFIRMED)
 번호  주문번호              고객               시료 ID   수량
  [1]  ORD-20260416-0042    SK하이닉스         S-001     150 ea
  [2]  ORD-20260416-0035    DB하이닉스         S-004     400 ea
------------------------------------------------------------
출고할 번호 (0: 위로) > 
```

### 출고 완료 결과 화면
```
------------------------------------------------------------
 출고 처리 완료.

 주문번호  : ORD-20260416-0042
 고객명   : SK하이닉스
 시료 ID  : S-001
 출고 수량 : 150 ea
 처리 일시 : 2026-04-16 09:34:02
 상태 변경 : CONFIRMED → RELEASED
------------------------------------------------------------
```

---

## 5. vcxproj 등록

### ClCompile 추가

```xml
<ClCompile Include="Controller\ReleaseController.cpp" />
```

### vcxproj.filters 추가

```xml
<ClCompile Include="Controller\ReleaseController.cpp">
  <Filter>소스 파일</Filter>
</ClCompile>
```

---

## 6. 주의사항

| 항목 | 내용 |
|------|------|
| 재고 차감 금지 | 출고 시 `updateStock()` 호출 없음. Phase 4 승인 시점에 이미 차감됨 |
| 상태 전환만 수행 | `CONFIRMED → RELEASED` 단순 상태 변경 |
| 처리 일시 기록 | 현재 시각을 결과 화면에만 표시 (Order 구조체에 별도 저장 없음) |
| 전용 View 없음 | `ReleaseController` 내부에서 직접 콘솔 출력 처리 |

---

## 7. Phase 6 완료 조건

- [ ] `ReleaseController`: CONFIRMED 목록 표시 → 번호 선택 → RELEASED 전환 구현 완료
- [ ] 출고 시 재고 차감 없음 확인
- [ ] 출고 완료 결과 (주문번호·고객·수량·처리일시·상태) 표시 확인
- [ ] vcxproj / vcxproj.filters 등록 완료
- [ ] 빌드 성공 (경고 없음)
- [ ] 사용자 검토 완료 후 commit
