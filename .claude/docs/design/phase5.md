# Phase 5 — 생산 라인 설계

> 참고: [PLAN.md](../PLAN.md) | [PRD.md](../PRD.md)  
> 구현 전 **사용자 검토 후 commit** 규칙 준수

---

## 목표

생산라인의 현재 처리 중인 작업과 FIFO 대기 목록을 조회하고,  
생산 완료 처리(PRODUCING → CONFIRMED + 재고 보충)를 수행한다.

`ProductionLine` 자료구조는 Phase 4에서 이미 구현되었으므로,  
이 Phase에서는 **ProductionView** 화면과 **ProductionController** 로직을 추가한다.

---

## 1. 생성 및 수정 파일 목록

| 구분 | 파일 | 작업 |
|------|------|------|
| View | `View/ProductionView.h` | 수정 (빈 선언 → 실제 선언) |
| View | `View/ProductionView.cpp` | 신규 |
| Controller | `Controller/ProductionController.h` | 신규 |
| Controller | `Controller/ProductionController.cpp` | 신규 |
| vcxproj | `SampleOrderSystem_project.vcxproj` | ClCompile 2개 추가 |
| vcxproj.filters | `SampleOrderSystem_project.vcxproj.filters` | 소스 파일 필터 2개 추가 |

---

## 2. 생산 완료 처리 로직

PRD 명세: **생산 완료 시 주문 상태 `PRODUCING` → `CONFIRMED` 변경**

콘솔 기반 시스템이므로 실제 시간 경과 대신 **"생산 완료 처리" 메뉴**로 수동 트리거한다.  
완료 시 재고 보충과 상태 전환을 수행한다.

```
생산 완료 처리 흐름:
1. ProductionLine.currentJob() 확인
2. 완료할 job의 actualQty만큼 재고 보충
   SampleModel.updateStock(job.sampleId, +job.actualQty)
3. 해당 주문 상태 변경
   OrderModel.updateStatus(job.orderId, CONFIRMED)
4. ProductionLine.completeCurrentJob() 호출 → 다음 대기 작업으로 이동
5. 결과 출력
```

---

## 3. ProductionView 설계

### View/ProductionView.h

```cpp
#pragma once
#include "../Model/ProductionJob.h"
#include <optional>
#include <string>
#include <vector>

class ProductionView {
public:
    // 서브 메뉴 출력
    void showMenu() const;

    // 생산라인 전체 현황 출력
    void showProductionStatus(
        const std::optional<ProductionJob>& current,
        const std::vector<ProductionJob>&   pending) const;

    // 생산 완료 처리 결과 출력
    void showCompleteResult(const ProductionJob& job) const;

    // 공통
    void showMessage(const std::string& msg) const;
    void showError(const std::string& msg)   const;
    int  promptMenuChoice()                  const;
};
```

### View/ProductionView.cpp — 화면 구성 명세

**`showMenu()`** 출력 형식:
```
============================================================
 [5] 생산라인 조회
------------------------------------------------------------
 [1] 생산 현황 조회   [2] 생산 완료 처리   [0] 위로
선택 > 
```

**`showProductionStatus()`** 출력 형식:
```
============================================================
 생산라인 현황   2026-04-16 09:32:15
------------------------------------------------------------
 ■ 현재 처리 중
   주문번호  : ORD-20260416-0038
   시료 ID  : S-003
   부족분   : 170 ea
   실 생산량 : 206 ea   (ceil(170 / (0.92 × 0.9)))
   총 생산시간: 164.8 min
------------------------------------------------------------
 ■ 대기 중인 주문  (FIFO 순)
 순서  주문번호           시료 ID   부족분   실생산량   총 생산시간
  [1]  ORD-20260416-0040  S-005    150 ea   190 ea    114.0 min
  [2]  ORD-20260416-0043  S-003    170 ea   206 ea    164.8 min
  [3]  ORD-20260416-0044  S-002     80 ea   114 ea     34.2 min
------------------------------------------------------------
 대기 건수: 3건
============================================================
```
- `current`가 nullopt이면 "현재 처리 중인 작업이 없습니다." 출력
- `pending`이 비어있으면 "대기 중인 작업이 없습니다." 출력

**`showCompleteResult()`** 출력 형식:
```
------------------------------------------------------------
 생산 완료 처리되었습니다.

 주문번호  : ORD-20260416-0038
 시료 ID  : S-003
 생산 수량 : 206 ea (재고 보충)
 상태 변경 : PRODUCING → CONFIRMED
------------------------------------------------------------
```

---

## 4. ProductionController 설계

### Controller/ProductionController.h (신규)

```cpp
#pragma once
#include "../Model/OrderModel.h"
#include "../Model/SampleModel.h"
#include "../Model/ProductionLine.h"
#include "../View/ProductionView.h"

class ProductionController {
public:
    ProductionController(OrderModel&     orderModel,
                         SampleModel&    sampleModel,
                         ProductionLine& productionLine,
                         ProductionView& view);
    void run();

private:
    void handleStatus();
    void handleComplete();

    OrderModel&     orderModel_;
    SampleModel&    sampleModel_;
    ProductionLine& productionLine_;
    ProductionView& view_;
};
```

### Controller/ProductionController.cpp — 흐름 명세

**`run()`**:
```cpp
while (true) {
    view_.showMenu();
    switch (view_.promptMenuChoice()) {
    case 1: handleStatus();   break;
    case 2: handleComplete(); break;
    case 0: return;
    default: view_.showError("잘못된 선택입니다.");
    }
}
```

**`handleStatus()`**:
```cpp
view_.showProductionStatus(
    productionLine_.currentJob(),
    productionLine_.pendingJobs()
);
```

**`handleComplete()`**:
```cpp
auto current = productionLine_.currentJob();
if (!current) {
    view_.showMessage("현재 처리 중인 생산 작업이 없습니다.");
    return;
}

// 현재 작업 정보 표시
view_.showProductionStatus(current, productionLine_.pendingJobs());

std::cout << "현재 작업을 완료 처리하시겠습니까? (Y/N) > ";
std::string ans; std::getline(std::cin, ans);
if (ans.empty() || (ans[0] != 'Y' && ans[0] != 'y')) return;

// 재고 보충
sampleModel_.updateStock(current->sampleId, +current->actualQty);

// 주문 상태 → CONFIRMED
orderModel_.updateStatus(current->orderId, OrderStatus::CONFIRMED);

// 다음 작업으로 이동
auto done = productionLine_.completeCurrentJob();

if (done) view_.showCompleteResult(*done);
```

---

## 5. vcxproj 등록

### ClCompile 추가

```xml
<ClCompile Include="View\ProductionView.cpp" />
<ClCompile Include="Controller\ProductionController.cpp" />
```

### vcxproj.filters 추가

```xml
<ClCompile Include="View\ProductionView.cpp">
  <Filter>소스 파일</Filter>
</ClCompile>
<ClCompile Include="Controller\ProductionController.cpp">
  <Filter>소스 파일</Filter>
</ClCompile>
```

---

## 6. 생산량 계산 공식 재확인

```
부족분      = 주문 수량 - 유효 재고  (Phase 4 승인 시 확정)
실 생산량   = ceil(부족분 / (수율 × 0.9))   ← 0.9: 고정 보정 상수
총 생산시간 = avgProdTime × 실 생산량
```

- 이 계산은 Phase 4 `handleApproval()`에서 이미 수행하여 `ProductionJob`에 저장됨
- Phase 5에서는 이 값을 읽어 화면에 표시하기만 함

---

## 7. 주의사항

| 항목 | 내용 |
|------|------|
| 재고 보충 시점 | `completeCurrentJob()` 호출 전에 `updateStock(+actualQty)` 수행 |
| 상태 전환 시점 | 재고 보충과 동시에 `PRODUCING → CONFIRMED` 전환 |
| 출고 시 재고 차감 없음 | Phase 6에서 CONFIRMED → RELEASED 전환만 수행, 재고 미차감 |
| 재고 차감 타이밍 정리 | 승인(Phase 4): 유효재고 차감 / 생산완료(Phase 5): actualQty 보충 / 출고(Phase 6): 차감 없음 |

---

## 8. Phase 5 완료 조건

- [ ] `ProductionView`: 생산 현황 조회 화면 및 완료 결과 출력 구현 완료
- [ ] `ProductionController`: handleStatus / handleComplete 구현 완료
- [ ] 생산 완료 시 재고 보충 (`updateStock(+actualQty)`) 확인
- [ ] 생산 완료 시 `PRODUCING → CONFIRMED` 상태 전환 확인
- [ ] 완료 후 다음 대기 작업이 current로 승격되는지 확인
- [ ] vcxproj / vcxproj.filters 등록 완료
- [ ] 빌드 성공 (경고 없음)
- [ ] 사용자 검토 완료 후 commit
