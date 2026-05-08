# Phase 1 — 프로젝트 뼈대 구성 설계

> 참고: [PLAN.md](../PLAN.md) | [PRD.md](../PRD.md)  
> 구현 전 **사용자 검토 후 commit** 규칙 준수

---

## 목표

전체 시스템이 공유할 MVC 디렉터리 구조와 도메인 모델 헤더를 정의한다.  
이 Phase에서는 **헤더 파일과 main.cpp만 작성**하며, 로직 구현은 포함하지 않는다.

---

## 1. 디렉터리 구조 및 파일 목록

```
SampleOrderSystem_project/
│
├── main.cpp
│
├── Model/
│   ├── Sample.h
│   ├── Order.h
│   └── ProductionJob.h
│
├── View/                          ← 헤더 선언만 (구현은 Phase 2~7)
│   ├── SampleView.h
│   ├── OrderView.h
│   ├── ProductionView.h
│   └── MonitorView.h
│
├── Controller/                    ← 헤더 선언만 (구현은 Phase 2~8)
│   ├── MainController.h
│   ├── SampleController.h
│   ├── OrderController.h
│   ├── ApprovalController.h
│   ├── ReleaseController.h
│   └── MonitorController.h
│
└── Data/                          ← 헤더 선언만 (구현은 Phase 8)
    ├── DataStore.h
    └── RecordStore.h
```

> Phase 1에서 실제 작성하는 파일: `main.cpp`, `Model/` 하위 3개 헤더  
> 나머지 View / Controller / Data 헤더는 빈 클래스 선언만 작성하여 구조만 잡는다.

---

## 2. 도메인 모델 상세 설계

### 2.1 Model/Sample.h

```cpp
#pragma once
#include <string>

struct Sample {
    std::string id;           // 고유 식별자. 형식: "S-NNN" (예: "S-001")
    std::string name;         // 시료명 (예: "실리콘 웨이퍼-8인치")
    double      avgProdTime;  // 평균 생산시간 (단위: min/ea)
    double      yield;        // 수율 (0.0 ~ 1.0, 예: 0.92)
    int         stock;        // 현재 재고 수량 (단위: ea)
};
```

**설계 결정:**
- `id`는 시스템 외부에서 입력받는 값으로, 자동 채번하지 않는다 (PRD 명세 준수).
- `yield`는 `double`로 저장하며 0.0 초과 1.0 이하의 값만 허용한다 (유효성 검사는 Model 계층에서 처리).
- `stock`은 음수가 될 수 없다 (출고·생산 로직에서 보장).

---

### 2.2 Model/Order.h

```cpp
#pragma once
#include <string>

enum class OrderStatus {
    RESERVED,   // 주문 접수
    REJECTED,   // 주문 거절
    PRODUCING,  // 생산 중 (재고 부족으로 생산라인 투입)
    CONFIRMED,  // 출고 대기 (재고 확보 완료)
    RELEASED    // 출고 완료
};

struct Order {
    std::string  orderId;    // 주문번호. 형식: "ORD-YYYYMMDD-NNNN"
    std::string  sampleId;   // 주문 대상 시료 ID
    std::string  customer;   // 고객명
    int          quantity;   // 주문 수량 (ea)
    OrderStatus  status;     // 현재 주문 상태
    std::string  createdAt;  // 주문 생성 일시. 형식: "YYYY-MM-DD HH:MM:SS"
};
```

**설계 결정:**
- `OrderStatus`를 `enum class`로 정의하여 타입 안전성을 보장한다.
- `orderId`는 OrderModel에서 자동 생성한다 (Phase 3에서 구현).
- `createdAt`은 주문 접수 시점의 시스템 시간을 문자열로 저장한다.

**주문번호 채번 규칙 (Phase 3에서 구현):**
```
형식: ORD-{YYYYMMDD}-{NNNN}
예시: ORD-20260416-0001
- NNNN: 날짜 내 일련번호, 0001부터 시작, 4자리 zero-padding
- 날짜가 바뀌면 NNNN을 0001로 리셋
```

---

### 2.3 Model/ProductionJob.h

```cpp
#pragma once
#include <string>

struct ProductionJob {
    std::string orderId;       // 연관 주문번호
    std::string sampleId;      // 생산할 시료 ID
    int         shortage;      // 부족분 = 주문 수량 - 현재 유효 재고
    int         actualQty;     // 실 생산량 = ceil(부족분 / (수율 × 0.9))
    double      totalMinutes;  // 총 생산시간 = avgProdTime × actualQty
};
```

**설계 결정:**
- `ProductionJob`은 `ProductionLine` 큐에 적재되는 단위 작업 객체다.
- 생산량 계산은 `enqueue()` 시점에 확정하여 저장한다 (조회 시마다 재계산하지 않음).
- `0.9` 보정 상수는 PRD 명세 고정값이며 코드에 직접 리터럴로 사용한다.

**생산량 계산 공식:**
```
부족분     = 주문 수량 - 현재 유효 재고
실 생산량  = ceil(부족분 / (수율 × 0.9))
총 생산시간 = avgProdTime × 실 생산량
```

---

## 3. View / Controller / Data 헤더 (빈 선언)

Phase 1에서는 전체 구조를 vcxproj에 등록하기 위해 빈 클래스 선언만 작성한다.

### View 헤더 공통 형식

```cpp
// View/SampleView.h (예시 — 나머지 View도 동일 형식)
#pragma once

class SampleView {
public:
    // Phase 2에서 구현
};
```

### Controller 헤더 공통 형식

```cpp
// Controller/MainController.h (예시)
#pragma once

class MainController {
public:
    void run();
    // Phase 8에서 구현
};
```

### Data 헤더 공통 형식

```cpp
// Data/DataStore.h (예시)
#pragma once

// Phase 8에서 DataPersistence 참고 저장소 기반으로 구현
class DataStore {
};
```

---

## 4. main.cpp

```cpp
#include "Controller/MainController.h"

int main() {
    MainController controller;
    controller.run();
    return 0;
}
```

**설계 결정:**
- Phase 1에서는 `MainController::run()`의 본체가 비어 있으므로 빌드만 성공하면 된다.
- Model·View 인스턴스를 main에서 생성하여 Controller에 주입하는 구조는 Phase 8 통합 시 확정한다.

---

## 5. vcxproj 파일 등록

Phase 1에서 생성한 모든 파일을 `SampleOrderSystem_project.vcxproj`와 `.vcxproj.filters`에 등록한다.

### vcxproj — ClInclude 추가 대상

```xml
<ItemGroup>
  <!-- Model -->
  <ClInclude Include="Model\Sample.h" />
  <ClInclude Include="Model\Order.h" />
  <ClInclude Include="Model\ProductionJob.h" />
  <!-- View -->
  <ClInclude Include="View\SampleView.h" />
  <ClInclude Include="View\OrderView.h" />
  <ClInclude Include="View\ProductionView.h" />
  <ClInclude Include="View\MonitorView.h" />
  <!-- Controller -->
  <ClInclude Include="Controller\MainController.h" />
  <ClInclude Include="Controller\SampleController.h" />
  <ClInclude Include="Controller\OrderController.h" />
  <ClInclude Include="Controller\ApprovalController.h" />
  <ClInclude Include="Controller\ReleaseController.h" />
  <ClInclude Include="Controller\MonitorController.h" />
  <!-- Data -->
  <ClInclude Include="Data\DataStore.h" />
  <ClInclude Include="Data\RecordStore.h" />
</ItemGroup>
```

### vcxproj — ClCompile 추가 대상

```xml
<ItemGroup>
  <ClCompile Include="main.cpp" />
</ItemGroup>
```

### vcxproj.filters — 필터 매핑

```xml
<!-- 소스 파일 -->
<ClCompile Include="main.cpp">
  <Filter>소스 파일</Filter>
</ClCompile>

<!-- 헤더 파일 -->
<ClInclude Include="Model\Sample.h">
  <Filter>헤더 파일</Filter>
</ClInclude>
<!-- ... 나머지 헤더도 동일하게 헤더 파일 필터에 등록 -->
```

---

## 6. Phase 1 완료 조건

- [ ] `Model/Sample.h`, `Model/Order.h`, `Model/ProductionJob.h` 작성 완료
- [ ] `View/`, `Controller/`, `Data/` 하위 헤더 빈 선언 완료
- [ ] `main.cpp` 작성 완료
- [ ] `vcxproj` 및 `vcxproj.filters`에 전체 파일 등록 완료
- [ ] **Debug / Release x64 빌드 성공** (경고 없음)
- [ ] 사용자 검토 완료 후 commit
