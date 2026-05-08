# Step 1 — 테스트 커버리지 보강 설계

> 참고: [PLAN.md](../PLAN.md)  
> 브랜치: `refactoring`  
> 구현 전 **사용자 검토 후 commit** 규칙 준수

---

## 목표

이후 Step 2~9의 리팩토링 과정에서 회귀(regression)를 즉시 감지할 수 있도록  
현재 커버리지가 부족한 영역의 단위 테스트를 먼저 작성한다.  
**동작 변경 없이 테스트 파일만 추가/수정한다.**

---

## 현재 테스트 현황 분석

| 대상 | 파일 | 현재 상태 |
|------|------|-----------|
| SampleModel | `Tests/SampleModelTest.cpp` | ✅ 10개 케이스, 충분 |
| OrderModel | `Tests/OrderModelTest.cpp` | ✅ 10개 케이스, 충분 |
| ProductionLine | `Tests/ProductionLineTest.cpp` | ✅ 10개 케이스, 충분 |
| ApprovalController | `Tests/ApprovalControllerTest.cpp` | ⚠️ 6개 케이스, MockView 미사용, 경계값 부족 |
| DataStore | 없음 | ❌ 미작성 |
| SampleStore | 없음 | ❌ 미작성 |
| OrderStore | 없음 | ❌ 미작성 |
| ProductionController | 없음 | ❌ 미작성 |
| ReleaseController | 없음 | ❌ 미작성 |

---

## 생성/수정 파일 목록

| 구분 | 파일 | 작업 |
|------|------|------|
| 신규 | `Tests/DataStoreTest.cpp` | DataStore 단위 테스트 |
| 신규 | `Tests/SampleStoreTest.cpp` | SampleStore 라운드트립 테스트 |
| 신규 | `Tests/OrderStoreTest.cpp` | OrderStore 라운드트립 테스트 |
| 수정 | `Tests/ApprovalControllerTest.cpp` | 경계값 케이스 3개 추가 |
| 신규 | `Tests/ProductionControllerTest.cpp` | 비즈니스 로직 테스트 |
| 신규 | `Tests/ReleaseControllerTest.cpp` | 비즈니스 로직 테스트 |
| 수정 | `SampleOrderSystem_project.vcxproj` | 신규 파일 ClCompile 등록 |
| 수정 | `SampleOrderSystem_project.vcxproj.filters` | 소스 파일 필터 등록 |

---

## Step 1-1. DataStoreTest

### 설계 방향

- 실제 파일 I/O를 테스트하므로 임시 파일 경로 사용
- 각 테스트 후 `TearDown()`에서 파일 삭제
- TXT 포맷 기준 (JSON은 내부 구현 세부사항)

### Tests/DataStoreTest.cpp

```cpp
#include <gtest/gtest.h>
#include <filesystem>
#include "../Data/DataStore.h"

class DataStoreTest : public ::testing::Test {
protected:
    const std::string testFile_ = "test_datastore_tmp.txt";

    void TearDown() override {
        std::filesystem::remove(testFile_);
    }
};
```

**테스트 케이스:**

| 테스트명 | 검증 항목 |
|---------|-----------|
| `SetAndGet_StoresValue` | set 후 get으로 동일 값 반환 |
| `Get_MissingKey_ReturnsDefault` | 없는 키 조회 시 defaultValue 반환 |
| `Has_ExistingKey_ReturnsTrue` | 존재하는 키 → true |
| `Has_MissingKey_ReturnsFalse` | 없는 키 → false |
| `Remove_DeletesKey` | remove 후 has() == false |
| `Keys_ReturnsAllKeys` | 등록한 키 목록 전부 반환 |
| `SaveAndLoad_TXT_RoundTrip` | save 후 새 인스턴스로 load → 동일 값 |
| `Load_NonExistentFile_EmptyStore` | 없는 파일 load → 데이터 없음 (예외 없음) |
| `SaveAndLoad_OverwritesExisting` | 두 번 save 시 최신 값으로 덮어씀 |

**구현 명세 예시:**

```cpp
TEST_F(DataStoreTest, SaveAndLoad_TXT_RoundTrip) {
    {
        DataStore store(testFile_);
        store.set("key1", "value1");
        store.set("key2", "value2");
        store.save();
    }
    DataStore store2(testFile_);
    store2.load();
    EXPECT_EQ(store2.get("key1"), "value1");
    EXPECT_EQ(store2.get("key2"), "value2");
}

TEST_F(DataStoreTest, Load_NonExistentFile_EmptyStore) {
    DataStore store("nonexistent_file_xyz.txt");
    EXPECT_NO_THROW(store.load());
    EXPECT_TRUE(store.keys().empty());
}
```

---

## Step 1-2. SampleStoreTest

### 설계 방향

- `SampleStore`의 직렬화 키 규칙(`sample.N.id` 등)의 정합성 검증
- PRD 예시 시료 5종을 저장 후 복원 → 동일 여부 확인
- 재고 변경 후 저장 → 복원 시 변경된 재고 반영 여부 확인

### Tests/SampleStoreTest.cpp

```cpp
#include <gtest/gtest.h>
#include <filesystem>
#include "../Data/SampleStore.h"
#include "../Model/SampleModel.h"

class SampleStoreTest : public ::testing::Test {
protected:
    const std::string testFile_ = "test_samplestore_tmp.txt";

    void TearDown() override {
        std::filesystem::remove(testFile_);
    }

    // PRD 예시 시료 5종 등록 헬퍼
    void populateModel(SampleModel& model) {
        model.add({ "S-001", "실리콘 웨이퍼-8인치",  0.5, 0.92, 480 });
        model.add({ "S-002", "GaN 에피택셀-4인치",   0.3, 0.78, 220 });
        model.add({ "S-003", "SiC 파워기판-6인치",   0.8, 0.92,  30 });
        model.add({ "S-004", "포토레지스트-PR7",     0.2, 0.95, 910 });
        model.add({ "S-005", "산화막 웨이퍼-SiO2",  0.6, 0.88,   0 });
    }
};
```

**테스트 케이스:**

| 테스트명 | 검증 항목 |
|---------|-----------|
| `Save_And_Load_Count_Matches` | 저장 5종 → 복원 후 count() == 5 |
| `Save_And_Load_Id_Matches` | 각 시료 ID 정확히 복원 |
| `Save_And_Load_AllFields_Match` | name / avgProdTime / yield / stock 전부 일치 |
| `Save_UpdatedStock_Persists` | 재고 변경 후 저장 → 복원 시 변경값 반영 |
| `Load_EmptyFile_EmptyModel` | 빈 파일 load → model.count() == 0 |
| `Load_NoFile_EmptyModel` | 파일 없음 → model.count() == 0 (예외 없음) |

---

## Step 1-3. OrderStoreTest

### 설계 방향

- `OrderStore`의 주문 직렬화 + `OrderStatus` 5종 모두 검증
- 채번 상태(`lastDate_`, `dailySeq_`) 복원 후 연속 채번 정합성 확인
- `addDirect()`로 복원된 주문 목록이 `findByStatus()`로 정상 조회되는지 확인

### Tests/OrderStoreTest.cpp

```cpp
#include <gtest/gtest.h>
#include <filesystem>
#include "../Data/OrderStore.h"
#include "../Model/OrderModel.h"

class OrderStoreTest : public ::testing::Test {
protected:
    const std::string testFile_ = "test_orderstore_tmp.txt";

    void TearDown() override {
        std::filesystem::remove(testFile_);
    }
};
```

**테스트 케이스:**

| 테스트명 | 검증 항목 |
|---------|-----------|
| `Save_And_Load_Count_Matches` | 저장한 주문 수와 복원 수 일치 |
| `Save_And_Load_AllFields_Match` | orderId / sampleId / customer / quantity / createdAt 일치 |
| `Save_And_Load_Status_Reserved` | RESERVED 상태 직렬화/역직렬화 |
| `Save_And_Load_Status_Confirmed` | CONFIRMED 상태 직렬화/역직렬화 |
| `Save_And_Load_Status_Producing` | PRODUCING 상태 직렬화/역직렬화 |
| `Save_And_Load_Status_Released` | RELEASED 상태 직렬화/역직렬화 |
| `Save_And_Load_Status_Rejected` | REJECTED 상태 직렬화/역직렬화 |
| `Save_DailySeq_RestoredCorrectly` | lastDate / dailySeq 복원 후 다음 채번이 연속됨 |
| `Load_NoFile_EmptyModel` | 파일 없음 → model 비어있음 (예외 없음) |

**채번 연속성 검증 예시:**

```cpp
TEST_F(OrderStoreTest, Save_DailySeq_RestoredCorrectly) {
    std::string firstId;
    {
        OrderModel m;
        m.reserve("S-001", "고객A", 10);
        m.reserve("S-001", "고객B", 20);  // dailySeq = 2
        firstId = m.all()[0].orderId;
        OrderStore store(testFile_);
        store.save(m);
    }
    OrderModel m2;
    OrderStore store2(testFile_);
    store2.load(m2);

    // 복원 후 추가 주문 시 일련번호가 3번부터 시작해야 함
    Order next = m2.reserve("S-001", "고객C", 30);
    std::string seq = next.orderId.substr(13);  // "NNNN" 부분
    EXPECT_EQ(seq, "0003");
}
```

---

## Step 1-4. ApprovalControllerTest 보완

### 현재 문제점

1. `MockOrderView`가 선언만 되고 실제 테스트에서 사용되지 않음
2. 경계값 테스트 부족:
   - 여러 CONFIRMED 주문이 동일 시료에 쌓였을 때 합산 차감 여부
   - 다른 시료의 CONFIRMED 주문이 영향을 주지 않는지 여부

### 추가할 테스트 케이스

```cpp
// 동일 시료 CONFIRMED 여러 건 → 합산 차감
TEST_F(ApprovalControllerTest,
       CalcEffectiveStock_MultipleConfirmed_SumDeducted) {
    // S-001: stock=100, CONFIRMED 30 + 20 = 50 → 유효재고 = 50
    Order o1 = orderModel_.reserve("S-001", "고객A", 30);
    Order o2 = orderModel_.reserve("S-001", "고객B", 20);
    orderModel_.updateStatus(o1.orderId, OrderStatus::CONFIRMED);
    orderModel_.updateStatus(o2.orderId, OrderStatus::CONFIRMED);

    int effectiveStock = sampleModel_.findById("S-001")->stock;
    for (const auto& o : orderModel_.findByStatus(OrderStatus::CONFIRMED))
        if (o.sampleId == "S-001") effectiveStock -= o.quantity;
    EXPECT_EQ(effectiveStock, 50);
}

// 다른 시료의 CONFIRMED는 영향 없음
TEST_F(ApprovalControllerTest,
       CalcEffectiveStock_DifferentSample_NotAffected) {
    // S-002 시료의 CONFIRMED가 S-001 유효재고에 영향 없어야 함
    sampleModel_.add(Sample{ "S-002", "다른 시료", 0.3, 0.8, 200 });
    Order o = orderModel_.reserve("S-002", "고객A", 50);
    orderModel_.updateStatus(o.orderId, OrderStatus::CONFIRMED);

    int effectiveStock = sampleModel_.findById("S-001")->stock;
    for (const auto& conf : orderModel_.findByStatus(OrderStatus::CONFIRMED))
        if (conf.sampleId == "S-001") effectiveStock -= conf.quantity;
    EXPECT_EQ(effectiveStock, 100);  // S-001 재고 그대로
}

// stock이 음수가 되는 극단값 (CONFIRMED 합이 stock 초과)
TEST_F(ApprovalControllerTest,
       CalcEffectiveStock_OverCommitted_ReturnsNegative) {
    // S-001: stock=100, CONFIRMED 120 → 유효재고 = -20
    Order o = orderModel_.reserve("S-001", "고객A", 120);
    orderModel_.updateStatus(o.orderId, OrderStatus::CONFIRMED);

    int effectiveStock = sampleModel_.findById("S-001")->stock;
    for (const auto& conf : orderModel_.findByStatus(OrderStatus::CONFIRMED))
        if (conf.sampleId == "S-001") effectiveStock -= conf.quantity;
    EXPECT_LT(effectiveStock, 0);
}
```

---

## Step 1-5. ProductionControllerTest

### 설계 방향

`ProductionController::handleComplete()`는 `std::cin`을 직접 사용하므로  
View를 Mock하지 않고는 완전한 Controller 테스트가 어렵다.  
**Step 1에서는 View 없이 검증 가능한 비즈니스 로직(재고 보충, 상태 전환)만 테스트한다.**  
View Mock 기반 완전 테스트는 Step 6(View virtual 일관화) 이후에 추가한다.

### Tests/ProductionControllerTest.cpp

```cpp
#include <gtest/gtest.h>
#include "../Model/SampleModel.h"
#include "../Model/OrderModel.h"
#include "../Model/ProductionLine.h"

class ProductionControllerTest : public ::testing::Test {
protected:
    SampleModel    sampleModel_;
    OrderModel     orderModel_;
    ProductionLine productionLine_;

    void SetUp() override {
        sampleModel_.add(Sample{ "S-001", "테스트 시료", 0.5, 0.9, 0 });
    }
};
```

**테스트 케이스:**

| 테스트명 | 검증 항목 |
|---------|-----------|
| `Complete_NoCurrentJob_NothingChanges` | 처리 중 작업 없을 때 모델 상태 불변 |
| `Complete_Job_StockIncreasesByActualQty` | 완료 시 재고가 actualQty만큼 증가 |
| `Complete_Job_StatusBecomesConfirmed` | 완료 시 주문 상태 → CONFIRMED |
| `Complete_Job_ProductionLineAdvances` | 완료 후 다음 대기 작업이 current로 승격 |
| `Complete_LastJob_ProductionLineBecomesIdle` | 마지막 작업 완료 후 isRunning() == false |

**구현 명세 예시:**

```cpp
TEST_F(ProductionControllerTest, Complete_Job_StockIncreasesByActualQty) {
    Order o = orderModel_.reserve("S-001", "고객A", 50);
    orderModel_.updateStatus(o.orderId, OrderStatus::PRODUCING);

    ProductionJob job{ o.orderId, "S-001", 50, 62, 31.0 };
    productionLine_.enqueue(job);

    // 완료 처리 로직 직접 실행 (View 없이)
    auto current = productionLine_.currentJob();
    ASSERT_TRUE(current.has_value());

    sampleModel_.updateStock(current->sampleId, current->actualQty);
    orderModel_.updateStatus(current->orderId, OrderStatus::CONFIRMED);
    productionLine_.completeCurrentJob();

    EXPECT_EQ(sampleModel_.findById("S-001")->stock, 62);
    EXPECT_EQ(orderModel_.findById(o.orderId)->status, OrderStatus::CONFIRMED);
    EXPECT_FALSE(productionLine_.isRunning());
}
```

---

## Step 1-6. ReleaseControllerTest

### 설계 방향

`ReleaseController::run()`도 `std::cin`을 직접 사용하므로  
**Step 1에서는 출고 핵심 비즈니스 로직만 직접 테스트한다.**  
View Mock 기반 완전 테스트는 Step 6 이후에 추가한다.

### Tests/ReleaseControllerTest.cpp

```cpp
#include <gtest/gtest.h>
#include "../Model/OrderModel.h"
#include "../Model/SampleModel.h"

class ReleaseControllerTest : public ::testing::Test {
protected:
    SampleModel sampleModel_;
    OrderModel  orderModel_;

    void SetUp() override {
        sampleModel_.add(Sample{ "S-001", "테스트 시료", 0.5, 0.9, 100 });
    }
};
```

**테스트 케이스:**

| 테스트명 | 검증 항목 |
|---------|-----------|
| `Release_ChangesStatusToReleased` | CONFIRMED → RELEASED 상태 전환 |
| `Release_DoesNotChangeStock` | 출고 후 재고 변화 없음 |
| `Release_OnlyConfirmedOrders_AreReleasable` | RESERVED/PRODUCING 주문은 CONFIRMED 목록에 없음 |
| `Release_MultipleConfirmed_EachReleasableIndependently` | 여러 CONFIRMED 주문 각각 독립 출고 가능 |

**구현 명세 예시:**

```cpp
TEST_F(ReleaseControllerTest, Release_DoesNotChangeStock) {
    // 승인 시점에 재고 차감이 이미 이루어졌다고 가정
    sampleModel_.updateStock("S-001", -30);  // 재고: 70
    Order o = orderModel_.reserve("S-001", "고객A", 30);
    orderModel_.updateStatus(o.orderId, OrderStatus::CONFIRMED);

    int stockBefore = sampleModel_.findById("S-001")->stock;

    // 출고 처리: 상태만 변경, 재고 차감 없음
    orderModel_.updateStatus(o.orderId, OrderStatus::RELEASED);

    EXPECT_EQ(sampleModel_.findById("S-001")->stock, stockBefore);
}
```

---

## vcxproj 등록

### ClCompile 추가 (vcxproj)

```xml
<ClCompile Include="Tests\DataStoreTest.cpp" />
<ClCompile Include="Tests\SampleStoreTest.cpp" />
<ClCompile Include="Tests\OrderStoreTest.cpp" />
<ClCompile Include="Tests\ProductionControllerTest.cpp" />
<ClCompile Include="Tests\ReleaseControllerTest.cpp" />
```

### vcxproj.filters 추가

```xml
<ClCompile Include="Tests\DataStoreTest.cpp"><Filter>소스 파일</Filter></ClCompile>
<ClCompile Include="Tests\SampleStoreTest.cpp"><Filter>소스 파일</Filter></ClCompile>
<ClCompile Include="Tests\OrderStoreTest.cpp"><Filter>소스 파일</Filter></ClCompile>
<ClCompile Include="Tests\ProductionControllerTest.cpp"><Filter>소스 파일</Filter></ClCompile>
<ClCompile Include="Tests\ReleaseControllerTest.cpp"><Filter>소스 파일</Filter></ClCompile>
```

---

## 제약 사항 및 주의점

| 항목 | 내용 |
|------|------|
| 임시 파일 정리 | DataStore/SampleStore/OrderStore 테스트는 `TearDown()`에서 반드시 임시 파일 삭제 |
| Controller 완전 테스트 제한 | View virtual화(Step 6) 전까지 Controller의 View 의존 로직은 테스트 범위 외 |
| `std::cin` 직접 사용 제한 | ProductionController/ReleaseController의 사용자 입력 부분은 Step 6 이후 Mock으로 테스트 |
| 파일 경로 | 테스트용 임시 파일은 실행 디렉터리 기준 상대 경로 사용 (프로젝트 폴더 내 생성) |

---

## Step 1 완료 조건

- [ ] `DataStoreTest`: 9개 케이스 모두 통과
- [ ] `SampleStoreTest`: 6개 케이스 모두 통과
- [ ] `OrderStoreTest`: 9개 케이스 모두 통과
- [ ] `ApprovalControllerTest`: 기존 6개 + 추가 3개 = 9개 모두 통과
- [ ] `ProductionControllerTest`: 5개 케이스 모두 통과
- [ ] `ReleaseControllerTest`: 4개 케이스 모두 통과
- [ ] 기존 테스트(SampleModel/OrderModel/ProductionLine) 전부 계속 통과
- [ ] 사용자 검토 완료 후 commit
