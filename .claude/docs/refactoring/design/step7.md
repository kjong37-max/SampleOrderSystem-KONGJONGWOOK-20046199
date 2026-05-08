# Step 7 — ReleaseController View 책임 분리 설계

> 참고: [PLAN.md](../PLAN.md) | 브랜치: `refactoring`  
> 구현 전 **사용자 검토 후 commit** 규칙 준수

---

## 목표

`ReleaseController`에 직접 구현된 `showConfirmedList()`(콘솔 출력)를 `OrderView`로 이동한다.  
Controller는 비즈니스 로직만 담당하고, 화면 출력은 View가 담당하도록 SRP를 적용한다.  
**동작 변경 없음. 기존 테스트 모두 계속 통과해야 한다.**

---

## 현재 문제

`ReleaseController.cpp:59-82`의 `showConfirmedList()`가 직접 `std::cout`으로 출력한다.  
이는 View 계층의 역할이므로 `OrderView`로 이동해야 한다.

---

## 생성/수정 파일 목록

| 파일 | 작업 |
|------|------|
| `View/OrderView.h` | `showConfirmedList()` 메서드 추가 |
| `View/OrderView.cpp` | `showConfirmedList()` 구현 이전 |
| `Controller/ReleaseController.h` | private `showConfirmedList()` 선언 제거 |
| `Controller/ReleaseController.cpp` | `showConfirmedList()` 구현 제거 → `view_.showConfirmedList()` 호출 |

---

## 수정 명세

### View/OrderView.h

기존 메서드 목록 뒤에 추가:

```cpp
// CONFIRMED 상태 주문 목록 출력 (출고 처리 메뉴용)
virtual void showConfirmedList(const std::vector<Order>& orders) const;
```

### View/OrderView.cpp

`showConfirmedList()` 구현 추가 — `ReleaseController.cpp:59-82`의 내용 그대로 이전:

```cpp
void OrderView::showConfirmedList(const std::vector<Order>& orders) const {
    std::cout << "============================================================\n"
              << " [6] 출고 처리\n"
              << "------------------------------------------------------------\n"
              << " 출고 가능 주문  (CONFIRMED)\n"
              << std::left
              << std::setw(6)  << " 번호"
              << std::setw(22) << "주문번호"
              << std::setw(18) << "고객"
              << std::setw(10) << "시료 ID"
              << "수량\n"
              << "------------------------------------------------------------\n";

    for (int i = 0; i < static_cast<int>(orders.size()); ++i) {
        const auto& o = orders[i];
        std::cout << std::left
                  << "  [" << std::setw(2) << (i + 1) << "] "
                  << std::setw(22) << o.orderId
                  << std::setw(18) << o.customer
                  << std::setw(10) << o.sampleId
                  << o.quantity << " ea\n";
    }
    std::cout << "------------------------------------------------------------\n";
}
```

`OrderView.cpp`에 `#include <iomanip>` 이미 있는지 확인 후 없으면 추가.

### Controller/ReleaseController.h

private 섹션에서 삭제:
```cpp
void showConfirmedList(const std::vector<Order>& orders) const;  // ← 삭제
```

### Controller/ReleaseController.cpp

- `showConfirmedList()` 구현 전체 삭제
- `run()` 내 `showConfirmedList(confirmed)` 호출 → `view_.showConfirmedList(confirmed)` 교체
- `#include <iomanip>` 제거 (더 이상 setw 사용 안 함)

---

## ReleaseControllerTest 영향 확인

`Tests/ReleaseControllerTest.cpp`는 Model 계층 직접 테스트이므로 영향 없음.  
`showConfirmedList()` 자체 테스트는 Step 10에서 OrderView Mock 테스트로 보완 가능.

---

## Step 7 완료 조건

- [ ] `OrderView.h` — `showConfirmedList()` 추가
- [ ] `OrderView.cpp` — 구현 이전
- [ ] `ReleaseController.h` — private 선언 제거
- [ ] `ReleaseController.cpp` — 구현 제거 + view_ 위임 교체
- [ ] 빌드 성공 (경고 없음)
- [ ] 전체 테스트 통과
- [ ] 사용자 검토 완료 후 commit
