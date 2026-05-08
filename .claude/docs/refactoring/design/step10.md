# Step 10 — 테스트 최종 점검 설계

> 참고: [PLAN.md](../PLAN.md) | 브랜치: `refactoring`  
> 구현 전 **사용자 검토 후 commit** 규칙 준수

---

## 목표

Step 2~9 리팩토링으로 새로 추가되거나 변경된 로직에 대한 테스트를 보완한다.  
전체 테스트가 통과함을 확인하고 리팩토링을 마무리한다.

---

## 추가할 테스트 목록

### Tests/TimeUtilsTest.cpp (신규)

`Utils/TimeUtils`의 포맷 정확성 검증:

| 테스트명 | 검증 항목 |
|---------|-----------|
| `NowDateTime_FormatIsCorrect` | "YYYY-MM-DD HH:MM:SS" 형식 (길이 19, 하이픈/콜론 위치) |
| `NowDate_FormatIsCorrect` | "YYYYMMDD" 형식 (길이 8, 숫자만) |

```cpp
TEST(TimeUtilsTest, NowDateTime_FormatIsCorrect) {
    std::string dt = TimeUtils::nowDateTime();
    EXPECT_EQ(dt.size(), 19u);    // "YYYY-MM-DD HH:MM:SS"
    EXPECT_EQ(dt[4],  '-');
    EXPECT_EQ(dt[7],  '-');
    EXPECT_EQ(dt[10], ' ');
    EXPECT_EQ(dt[13], ':');
    EXPECT_EQ(dt[16], ':');
}

TEST(TimeUtilsTest, NowDate_FormatIsCorrect) {
    std::string d = TimeUtils::nowDate();
    EXPECT_EQ(d.size(), 8u);     // "YYYYMMDD"
    for (char c : d) EXPECT_TRUE(std::isdigit(c));
}
```

---

### Tests/OrderStatusUtilsTest.cpp (신규)

`Model/Order.h`의 `orderStatusToStr()` / `strToOrderStatus()` 양방향 검증:

| 테스트명 | 검증 항목 |
|---------|-----------|
| `StatusToStr_AllValues` | 5개 상태값 → 올바른 문자열 |
| `StrToStatus_AllValues` | 5개 문자열 → 올바른 enum 값 |
| `StrToStatus_Unknown_ReturnsReserved` | 알 수 없는 문자열 → RESERVED (기본값) |
| `RoundTrip_AllValues` | str→enum→str 변환 후 원래 값과 동일 |

```cpp
TEST(OrderStatusUtilsTest, StatusToStr_AllValues) {
    EXPECT_EQ(orderStatusToStr(OrderStatus::RESERVED),  "RESERVED");
    EXPECT_EQ(orderStatusToStr(OrderStatus::REJECTED),  "REJECTED");
    EXPECT_EQ(orderStatusToStr(OrderStatus::PRODUCING), "PRODUCING");
    EXPECT_EQ(orderStatusToStr(OrderStatus::CONFIRMED), "CONFIRMED");
    EXPECT_EQ(orderStatusToStr(OrderStatus::RELEASED),  "RELEASED");
}

TEST(OrderStatusUtilsTest, StrToStatus_AllValues) {
    EXPECT_EQ(strToOrderStatus("RESERVED"),  OrderStatus::RESERVED);
    EXPECT_EQ(strToOrderStatus("REJECTED"),  OrderStatus::REJECTED);
    EXPECT_EQ(strToOrderStatus("PRODUCING"), OrderStatus::PRODUCING);
    EXPECT_EQ(strToOrderStatus("CONFIRMED"), OrderStatus::CONFIRMED);
    EXPECT_EQ(strToOrderStatus("RELEASED"),  OrderStatus::RELEASED);
}

TEST(OrderStatusUtilsTest, StrToStatus_Unknown_ReturnsReserved) {
    EXPECT_EQ(strToOrderStatus("UNKNOWN"), OrderStatus::RESERVED);
    EXPECT_EQ(strToOrderStatus(""),        OrderStatus::RESERVED);
}

TEST(OrderStatusUtilsTest, RoundTrip_AllValues) {
    for (auto status : { OrderStatus::RESERVED, OrderStatus::REJECTED,
                         OrderStatus::PRODUCING, OrderStatus::CONFIRMED,
                         OrderStatus::RELEASED }) {
        EXPECT_EQ(strToOrderStatus(orderStatusToStr(status)), status);
    }
}
```

---

### Tests/SampleModelTest.cpp 보완

Step 8에서 `SampleModel`에 `effectiveStock()` / `stockStatus()`가 추가되었으므로 테스트 추가:

| 테스트명 | 검증 항목 |
|---------|-----------|
| `EffectiveStock_NoConfirmed_EqualsStock` | CONFIRMED 없으면 유효재고 = stock |
| `EffectiveStock_WithConfirmed_Deducts` | CONFIRMED 수량 차감 |
| `EffectiveStock_DifferentSample_NotAffected` | 다른 시료 CONFIRMED 영향 없음 |
| `StockStatus_ZeroStock_IsDepleted` | stock==0 → DEPLETED |
| `StockStatus_SufficientStock_IsSurplus` | 수요 < stock → SURPLUS |
| `StockStatus_InsufficientDemand_IsShort` | 수요 > stock → SHORT |

---

## vcxproj 등록

```xml
<ClCompile Include="Tests\TimeUtilsTest.cpp" />
<ClCompile Include="Tests\OrderStatusUtilsTest.cpp" />
```

---

## 전체 테스트 통과 확인 체크리스트

| 테스트 파일 | 케이스 수 |
|------------|-----------|
| SampleModelTest | 10 + 6 = 16개 |
| OrderModelTest | 10개 |
| ProductionLineTest | 10개 |
| ApprovalControllerTest | 6 + 3 = 9개 |
| DataStoreTest | 9개 |
| SampleStoreTest | 6개 |
| OrderStoreTest | 8개 |
| ProductionControllerTest | 5개 |
| ReleaseControllerTest | 4개 |
| TimeUtilsTest | 2개 |
| OrderStatusUtilsTest | 4개 |
| **합계** | **83개** |

---

## Step 10 완료 조건

- [ ] `Tests/TimeUtilsTest.cpp` 추가 및 통과
- [ ] `Tests/OrderStatusUtilsTest.cpp` 추가 및 통과
- [ ] `Tests/SampleModelTest.cpp` effectiveStock/stockStatus 6개 케이스 추가 및 통과
- [ ] 전체 83개 테스트 통과
- [ ] 빌드 성공 (경고 없음)
- [ ] 사용자 검토 완료 후 commit
