# Step 8 — 유효 재고 계산 단일화 설계

> 참고: [PLAN.md](../PLAN.md) | 브랜치: `refactoring`  
> 구현 전 **사용자 검토 후 commit** 규칙 준수

---

## 목표

재고 계산 로직이 두 Controller에 분산되어 있는 것을 `SampleModel`로 통합한다.  
**동작 변경 없음. 기존 테스트 모두 계속 통과해야 한다.**

---

## 현재 분산 위치

| 파일 | 함수 | 로직 |
|------|------|------|
| `Controller/ApprovalController.cpp:105-114` | `calcEffectiveStock()` | `stock - Σ(CONFIRMED 수량)` |
| `Controller/MonitorController.cpp:39-48` | `calcStockStatus()` | stock==0→고갈, `Σ(CONFIRMED+RESERVED) > stock`→부족, 그 외→여유 |

두 함수는 서로 다른 역할이지만 모두 재고와 주문 수요를 기반으로 계산한다.  
이 로직은 `OrderModel`을 참조해야 하므로 `SampleModel`에 `OrderModel`을 인자로 받는 메서드로 추가한다.

---

## StockStatus enum 이전

현재 `StockStatus`가 `View/MonitorView.h`에 정의되어 있어 Model 계층에서 사용할 수 없다.  
`Model/Sample.h`로 이전한다.

---

## 생성/수정 파일 목록

| 파일 | 작업 |
|------|------|
| `Model/Sample.h` | `StockStatus` enum 추가 |
| `View/MonitorView.h` | `StockStatus` 정의 제거 → `Model/Sample.h` include |
| `Controller/MonitorController.h` | `StockStatus` include 경로 변경 |
| `Model/SampleModel.h` | `effectiveStock()`, `stockStatus()` 메서드 추가 |
| `Model/SampleModel.cpp` | 두 메서드 구현 |
| `Controller/ApprovalController.h` | `calcEffectiveStock()` private 선언 제거 |
| `Controller/ApprovalController.cpp` | `calcEffectiveStock()` 구현 제거 → `sampleModel_.effectiveStock()` 사용 |
| `Controller/MonitorController.h` | `calcStockStatus()` private 선언 제거 |
| `Controller/MonitorController.cpp` | `calcStockStatus()` 구현 제거 → `sampleModel_.stockStatus()` 사용 |

---

## Model/Sample.h 추가

```cpp
// Order.h include 없이 사용하기 위해 OrderModel 전방선언 불가
// → 메서드 인자로 OrderModel 참조를 받는 방식 채택 (SampleModel.h에서 처리)

// StockStatus enum을 Sample.h로 이전
enum class StockStatus { SURPLUS, SHORT, DEPLETED };
```

## Model/SampleModel.h 추가

```cpp
// 전방선언 (순환 include 방지)
class OrderModel;

class SampleModel {
public:
    // ... 기존 메서드 ...

    // 유효 재고 = stock - Σ(CONFIRMED 주문 수량)
    int effectiveStock(const std::string& sampleId,
                       const OrderModel&  orderModel) const;

    // 재고 상태 판정
    // 고갈: stock == 0
    // 부족: Σ(CONFIRMED + RESERVED) > stock
    // 여유: 그 외
    StockStatus stockStatus(const std::string& sampleId,
                            const OrderModel&  orderModel) const;
};
```

## Model/SampleModel.cpp 추가

```cpp
#include "../Model/OrderModel.h"  // effectiveStock/stockStatus 구현에 필요

int SampleModel::effectiveStock(const std::string& sampleId,
                                 const OrderModel&  orderModel) const {
    auto s = findById(sampleId);
    int stock = s ? s->stock : 0;
    for (const auto& o : orderModel.findByStatus(OrderStatus::CONFIRMED))
        if (o.sampleId == sampleId) stock -= o.quantity;
    return stock;
}

StockStatus SampleModel::stockStatus(const std::string& sampleId,
                                      const OrderModel&  orderModel) const {
    auto s = findById(sampleId);
    if (!s || s->stock == 0) return StockStatus::DEPLETED;

    int demand = 0;
    for (const auto& o : orderModel.findByStatus(OrderStatus::CONFIRMED))
        if (o.sampleId == sampleId) demand += o.quantity;
    for (const auto& o : orderModel.findByStatus(OrderStatus::RESERVED))
        if (o.sampleId == sampleId) demand += o.quantity;

    return (demand > s->stock) ? StockStatus::SHORT : StockStatus::SURPLUS;
}
```

---

## Controller 수정 명세

### ApprovalController.h

private 섹션에서 삭제:
```cpp
int calcEffectiveStock(const std::string& sampleId) const;  // ← 삭제
```

### ApprovalController.cpp

- `calcEffectiveStock()` 구현 전체 삭제
- `handleApproval()` 내 `calcEffectiveStock(order.sampleId)` → `sampleModel_.effectiveStock(order.sampleId, orderModel_)`

### MonitorController.h

private 섹션에서 삭제:
```cpp
StockStatus calcStockStatus(const Sample& sample) const;  // ← 삭제
```

### MonitorController.cpp

- `calcStockStatus()` 구현 전체 삭제
- `handleStockStats()` 내:
  ```cpp
  // 변경 전
  statuses.push_back(calcStockStatus(s));

  // 변경 후
  statuses.push_back(sampleModel_.stockStatus(s.id, orderModel_));
  ```

---

## 순환 include 방지

`SampleModel.h`에서 `OrderModel` 전방선언, `SampleModel.cpp`에서만 full include:

```cpp
// SampleModel.h
class OrderModel;  // 전방선언

// SampleModel.cpp
#include "../Model/OrderModel.h"  // full include
```

---

## SampleModelTest 보완 (Step 10에서 추가)

`effectiveStock()` / `stockStatus()` 에 대한 테스트는 Step 10에서 추가한다.

---

## Step 8 완료 조건

- [ ] `StockStatus` → `Model/Sample.h`로 이전
- [ ] `SampleModel`에 `effectiveStock()` / `stockStatus()` 추가
- [ ] `ApprovalController` / `MonitorController` 에서 중복 제거
- [ ] `MonitorView.h`에서 `StockStatus` 정의 제거
- [ ] 빌드 성공 (경고 없음)
- [ ] 전체 테스트 통과
- [ ] 사용자 검토 완료 후 commit
