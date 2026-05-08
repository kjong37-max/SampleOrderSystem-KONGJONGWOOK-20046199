# Step 4 — OrderStatus 변환 함수 통합 설계

> 참고: [PLAN.md](../PLAN.md) | 브랜치: `refactoring`  
> 구현 전 **사용자 검토 후 commit** 규칙 준수

---

## 목표

`OrderStatus ↔ 문자열` 변환 코드가 2곳에 중복 → `Model/Order.h`의 공용 인라인 함수로 통합한다.  
**동작 변경 없음. 기존 테스트 모두 계속 통과해야 한다.**

---

## 현재 중복 위치

| 파일 | 코드 | 내용 |
|------|------|------|
| `Data/OrderStore.cpp:8-25` | `statusToStr()`, `strToStatus()` | 직렬화용 변환 |
| `View/OrderView.cpp:97-104` | switch 문 inline | 화면 출력용 변환 |

---

## 생성/수정 파일 목록

| 구분 | 파일 | 작업 |
|------|------|------|
| 수정 | `Model/Order.h` | 공용 인라인 함수 2개 추가 |
| 수정 | `Data/OrderStore.h` | private statusToStr/strToStatus 선언 제거 |
| 수정 | `Data/OrderStore.cpp` | 구현 제거 → Order.h 공용 함수 사용 |
| 수정 | `View/OrderView.cpp` | switch 문 제거 → Order.h 공용 함수 사용 |

---

## Model/Order.h 추가 내용

기존 `OrderStatus` enum 정의 아래에 인라인 함수 추가:

```cpp
inline std::string orderStatusToStr(OrderStatus s) {
    switch (s) {
    case OrderStatus::RESERVED:  return "RESERVED";
    case OrderStatus::REJECTED:  return "REJECTED";
    case OrderStatus::PRODUCING: return "PRODUCING";
    case OrderStatus::CONFIRMED: return "CONFIRMED";
    case OrderStatus::RELEASED:  return "RELEASED";
    default:                     return "RESERVED";
    }
}

inline OrderStatus strToOrderStatus(const std::string& s) {
    if (s == "REJECTED")  return OrderStatus::REJECTED;
    if (s == "PRODUCING") return OrderStatus::PRODUCING;
    if (s == "CONFIRMED") return OrderStatus::CONFIRMED;
    if (s == "RELEASED")  return OrderStatus::RELEASED;
    return OrderStatus::RESERVED;
}
```

---

## 수정 명세

### Data/OrderStore.h

private 섹션에서 아래 선언 제거:
```cpp
static std::string statusToStr(OrderStatus s);   // ← 삭제
static OrderStatus strToStatus(const std::string& s);  // ← 삭제
```

### Data/OrderStore.cpp

- `statusToStr()` / `strToStatus()` 구현 전체 삭제
- `load()` 내: `strToStatus(...)` → `strToOrderStatus(...)`
- `save()` 내: `statusToStr(...)` → `orderStatusToStr(...)`

### View/OrderView.cpp

`showOrderResult()` 내 switch 문 제거:
```cpp
// 변경 전
std::string statusStr;
switch (order.status) {
    case OrderStatus::RESERVED:  statusStr = "RESERVED";  break;
    // ...
}
std::cout << " 현재 상태 : " << statusStr << "\n";

// 변경 후
std::cout << " 현재 상태 : " << orderStatusToStr(order.status) << "\n";
```

---

## Step 4 완료 조건

- [ ] `Model/Order.h`에 공용 함수 2개 추가
- [ ] `Data/OrderStore.h/.cpp` 수정 완료
- [ ] `View/OrderView.cpp` 수정 완료
- [ ] 빌드 성공 (경고 없음)
- [ ] 전체 테스트 통과 (OrderStoreTest의 상태값 검증 포함)
- [ ] 사용자 검토 완료 후 commit
