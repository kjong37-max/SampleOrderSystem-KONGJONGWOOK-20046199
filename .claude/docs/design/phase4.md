# Phase 4 — 주문 승인/거절 설계

> 참고: [PLAN.md](../PLAN.md) | [PRD.md](../PRD.md)  
> 구현 전 **사용자 검토 후 commit** 규칙 준수

---

## 목표

`RESERVED` 상태 주문 목록을 확인하고 승인 또는 거절을 처리한다.  
승인 시 재고 상황에 따라 자동으로 `CONFIRMED` 또는 `PRODUCING`으로 분기한다.  
이 Phase에서 **생산라인(`ProductionLine`) 의 기본 자료구조**도 함께 정의한다.

---

## 1. 생성 및 수정 파일 목록

| 구분 | 파일 | 작업 |
|------|------|------|
| Model | `Model/ProductionLine.h` | 신규 |
| Model | `Model/ProductionLine.cpp` | 신규 |
| Controller | `Controller/ApprovalController.h` | 수정 (빈 선언 → 실제 선언) |
| Controller | `Controller/ApprovalController.cpp` | 신규 |
| vcxproj | `SampleOrderSystem_project.vcxproj` | ClCompile 2개 추가 |
| vcxproj.filters | `SampleOrderSystem_project.vcxproj.filters` | 소스 파일 필터 2개 추가 |

> `ApprovalView`는 별도 View 클래스를 만들지 않고, Phase 3에서 구현된  
> `OrderView::showReservedList()` 를 재사용한다.  
> 승인/거절 관련 프롬프트는 `ApprovalController` 내부에서 직접 처리한다.

---

## 2. 핵심 로직 — 유효 재고 계산

> 이 계산이 Phase 4의 핵심이다. 잘못 구현하면 동일 재고를 중복 승인하는 버그가 발생한다.

```
유효 재고 = 현재 재고(stock)
           - CONFIRMED 상태인 동일 시료 주문 수량 합산
           - PRODUCING 상태인 동일 시료 주문 수량 합산 중 재고로 충당된 부분
```

### 단순화 정의 (구현 채택)

```
유효 재고 = sample.stock
           - Σ(CONFIRMED 상태인 동일 sampleId 주문의 quantity)
```

- `PRODUCING` 상태 주문은 재고가 이미 부족하여 생산 중이므로, 재고에서 차감할 몫이 없다.
- `RELEASED` 상태 주문은 출고 시점에 재고를 차감했으므로 고려 불필요.
- `RESERVED` 상태 주문은 아직 승인되지 않았으므로 고려 불필요.

### 승인 분기 로직

```
유효 재고 = sample.stock - Σ(CONFIRMED 주문의 quantity, sampleId 동일)

if (유효 재고 >= 주문 수량)
    // 재고 충분
    SampleModel.updateStock(sampleId, -주문수량)   // 재고 차감
    OrderModel.updateStatus(orderId, CONFIRMED)

else
    // 재고 부족
    부족분   = 주문 수량 - max(유효 재고, 0)
    actualQty = ceil(부족분 / (yield × 0.9))
    totalMin  = avgProdTime × actualQty

    ProductionJob job { orderId, sampleId, 부족분, actualQty, totalMin }
    ProductionLine.enqueue(job)
    OrderModel.updateStatus(orderId, PRODUCING)

    // 유효 재고만큼은 즉시 차감
    if (유효 재고 > 0)
        SampleModel.updateStock(sampleId, -유효 재고)
```

---

## 3. ProductionLine 설계

### Model/ProductionLine.h

```cpp
#pragma once
#include "ProductionJob.h"
#include <optional>
#include <queue>
#include <vector>

class ProductionLine {
public:
    // 생산 큐에 작업 추가 (FIFO)
    void enqueue(const ProductionJob& job);

    // 현재 처리 중인 작업 (없으면 nullopt)
    std::optional<ProductionJob> currentJob() const;

    // 대기 중인 작업 목록 (FIFO 순, 복사본)
    std::vector<ProductionJob> pendingJobs() const;

    // 대기 큐 건수
    int pendingCount() const;

    // 현재 작업 완료 처리 → 다음 대기 작업으로 이동
    // 완료된 job 반환 (없으면 nullopt)
    std::optional<ProductionJob> completeCurrentJob();

    // 생산라인이 실행 중인지 (current_ 존재 여부)
    bool isRunning() const;

private:
    std::optional<ProductionJob> current_;
    std::queue<ProductionJob>    queue_;
};
```

### Model/ProductionLine.cpp — 구현 명세

**`enqueue()`**
```
if (!current_.has_value())
    current_ = job;   // 즉시 처리 시작
else
    queue_.push(job); // 대기 큐에 추가
```

**`currentJob()`**: `current_` 반환

**`pendingJobs()`**
```
queue를 복사하여 vector로 변환 후 반환
(std::queue는 iterator 미지원 → 임시 큐를 pop하며 vector에 추가)
```

**`pendingCount()`**: `static_cast<int>(queue_.size())`

**`completeCurrentJob()`**
```
if (!current_.has_value()) return std::nullopt;
ProductionJob done = *current_;
if (!queue_.empty()) {
    current_ = queue_.front();
    queue_.pop();
} else {
    current_ = std::nullopt;
}
return done;
```

**`isRunning()`**: `current_.has_value()`

---

## 4. ApprovalController 설계

### Controller/ApprovalController.h

```cpp
#pragma once
#include "../Model/OrderModel.h"
#include "../Model/SampleModel.h"
#include "../Model/ProductionLine.h"
#include "../View/OrderView.h"
#include <cmath>

class ApprovalController {
public:
    ApprovalController(OrderModel&    orderModel,
                       SampleModel&   sampleModel,
                       ProductionLine& productionLine,
                       OrderView&     view);
    void run();

private:
    void handleApproval();

    // 유효 재고 계산
    int calcEffectiveStock(const std::string& sampleId) const;

    OrderModel&    orderModel_;
    SampleModel&   sampleModel_;
    ProductionLine& productionLine_;
    OrderView&     view_;
};
```

### Controller/ApprovalController.cpp — 흐름 명세

**`run()`**
```cpp
while (true) {
    auto reserved = orderModel_.findByStatus(OrderStatus::RESERVED);
    view_.showReservedList(reserved, sampleModel_.all());

    if (reserved.empty()) return;

    std::cout << "처리할 번호 (0: 위로) > ";
    // 번호 입력 → reserved[idx-1] 선택
    // 0 입력 시 return

    std::cout << "[1] 승인   [2] 거절   [0] 취소 > ";
    // 1 → handleApproval(selected)
    // 2 → orderModel_.updateStatus(orderId, REJECTED)
    //      view_.showMessage("거절 처리되었습니다.")
    // 0 → continue (목록으로 돌아감)
}
```

**`handleApproval(const Order& order)`**
```
Sample* s = sampleModel_.findById(order.sampleId)

int effectiveStock = calcEffectiveStock(order.sampleId);

// 재고 정보 표시
cout << "시료     : " << s->name
cout << "주문 수량 : " << order.quantity << " ea"
cout << "현재 재고 : " << s->stock << " ea"
cout << "유효 재고 : " << effectiveStock << " ea"
     << " (CONFIRMED 대기분 차감 후)"

if (effectiveStock >= order.quantity) {
    // 재고 충분
    sampleModel_.updateStock(order.sampleId, -order.quantity);
    orderModel_.updateStatus(order.orderId, OrderStatus::CONFIRMED);
    cout << "[승인 완료] RESERVED → CONFIRMED"
} else {
    // 재고 부족
    int shortage   = order.quantity - std::max(effectiveStock, 0);
    int actualQty  = static_cast<int>(
                        std::ceil(shortage / (s->yield * 0.9)));
    double totalMin = s->avgProdTime * actualQty;

    cout << "재고 부족. 부족분: " << shortage << " ea"
    cout << "실 생산량: " << actualQty << " ea"
         << "  /  총 생산시간: " << totalMin << " min"
    cout << "[Y] 승인(생산 등록)   [N] 거절 > ";

    if (Y) {
        if (effectiveStock > 0)
            sampleModel_.updateStock(order.sampleId, -effectiveStock);
        ProductionJob job { order.orderId, order.sampleId,
                            shortage, actualQty, totalMin };
        productionLine_.enqueue(job);
        orderModel_.updateStatus(order.orderId, OrderStatus::PRODUCING);
        cout << "[승인 완료] RESERVED → PRODUCING (생산라인 등록)";
    } else {
        orderModel_.updateStatus(order.orderId, OrderStatus::REJECTED);
        cout << "[거절 처리] RESERVED → REJECTED";
    }
}
```

**`calcEffectiveStock()`**
```cpp
int effectiveStock = 0;
if (auto s = sampleModel_.findById(sampleId))
    effectiveStock = s->stock;

for (const auto& o : orderModel_.findByStatus(OrderStatus::CONFIRMED))
    if (o.sampleId == sampleId)
        effectiveStock -= o.quantity;

return effectiveStock;
```

---

## 5. vcxproj 등록

### ClCompile 추가

```xml
<ClCompile Include="Model\ProductionLine.cpp" />
<ClCompile Include="Controller\ApprovalController.cpp" />
```

### vcxproj.filters 추가

```xml
<ClCompile Include="Model\ProductionLine.cpp">
  <Filter>소스 파일</Filter>
</ClCompile>
<ClCompile Include="Controller\ApprovalController.cpp">
  <Filter>소스 파일</Filter>
</ClCompile>
```

---

## 6. 주의사항

| 항목 | 내용 |
|------|------|
| 유효 재고 하한 | `max(effectiveStock, 0)` — 음수가 되지 않도록 보정 |
| 생산량 보정 상수 | `0.9` 는 PRD 고정 상수, 변수화 금지 |
| 재고 차감 시점 | 승인 시점에만 차감 (출고 시 추가 차감 없음 — Phase 6 주의) |
| 거절 즉시 처리 | `REJECTED` 전환 후 별도 처리 없음 |
| `ceil` 사용 | `<cmath>` 의 `std::ceil` 사용 |

---

## 7. Phase 4 완료 조건

- [ ] `ProductionLine`: enqueue / currentJob / pendingJobs / completeCurrentJob / isRunning 구현 완료
- [ ] `ApprovalController`: 유효 재고 계산 및 승인/거절 분기 구현 완료
- [ ] 재고 충분 → `CONFIRMED` 전환 + 재고 차감 확인
- [ ] 재고 부족 → `PRODUCING` 전환 + `ProductionLine` 큐 등록 확인
- [ ] 거절 → `REJECTED` 즉시 전환 확인
- [ ] vcxproj / vcxproj.filters 등록 완료
- [ ] 빌드 성공 (경고 없음)
- [ ] 사용자 검토 완료 후 commit
