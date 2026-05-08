# 리팩토링 계획 (PLAN)

> 브랜치: `refactoring`  
> 원칙: **테스트 먼저 보강 → 영향 작은 변경 → 영향 큰 변경** 순으로 진행  
> 각 단계 완료 후 빌드 + 테스트 통과 확인 → commit

---

## 진행 원칙

1. 각 Step은 독립적으로 빌드·테스트 가능해야 한다
2. 리팩토링 전에 해당 영역의 테스트가 충분히 있어야 한다
3. 동작 변경 없이 구조만 개선한다 (기능 추가는 별도 브랜치)
4. 각 Step 완료 시 `refactoring` 브랜치에 commit

---

## Step 1 — 테스트 커버리지 보강 (기반 작업)

> **목적**: 이후 리팩토링 과정에서 회귀를 즉시 감지할 수 있도록 테스트를 먼저 충분히 작성한다.

### Step 1-1. DataStore 테스트 추가

**파일**: `Tests/DataStoreTest.cpp` (신규)

| 테스트명 | 검증 항목 |
|---------|-----------|
| `SetAndGet_StoresValue` | set/get 기본 동작 |
| `Has_ReturnsTrueForExistingKey` | has() 검증 |
| `Remove_DeletesKey` | remove() 검증 |
| `SaveAndLoad_TXT_RoundTrip` | TXT 저장 후 재로드 시 동일한 값 |
| `SaveAndLoad_EmptyStore_CreatesFile` | 빈 저장소 저장 |
| `Load_NonExistentFile_EmptyStore` | 존재하지 않는 파일 로드 시 빈 상태 |

### Step 1-2. SampleStore / OrderStore 테스트 추가

**파일**: `Tests/SampleStoreTest.cpp`, `Tests/OrderStoreTest.cpp` (신규)

**SampleStore 테스트:**

| 테스트명 | 검증 항목 |
|---------|-----------|
| `Save_And_Load_RoundTrip` | 저장 후 로드 시 동일한 시료 5종 복원 |
| `Load_EmptyFile_EmptyModel` | 빈 파일 로드 시 모델 비어있음 |
| `Save_UpdatedStock_PersistsCorrectly` | 재고 변경 후 저장·복원 정합성 |

**OrderStore 테스트:**

| 테스트명 | 검증 항목 |
|---------|-----------|
| `Save_And_Load_RoundTrip` | 주문 저장 후 복원 (상태 포함) |
| `Save_DailySeq_Restored` | 채번 상태(lastDate, dailySeq) 복원 |
| `Load_AllStatusValues_Correctly` | 5개 상태값 직렬화/역직렬화 정확성 |

### Step 1-3. ApprovalController 테스트 보완

**파일**: `Tests/ApprovalControllerTest.cpp` (수정)

현재 문제: `MockOrderView`가 선언만 되어 있고 실제 사용 안 됨.

**추가할 테스트:**

| 테스트명 | 검증 항목 |
|---------|-----------|
| `Run_EmptyReservedList_NoInteraction` | RESERVED 없을 때 루프 즉시 종료 확인 |
| `CalcEffectiveStock_MultipleConfirmed_SumDeducted` | 동일 시료 CONFIRMED 여러 건 차감 |
| `CalcEffectiveStock_DifferentSample_NotAffected` | 다른 시료 CONFIRMED는 영향 없음 |

### Step 1-4. ProductionController / ReleaseController 테스트 추가

**파일**: `Tests/ProductionControllerTest.cpp`, `Tests/ReleaseControllerTest.cpp` (신규)

**ProductionController 테스트:**

| 테스트명 | 검증 항목 |
|---------|-----------|
| `HandleComplete_NoCurrentJob_ShowsMessage` | 처리 중 작업 없을 때 메시지 출력 |
| `HandleComplete_CompletesJob_UpdatesStockAndStatus` | 완료 시 재고 보충 + CONFIRMED 전환 |

**ReleaseController 테스트:**

| 테스트명 | 검증 항목 |
|---------|-----------|
| `Run_NoConfirmedOrders_ShowsEmptyMessage` | CONFIRMED 없을 때 메시지 출력 |
| `Release_ChangesStatusToReleased` | CONFIRMED → RELEASED 전환 |
| `Release_DoesNotChangeStock` | 출고 시 재고 변화 없음 |

---

## Step 2 — 매직 넘버 상수화 (영향: 최소)

> **목적**: 코드 가독성 향상. 동작 변화 없음.

### Step 2-1. 생산 보정 상수

**파일**: `Model/ProductionJob.h` 또는 신규 `Model/Constants.h`

```cpp
namespace SemiConst {
    constexpr double PRODUCTION_CORRECTION = 0.9;  // 생산량 보정 비율
    constexpr int    PAGE_SIZE             = 5;     // 목록 페이지 크기
    constexpr int    BAR_WIDTH             = 20;    // 모니터링 바 그래프 너비
}
```

**수정 대상:**
- `Controller/ApprovalController.cpp` — `0.9` → `SemiConst::PRODUCTION_CORRECTION`
- `View/SampleView.cpp` — `PAGE_SIZE = 5` → `SemiConst::PAGE_SIZE`
- `View/MonitorView.cpp` — `width = 20` → `SemiConst::BAR_WIDTH`

---

## Step 3 — 날짜/시간 유틸리티 추출 (영향: 낮음)

> **목적**: 동일한 날짜 포맷 코드 3곳 → 1곳으로 통합

**파일**: `Utils/TimeUtils.h`, `Utils/TimeUtils.cpp` (신규)

```cpp
namespace TimeUtils {
    // "YYYY-MM-DD HH:MM:SS" 형식 현재 시각 반환
    std::string nowDateTime();

    // "YYYYMMDD" 형식 현재 날짜 반환
    std::string nowDate();
}
```

**수정 대상:**
- `Model/OrderModel.cpp` — createdAt 설정
- `Controller/MainController.cpp` — 메인 메뉴 시각 출력
- `Controller/ReleaseController.cpp` — 출고 처리 일시

---

## Step 4 — OrderStatus 변환 함수 통합 (영향: 낮음)

> **목적**: 동일한 status ↔ 문자열 변환이 3곳에 중복 → 1곳으로 통합

**파일**: `Model/Order.h`에 인라인 함수로 추가

```cpp
// Order.h 하단에 추가
inline std::string orderStatusToStr(OrderStatus s);
inline OrderStatus strToOrderStatus(const std::string& s);
```

**수정 대상:**
- `Data/OrderStore.cpp` — statusToStr(), strToStatus() 제거 후 공용 함수 사용
- `View/OrderView.cpp` — status 표시 로직에 공용 함수 사용
- `Controller/ApprovalController.cpp` — status 출력 부분

---

## Step 5 — promptMenuChoice() 공통화 (영향: 낮음)

> **목적**: 4개 View에 동일하게 구현된 `promptMenuChoice()` 중복 제거

**파일**: `Utils/ConsoleUtils.h`, `Utils/ConsoleUtils.cpp` (신규)

```cpp
namespace ConsoleUtils {
    int  promptMenuChoice();
    void printSeparator(char c = '-', int width = 60);
    void printDoubleSeparator(int width = 60);
}
```

**수정 대상:**
- `View/SampleView.cpp`, `View/OrderView.cpp`,
  `View/ProductionView.cpp`, `View/MonitorView.cpp`의 `promptMenuChoice()` 구현 제거 →
  `ConsoleUtils::promptMenuChoice()` 위임

---

## Step 6 — View virtual 선언 일관화 (영향: 낮음)

> **목적**: `OrderView`만 virtual인 현재 상태를 전체 View에 일관 적용

**수정 대상:**
- `View/SampleView.h` — virtual 소멸자 + virtual 메서드 추가
- `View/ProductionView.h` — 동일
- `View/MonitorView.h` — 동일

---

## Step 7 — ReleaseController View 책임 분리 (영향: 중간)

> **목적**: `ReleaseController`에 직접 구현된 `showConfirmedList()` → `OrderView`로 이동

**변경 내용:**
- `View/OrderView.h` — `showConfirmedList(const std::vector<Order>&, std::span<const Sample>)` 추가
- `View/OrderView.cpp` — 구현 이전
- `Controller/ReleaseController.cpp` — `showConfirmedList()` 제거, `view_.showConfirmedList()` 호출로 교체
- `Controller/ReleaseController.h` — private `showConfirmedList()` 제거

---

## Step 8 — 유효 재고 계산 단일화 (영향: 중간)

> **목적**: `ApprovalController`와 `MonitorController` 두 곳에 분산된 재고 계산 로직 통합

**파일**: `Model/SampleModel.h/.cpp` 에 메서드 추가

```cpp
// SampleModel에 추가
// 특정 시료의 유효 재고 계산 (현재 stock - CONFIRMED 수량 합)
int effectiveStock(const std::string& sampleId,
                   const OrderModel&  orderModel) const;

// 재고 상태 판정
StockStatus calcStockStatus(const std::string& sampleId,
                             const OrderModel&  orderModel) const;
```

**수정 대상:**
- `Controller/ApprovalController.cpp` — `calcEffectiveStock()` 제거 → `SampleModel` 메서드 사용
- `Controller/MonitorController.cpp` — `calcStockStatus()` 제거 → `SampleModel` 메서드 사용

> `StockStatus` enum은 `Model/Sample.h`로 이전 (현재 `View/MonitorView.h`에 정의됨)

---

## Step 9 — Controller 입력 처리 구조 통일 (영향: 중간)

> **목적**: Controller마다 다른 메뉴 루프 구조를 일관된 패턴으로 통일

**변경 패턴**: 모든 Sub-Controller가 아래 구조를 따른다.

```cpp
void XxxController::run() {
    while (true) {
        view_.showMenu();
        switch (view_.promptMenuChoice()) {
        case 1: handleXxx1(); break;
        case 2: handleXxx2(); break;
        case 0: return;
        default: view_.showError("잘못된 선택입니다.");
        }
    }
}
```

**수정 대상:**
- `Controller/OrderController.cpp` — `run()` 구조 통일 (현재 handleReserve 직접 호출)
- `Controller/ReleaseController.cpp` — while + switch 구조로 통일

---

## Step 10 — 테스트 커버리지 최종 점검 (마무리)

> Step 2~9 리팩토링 후 새로 추가된 유틸리티/변경된 로직에 대한 테스트 추가

- `Tests/TimeUtilsTest.cpp` — nowDateTime() / nowDate() 포맷 검증
- `Tests/OrderStatusUtilsTest.cpp` — statusToStr / strToStatus 양방향 검증
- 기존 테스트 전체 통과 확인

---

## 전체 일정 요약

| Step | 내용 | 영향도 | 예상 변경 파일 수 |
|------|------|--------|-----------------|
| 1 | 테스트 커버리지 보강 | 없음 (테스트만) | +6개 |
| 2 | 매직 넘버 상수화 | 최소 | +1, 수정 3 |
| 3 | 날짜/시간 유틸리티 추출 | 낮음 | +2, 수정 3 |
| 4 | OrderStatus 변환 통합 | 낮음 | 수정 3 |
| 5 | promptMenuChoice 공통화 | 낮음 | +2, 수정 4 |
| 6 | View virtual 일관화 | 낮음 | 수정 3 |
| 7 | ReleaseController View 분리 | 중간 | 수정 4 |
| 8 | 유효 재고 계산 단일화 | 중간 | 수정 3 |
| 9 | Controller 구조 통일 | 중간 | 수정 2 |
| 10 | 테스트 최종 점검 | 없음 | +2 |
