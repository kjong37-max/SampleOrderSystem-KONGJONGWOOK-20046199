# Phase 9 — 더미 데이터 & gMock 테스트 설계

> 참고: [PLAN.md](../PLAN.md) | [PRD.md](../PRD.md)  
> 구현 전 **사용자 검토 후 commit** 규칙 준수

---

## 목표

1. **초기 더미 데이터**: PRD 예시 데이터 5종을 `Data/samples.txt`에 미리 작성하여  
   프로그램 최초 실행 시 시료가 등록된 상태로 시작한다.
2. **gMock 단위 테스트**: 핵심 도메인 로직(SampleModel, OrderModel, ProductionLine,  
   ApprovalController)을 gMock 기반으로 검증한다.

---

## 1. 생성 파일 목록

| 구분 | 파일 | 작업 |
|------|------|------|
| 데이터 | `Data/samples.txt` | 신규 (초기 더미 데이터) |
| 테스트 | `Tests/SampleModelTest.cpp` | 신규 |
| 테스트 | `Tests/OrderModelTest.cpp` | 신규 |
| 테스트 | `Tests/ProductionLineTest.cpp` | 신규 |
| 테스트 | `Tests/ApprovalControllerTest.cpp` | 신규 |

> 테스트 프로젝트는 gMock(NuGet: `gmock.1.11.0`)이 이미 vcxproj에 연결되어 있다.  
> 테스트 파일은 메인 프로젝트의 vcxproj에 추가하거나 별도 테스트 프로젝트로 구성할 수 있다.  
> _DEBUG로 main.cpp안에 있는 main함수를 두 갈래로 분기한다. _DEBUG가 define이 되어 있으면 gTest를 동작하도록 구현한다.

---

## 2. 초기 더미 데이터

### Data/samples.txt

PRD 예시 데이터 5종을 `SampleStore` 직렬화 형식에 맞게 작성:

```
sample.count=5
sample.0.id=S-001
sample.0.name=실리콘 웨이퍼-8인치
sample.0.avgProdTime=0.500000
sample.0.yield=0.920000
sample.0.stock=480
sample.1.id=S-002
sample.1.name=GaN 에피택셀-4인치
sample.1.avgProdTime=0.300000
sample.1.yield=0.780000
sample.1.stock=220
sample.2.id=S-003
sample.2.name=SiC 파워기판-6인치
sample.2.avgProdTime=0.800000
sample.2.yield=0.920000
sample.2.stock=30
sample.3.id=S-004
sample.3.name=포토레지스트-PR7
sample.3.avgProdTime=0.200000
sample.3.yield=0.950000
sample.3.stock=910
sample.4.id=S-005
sample.4.name=산화막 웨이퍼-SiO2
sample.4.avgProdTime=0.600000
sample.4.yield=0.880000
sample.4.stock=0
```

이 파일을 프로젝트 루트의 `Data/` 폴더에 배치하면 프로그램 첫 실행 시 자동으로 로드된다.

---

## 3. gMock 테스트 설계

### 테스트 환경

- **프레임워크**: Google Test + Google Mock (NuGet `gmock.1.11.0`)
- **테스트 진입점**: 별도 `Tests/main_test.cpp`에 `RUN_ALL_TESTS()` 작성
- **빌드 구성**: `Debug|x64` 기준

### 테스트 파일 구조

```
Tests/
├── main_test.cpp           ← gtest 진입점
├── SampleModelTest.cpp     ← SampleModel 단위 테스트
├── OrderModelTest.cpp      ← OrderModel 단위 테스트
├── ProductionLineTest.cpp  ← ProductionLine 단위 테스트
└── ApprovalControllerTest.cpp ← ApprovalController 통합 테스트
```

---

## 4. SampleModelTest 설계

### Tests/SampleModelTest.cpp

```cpp
#include <gtest/gtest.h>
#include "../Model/SampleModel.h"

// 기본 샘플 생성 헬퍼
static Sample makeSample(const std::string& id, int stock = 100) {
    return Sample{ id, "테스트 시료", 0.5, 0.9, stock };
}
```

**테스트 케이스:**

| 테스트명 | 검증 항목 |
|---------|-----------|
| `Add_NewSample_ReturnsTrue` | 새 시료 등록 성공 → true 반환 |
| `Add_DuplicateId_ReturnsFalse` | 중복 ID 등록 → false 반환 |
| `FindById_Exists_ReturnsValue` | 존재하는 ID 조회 → optional 값 반환 |
| `FindById_NotExists_ReturnsNullopt` | 없는 ID 조회 → nullopt 반환 |
| `FindByName_MatchesSubstring` | 부분 문자열 검색 → 일치 항목 반환 |
| `FindByName_NoMatch_ReturnsEmpty` | 검색 결과 없음 → 빈 벡터 반환 |
| `UpdateStock_Positive_IncreasesStock` | 재고 증가 정상 처리 |
| `UpdateStock_NegativeBelowZero_ReturnsFalse` | 재고가 0 미만으로 내려가면 false |
| `TotalStock_SumsAllSamples` | 전체 재고 합산 정확성 |
| `Count_ReturnsCorrectCount` | 등록 시료 수 정확성 |

---

## 5. OrderModelTest 설계

### Tests/OrderModelTest.cpp

```cpp
#include <gtest/gtest.h>
#include "../Model/OrderModel.h"
```

**테스트 케이스:**

| 테스트명 | 검증 항목 |
|---------|-----------|
| `Reserve_CreatesReservedOrder` | 주문 생성 후 status == RESERVED |
| `Reserve_OrderIdFormat_IsCorrect` | 주문번호 형식 `ORD-YYYYMMDD-NNNN` 검증 |
| `Reserve_DailySeqIncrement` | 같은 날 주문 시 일련번호 증가 |
| `FindById_Exists_ReturnsOrder` | 주문번호로 단건 조회 성공 |
| `FindById_NotExists_ReturnsNullopt` | 없는 주문번호 → nullopt |
| `FindByStatus_ReturnsMatchingOrders` | 상태별 조회 정확성 |
| `UpdateStatus_ChangesStatus` | 상태 변경 정상 처리 |
| `UpdateStatus_NotFound_ReturnsFalse` | 없는 주문번호 상태 변경 → false |
| `TotalCount_ReturnsCorrectCount` | 전체 주문 수 정확성 |
| `AddDirect_RestoresOrder` | addDirect()로 주문 복원 후 조회 가능 |

---

## 6. ProductionLineTest 설계

### Tests/ProductionLineTest.cpp

```cpp
#include <gtest/gtest.h>
#include "../Model/ProductionLine.h"
```

**테스트 케이스:**

| 테스트명 | 검증 항목 |
|---------|-----------|
| `Enqueue_FirstJob_BecomesCurrentJob` | 첫 작업 → current로 즉시 배정 |
| `Enqueue_SecondJob_GoesToQueue` | 두 번째 작업 → 대기 큐에 추가 |
| `PendingCount_ReflectsQueueSize` | 대기 건수 정확성 |
| `IsRunning_WithCurrentJob_ReturnsTrue` | 작업 진행 중 → isRunning() == true |
| `IsRunning_Empty_ReturnsFalse` | 작업 없음 → isRunning() == false |
| `CompleteCurrentJob_ReturnsJob` | 완료 처리 후 완료된 job 반환 |
| `CompleteCurrentJob_PromotesNextFromQueue` | 완료 후 대기 작업이 current로 승격 |
| `CompleteCurrentJob_EmptyQueue_CurrentBecomesNullopt` | 대기 없음 → complete 후 current = nullopt |
| `PendingJobs_ReturnsFIFOOrder` | pendingJobs() 반환 순서가 FIFO 순서 |
| `ActualQty_CeilFormula_IsCorrect` | `ceil(부족분 / (수율 × 0.9))` 계산 검증 |

**생산량 공식 검증 예시:**
```cpp
TEST(ProductionLineTest, ActualQty_CeilFormula_IsCorrect) {
    // 부족분=170, 수율=0.92 → ceil(170/(0.92*0.9)) = ceil(170/0.828) = ceil(205.3) = 206
    double yield    = 0.92;
    int    shortage = 170;
    int    expected = 206;
    int    actual   = static_cast<int>(
                        std::ceil(static_cast<double>(shortage) / (yield * 0.9)));
    EXPECT_EQ(actual, expected);
}
```

---

## 7. ApprovalControllerTest 설계

`ApprovalController`는 `OrderModel`, `SampleModel`, `ProductionLine`, `OrderView`에 의존한다.  
gMock으로 `OrderView`를 Mock하고, 나머지는 실제 객체를 사용한다.

### Tests/ApprovalControllerTest.cpp

```cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../Controller/ApprovalController.h"
#include "../Model/OrderModel.h"
#include "../Model/SampleModel.h"
#include "../Model/ProductionLine.h"
#include "../View/OrderView.h"

class MockOrderView : public OrderView {
public:
    MOCK_METHOD(Order, promptOrderInput,
                (std::span<const Sample>), (const, override));
    MOCK_METHOD(void, showOrderResult, (const Order&), (const, override));
    MOCK_METHOD(void, showReservedList,
                (const std::vector<Order>&, std::span<const Sample>),
                (const, override));
    MOCK_METHOD(void, showMessage, (const std::string&), (const, override));
    MOCK_METHOD(void, showError,   (const std::string&), (const, override));
    MOCK_METHOD(int,  promptMenuChoice, (), (const, override));
};
```

> `OrderView`의 메서드들을 `virtual`로 선언해야 Mock 가능.  
> Phase 9 구현 전 `OrderView.h`에 `virtual` 키워드 추가가 필요하다.

**테스트 케이스:**

| 테스트명 | 검증 항목 |
|---------|-----------|
| `Approve_SufficientStock_StatusBecomesConfirmed` | 재고 충분 → CONFIRMED 전환 + 재고 차감 |
| `Approve_InsufficientStock_StatusBecomesProducing` | 재고 부족 → PRODUCING 전환 + 생산라인 등록 |
| `Approve_EffectiveStock_DeductsConfirmedOrders` | 유효 재고 계산: CONFIRMED 대기분 차감 |
| `Reject_StatusBecomesRejected` | 거절 → REJECTED 즉시 전환 |
| `CalcEffectiveStock_NoConfirmed_EqualsStock` | CONFIRMED 없으면 유효 재고 = stock |
| `CalcEffectiveStock_WithConfirmed_DeductsAmount` | CONFIRMED 있으면 유효 재고 차감 확인 |

---

## 8. Tests/main_test.cpp

```cpp
#include <gtest/gtest.h>

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
```

---

## 9. OrderView virtual 선언 추가 (ApprovalControllerTest 선행 조건)

`OrderView.h`의 메서드들을 Mock 가능하도록 `virtual` 추가:

```cpp
class OrderView {
public:
    virtual ~OrderView() = default;
    virtual Order promptOrderInput(std::span<const Sample> samples) const;
    virtual void  showOrderResult(const Order& order) const;
    virtual void  showReservedList(const std::vector<Order>& orders,
                                   std::span<const Sample>   samples) const;
    virtual void  showMessage(const std::string& msg) const;
    virtual void  showError(const std::string& msg)   const;
    virtual int   promptMenuChoice()                  const;
};
```

---

## 10. Phase 9 완료 조건

- [ ] `Data/samples.txt` PRD 예시 시료 5종 작성 완료
- [ ] 프로그램 첫 실행 시 시료 5종 자동 로드 확인
- [ ] `OrderView` 메서드에 `virtual` 추가 완료
- [ ] `SampleModelTest`: 10개 테스트 케이스 모두 통과
- [ ] `OrderModelTest`: 10개 테스트 케이스 모두 통과
- [ ] `ProductionLineTest`: 10개 테스트 케이스 모두 통과
- [ ] `ApprovalControllerTest`: 6개 테스트 케이스 모두 통과
- [ ] 모든 테스트 빌드 성공 및 실패 없음
- [ ] 사용자 검토 완료 후 commit
