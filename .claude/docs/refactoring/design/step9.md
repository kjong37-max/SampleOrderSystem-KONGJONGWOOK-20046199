# Step 9 — Controller 입력 처리 구조 통일 설계

> 참고: [PLAN.md](../PLAN.md) | 브랜치: `refactoring`  
> 구현 전 **사용자 검토 후 commit** 규칙 준수

---

## 목표

Controller마다 다른 메뉴 루프 구조를 일관된 패턴으로 통일한다.  
**동작 변경 없음. 기존 테스트 모두 계속 통과해야 한다.**

---

## 현재 불일치 현황

| Controller | run() 구조 | 문제 |
|------------|-----------|------|
| `SampleController` | while + showMenu + switch | ✅ 표준 패턴 |
| `OrderController` | handleReserve() 직접 호출 | ❌ 루프/메뉴 없음 |
| `ApprovalController` | while + inline 처리 | ⚠️ view 메서드 대신 직접 cout |
| `ProductionController` | while + showMenu + switch | ✅ 표준 패턴 |
| `MonitorController` | while + showMenu + switch | ✅ 표준 패턴 |
| `ReleaseController` | while + inline 처리 | ⚠️ showMenu 없이 직접 처리 |

---

## 변경 대상

### OrderController — 메뉴 루프 추가

현재 `run()`이 `handleReserve()`를 직접 호출하고 종료된다.  
[2] 시료 주문 메뉴는 단일 기능이므로 루프보다 단순 호출이 적절하나,  
다른 Controller와 구조를 맞추기 위해 `view_.showMenu()` 출력 후 처리하는 방식으로 통일한다.

```cpp
// 변경 전
void OrderController::run() {
    handleReserve();
}

// 변경 후
void OrderController::run() {
    view_.showMenu();
    handleReserve();
}
```

> `OrderView`에 `showMenu()` 메서드가 없으므로 추가 필요.

### ReleaseController — showMenu 추가

현재 `run()`이 CONFIRMED 목록을 바로 표시한다.  
진입 시 `[6] 출고 처리` 헤더를 먼저 표시하는 구조로 통일한다.

실제로 `showConfirmedList()`가 헤더 출력을 포함하고 있어(Step 7 이후 OrderView로 이동),  
별도 `showMenu()`는 불필요할 수 있다. **이 Step에서는 OrderController만 수정한다.**

---

## 생성/수정 파일 목록

| 파일 | 작업 |
|------|------|
| `View/OrderView.h` | `showMenu()` 메서드 추가 |
| `View/OrderView.cpp` | `showMenu()` 구현 추가 |
| `Controller/OrderController.cpp` | `run()` 수정 |

---

## 수정 명세

### View/OrderView.h

```cpp
virtual void showMenu() const;  // 추가
```

### View/OrderView.cpp

```cpp
void OrderView::showMenu() const {
    std::cout << "============================================================\n";
    std::cout << " [2] 시료 주문\n";
    std::cout << "------------------------------------------------------------\n";
}
```

### Controller/OrderController.cpp

```cpp
void OrderController::run() {
    view_.showMenu();
    handleReserve();
}
```

---

## Step 9 완료 조건

- [ ] `OrderView.h` — `showMenu()` 추가
- [ ] `OrderView.cpp` — `showMenu()` 구현 추가
- [ ] `OrderController.cpp` — `run()` 수정
- [ ] 빌드 성공 (경고 없음)
- [ ] 전체 테스트 통과
- [ ] 사용자 검토 완료 후 commit
