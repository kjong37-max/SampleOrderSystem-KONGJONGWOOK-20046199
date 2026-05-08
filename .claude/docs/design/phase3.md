# Phase 3 — 주문 접수 설계

> 참고: [PLAN.md](../PLAN.md) | [PRD.md](../PRD.md)  
> 구현 전 **사용자 검토 후 commit** 규칙 준수

---

## 목표

고객 주문을 접수하여 `RESERVED` 상태의 주문을 생성한다.  
시료 ID 유효성 확인 후 주문번호를 자동 채번하고, 재고 확인은 하지 않는다(Phase 4에서 처리).

---

## 1. 생성 및 수정 파일 목록

| 구분 | 파일 | 작업 |
|------|------|------|
| Model | `Model/OrderModel.h` | 신규 |
| Model | `Model/OrderModel.cpp` | 신규 |
| View | `View/OrderView.h` | 수정 (빈 선언 → 실제 선언) |
| View | `View/OrderView.cpp` | 신규 |
| Controller | `Controller/OrderController.h` | 수정 (빈 선언 → 실제 선언) |
| Controller | `Controller/OrderController.cpp` | 신규 |
| vcxproj | `SampleOrderSystem_project.vcxproj` | ClCompile 3개 추가 |
| vcxproj.filters | `SampleOrderSystem_project.vcxproj.filters` | 소스 파일 필터 3개 추가 |

---

## 2. OrderModel 설계

### Model/OrderModel.h

```cpp
#pragma once
#include "Order.h"
#include <optional>
#include <span>
#include <string>
#include <vector>

class OrderModel {
public:
    // 주문 접수: 주문번호 자동 채번 후 RESERVED 상태로 등록. 생성된 주문 반환
    Order reserve(const std::string& sampleId,
                  const std::string& customer,
                  int                quantity);

    // 주문번호로 단건 조회
    std::optional<Order> findById(const std::string& orderId) const;

    // 상태별 주문 목록 조회
    std::vector<Order> findByStatus(OrderStatus status) const;

    // 주문 상태 변경
    bool updateStatus(const std::string& orderId, OrderStatus newStatus);

    // 전체 주문 목록
    std::span<const Order> all() const;

    // 전체 주문 수
    int totalCount() const;

private:
    std::vector<Order> orders_;

    // 주문번호 채번
    std::string generateOrderId();

    // 당일 일련번호 추적
    std::string lastDate_;   // "YYYYMMDD"
    int         dailySeq_{ 0 };
};
```

### Model/OrderModel.cpp — 주요 구현 명세

**`reserve()`**
```
Order o;
o.sampleId  = sampleId;
o.customer  = customer;
o.quantity  = quantity;
o.status    = OrderStatus::RESERVED;
o.createdAt = 현재 시스템 시각 ("YYYY-MM-DD HH:MM:SS")
o.orderId   = generateOrderId();
orders_.push_back(o);
return o;
```

**`generateOrderId()`**
```
현재 날짜를 "YYYYMMDD" 형식으로 취득
if (lastDate_ != 오늘날짜)
    lastDate_ = 오늘날짜, dailySeq_ = 0
++dailySeq_
return "ORD-" + lastDate_ + "-" + zero-padded(dailySeq_, 4)
예) "ORD-20260416-0001"
```
- `GetLocalTime()` (Win32) 또는 `std::chrono` + `std::format` 사용
- 일련번호는 날짜가 바뀌면 0001로 리셋

**`findById()`**
- `std::ranges::find_if`로 `orderId` 일치 탐색
- 없으면 `std::nullopt`

**`findByStatus()`**
- `std::ranges::filter_view` 또는 copy_if로 해당 상태 항목 수집
- `std::vector<Order>` 반환

**`updateStatus()`**
- `findById`로 대상 탐색 → 없으면 `false`
- 상태 변경 후 `true` 반환

**`all()`**
- `std::span<const Order>` 반환 (복사 없음)

**`totalCount()`**
- `static_cast<int>(orders_.size())`

---

## 3. OrderView 설계

### View/OrderView.h

```cpp
#pragma once
#include "../Model/Order.h"
#include "../Model/Sample.h"
#include <span>
#include <string>
#include <vector>

class OrderView {
public:
    // 시료 목록을 보여주며 주문 정보 입력 받기
    // sampleId, customer, quantity를 채운 Order 반환 (orderId/status/createdAt은 미설정)
    Order promptOrderInput(std::span<const Sample> samples) const;

    // 접수 완료 결과 출력
    void showOrderResult(const Order& order) const;

    // RESERVED 주문 목록 출력 (승인/거절 메뉴용 — Phase 4에서도 재사용)
    void showReservedList(const std::vector<Order>& orders,
                          std::span<const Sample>   samples) const;

    // 공통
    void showMessage(const std::string& msg) const;
    void showError(const std::string& msg)   const;
    int  promptMenuChoice()                  const;
};
```

### View/OrderView.cpp — 화면 구성 명세

**`promptOrderInput()`** 입력 순서:
```
============================================================
 [2] 시료 주문
------------------------------------------------------------
 ※ 등록된 시료 목록
 ID       시료명                    재고
 S-001    실리콘 웨이퍼-8인치       480 ea
 S-002    GaN 에피택셀-4인치        220 ea
 ...
------------------------------------------------------------
 시료 ID  > S-003
 고객명   > 삼성전자 파운드리
 주문 수량 > 200

 --- 입력 확인 ---
 시료     : SiC 파워기판-6인치  (S-003)
 고객     : 삼성전자 파운드리
 수량     : 200 ea

 [Y] 예약 접수   [N] 취소
 선택 > 
```
- 시료 ID 입력 후 samples에서 유효성 확인 → 없으면 오류 메시지 후 재입력
- 고객명: 비어있으면 재입력
- 주문 수량: 1 이상 정수만 허용
- 최종 확인 N → 처음부터 재입력

**`showOrderResult()`** 출력 형식:
```
------------------------------------------------------------
 예약 접수 완료.

 주문번호  : ORD-20260416-0043
 시료 ID  : S-003
 고객명   : 삼성전자 파운드리
 수량     : 200 ea
 현재 상태 : RESERVED
 접수 일시 : 2026-04-16 09:32:15

 ※ 재고 확인 및 승인은 [3] 주문 승인/거절 메뉴에서 진행하세요.
------------------------------------------------------------
```

**`showReservedList()`** 출력 형식:
```
 승인 대기 중인 예약 목록  (RESERVED)
------------------------------------------------------------
 번호  주문번호           고객              시료            수량    상태
  [1]  ORD-20260416-0041  LG이노텍          산화막 웨이퍼   300 ea  RESERVED
  [2]  ORD-20260416-0042  SK하이닉스        실리콘 웨이퍼   150 ea  RESERVED
  [3]  ORD-20260416-0043  삼성전자 파운드리  SiC 파워기판   200 ea  RESERVED
------------------------------------------------------------
```
- `samples`에서 시료명을 찾아 주문번호와 함께 표시
- 목록이 비어있으면 "대기 중인 주문이 없습니다." 출력

---

## 4. OrderController 설계

### Controller/OrderController.h

```cpp
#pragma once
#include "../Model/OrderModel.h"
#include "../Model/SampleModel.h"
#include "../View/OrderView.h"

class OrderController {
public:
    OrderController(OrderModel&  orderModel,
                    SampleModel& sampleModel,
                    OrderView&   view);
    void run();

private:
    void handleReserve();

    OrderModel&  orderModel_;
    SampleModel& sampleModel_;
    OrderView&   view_;
};
```

### Controller/OrderController.cpp — 흐름 명세

**`run()`**
```
// Phase 3에서는 주문 접수 단일 기능
handleReserve();
```
> Phase 8 통합 시 MainController에서 직접 호출하는 방식으로 단순화 가능

**`handleReserve()`**
```
Order o = view_.promptOrderInput(sampleModel_.all());
Order created = orderModel_.reserve(o.sampleId, o.customer, o.quantity);
view_.showOrderResult(created);
```

---

## 5. vcxproj 등록

### ClCompile 추가 (SampleOrderSystem_project.vcxproj)

```xml
<ClCompile Include="Model\OrderModel.cpp" />
<ClCompile Include="View\OrderView.cpp" />
<ClCompile Include="Controller\OrderController.cpp" />
```

### vcxproj.filters 추가

```xml
<ClCompile Include="Model\OrderModel.cpp">
  <Filter>소스 파일</Filter>
</ClCompile>
<ClCompile Include="View\OrderView.cpp">
  <Filter>소스 파일</Filter>
</ClCompile>
<ClCompile Include="Controller\OrderController.cpp">
  <Filter>소스 파일</Filter>
</ClCompile>
```

---

## 6. 유효성 검사 규칙

| 입력 항목 | 규칙 | 처리 위치 |
|-----------|------|-----------|
| 시료 ID | `SampleModel`에 등록된 ID여야 함 | `OrderView::promptOrderInput()` |
| 고객명 | 비어있으면 안 됨 | `OrderView::promptOrderInput()` |
| 주문 수량 | 1 이상 정수 | `OrderView::promptOrderInput()` |

- 주문 시점에는 재고 확인을 하지 않는다 (PRD 명세 준수).
- 유효하지 않은 시료 ID 입력 시 시료 목록을 다시 표시하고 재입력 유도.

---

## 7. 주문번호 채번 상세

```
형식  : ORD-{YYYYMMDD}-{NNNN}
예시  : ORD-20260416-0001

규칙:
- YYYYMMDD : 주문 접수 시점의 날짜
- NNNN     : 당일 접수 순서 (4자리 zero-padding, 0001부터 시작)
- 날짜가 바뀌면 NNNN을 0001로 리셋
- 동일 날짜 내 중복 없음
```

Win32 `GetLocalTime()` 사용 예:
```cpp
SYSTEMTIME st{};
GetLocalTime(&st);
// std::format으로 "YYYYMMDD" 조합
```

---

## 8. Phase 3 완료 조건

- [ ] `OrderModel`: reserve / findById / findByStatus / updateStatus / all / totalCount 구현 완료
- [ ] 주문번호 채번 (`ORD-YYYYMMDD-NNNN`) 정상 동작 확인
- [ ] `OrderView`: 주문 입력·확인·결과·RESERVED 목록 화면 구현 완료
- [ ] `OrderController`: handleReserve 흐름 구현 완료
- [ ] vcxproj / vcxproj.filters 등록 완료
- [ ] 빌드 성공 (경고 없음)
- [ ] 시료 ID 유효성 검사, 주문 접수, 결과 출력 동작 확인
- [ ] 사용자 검토 완료 후 commit
