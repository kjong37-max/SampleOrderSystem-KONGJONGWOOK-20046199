# Phase 8 — 메인 통합 & 데이터 영속성 설계

> 참고: [PLAN.md](../PLAN.md) | [PRD.md](../PRD.md)  
> 구현 전 **사용자 검토 후 commit** 규칙 준수

---

## 목표

1. **MainController**: 모든 Model·View·Controller를 통합하는 메인 메뉴 루프 구현  
2. **데이터 영속성**: 시작 시 파일 복원, 종료 시 파일 저장 (DataPersistence 참고 저장소 기반)

---

## 1. 생성 및 수정 파일 목록

| 구분 | 파일 | 작업 |
|------|------|------|
| Data | `Data/DataStore.h` | 수정 (빈 선언 → 실제 선언, 참고 저장소 기반) |
| Data | `Data/DataStore.cpp` | 신규 |
| Data | `Data/SampleStore.h` | 신규 |
| Data | `Data/SampleStore.cpp` | 신규 |
| Data | `Data/OrderStore.h` | 신규 |
| Data | `Data/OrderStore.cpp` | 신규 |
| Controller | `Controller/MainController.h` | 수정 (기존 선언 확장) |
| Controller | `Controller/MainController.cpp` | 수정 (빈 run() → 실제 구현) |
| main.cpp | `main.cpp` | 수정 (빈 main → 실제 구현) |
| vcxproj | `SampleOrderSystem_project.vcxproj` | ClCompile 4개, ClInclude 2개 추가 |
| vcxproj.filters | `SampleOrderSystem_project.vcxproj.filters` | 필터 항목 추가 |

> `Data/RecordStore.h`는 이번 Phase에서 사용하지 않는다.  
> Sample·Order 전용 Store 클래스(`SampleStore`, `OrderStore`)로 직접 직렬화한다.

---

## 2. DataStore 설계 (DataPersistence 참고 저장소 그대로 적용)

### Data/DataStore.h

DataPersistence 참고 저장소의 `DataStore.h` 와 동일하게 구현.

```cpp
#pragma once
#include <string>
#include <unordered_map>
#include <vector>

enum class StoreFormat { TXT, JSON };

class DataStore {
public:
    explicit DataStore(const std::string& filePath);

    void load();
    void save() const;

    void                     set(const std::string& key, const std::string& value);
    std::string              get(const std::string& key,
                                 const std::string& defaultValue = "") const;
    bool                     has(const std::string& key) const;
    void                     remove(const std::string& key);
    std::vector<std::string> keys() const;
    StoreFormat              format() const;

private:
    std::string                                  m_filePath;
    StoreFormat                                  m_format;
    std::unordered_map<std::string, std::string> m_data;

    void loadTxt();
    void loadJson();
    void saveTxt()  const;
    void saveJson() const;

    static StoreFormat detectFormat(const std::string& filePath);
    static std::string jsonEscape(const std::string& s);
};
```

### Data/DataStore.cpp

DataPersistence 참고 저장소의 `DataStore.cpp` 와 동일하게 구현한다.  
(TXT: `key=value` 한 줄씩 / JSON: 단순 flat key-value)

---

## 3. SampleStore 설계

Sample 데이터를 DataStore에 직렬화/역직렬화한다.

### 직렬화 키 규칙

```
sample.count       = "5"
sample.0.id        = "S-001"
sample.0.name      = "실리콘 웨이퍼-8인치"
sample.0.avgProdTime = "0.5"
sample.0.yield     = "0.92"
sample.0.stock     = "480"
sample.1.id        = "S-002"
...
```

### Data/SampleStore.h

```cpp
#pragma once
#include "DataStore.h"
#include "../Model/SampleModel.h"

class SampleStore {
public:
    explicit SampleStore(const std::string& filePath);

    void load(SampleModel& model);
    void save(const SampleModel& model) const;

private:
    DataStore store_;
};
```

### Data/SampleStore.cpp — 구현 명세

**`load()`**:
```
store_.load();
int count = stoi(store_.get("sample.count", "0"));
for (int i = 0; i < count; ++i) {
    Sample s;
    s.id          = store_.get("sample." + i + ".id");
    s.name        = store_.get("sample." + i + ".name");
    s.avgProdTime = stod(store_.get("sample." + i + ".avgProdTime", "0"));
    s.yield       = stod(store_.get("sample." + i + ".yield", "0"));
    s.stock       = stoi(store_.get("sample." + i + ".stock", "0"));
    model.add(s);
}
```

**`save()`**:
```
DataStore store(filePath);
auto samples = model.all();  // std::span<const Sample>
store.set("sample.count", to_string(samples.size()));
for (int i = 0; i < samples.size(); ++i) {
    string prefix = "sample." + to_string(i);
    store.set(prefix + ".id",          samples[i].id);
    store.set(prefix + ".name",        samples[i].name);
    store.set(prefix + ".avgProdTime", to_string(samples[i].avgProdTime));
    store.set(prefix + ".yield",       to_string(samples[i].yield));
    store.set(prefix + ".stock",       to_string(samples[i].stock));
}
store.save();
```

---

## 4. OrderStore 설계

Order 데이터 및 OrderModel의 채번 상태(lastDate_, dailySeq_)를 함께 저장한다.

### 직렬화 키 규칙

```
order.count      = "36"
order.lastDate   = "20260416"
order.dailySeq   = "43"
order.0.orderId   = "ORD-20260416-0001"
order.0.sampleId  = "S-001"
order.0.customer  = "삼성전자 파운드리"
order.0.quantity  = "200"
order.0.status    = "RESERVED"
order.0.createdAt = "2026-04-16 09:32:15"
order.1.orderId   = "ORD-20260416-0002"
...
```

### OrderStatus 직렬화 변환

```cpp
// to string
std::string statusToStr(OrderStatus s) {
    switch (s) {
    case OrderStatus::RESERVED:  return "RESERVED";
    case OrderStatus::REJECTED:  return "REJECTED";
    case OrderStatus::PRODUCING: return "PRODUCING";
    case OrderStatus::CONFIRMED: return "CONFIRMED";
    case OrderStatus::RELEASED:  return "RELEASED";
    }
    return "RESERVED";
}

// from string
OrderStatus strToStatus(const std::string& s) {
    if (s == "REJECTED")  return OrderStatus::REJECTED;
    if (s == "PRODUCING") return OrderStatus::PRODUCING;
    if (s == "CONFIRMED") return OrderStatus::CONFIRMED;
    if (s == "RELEASED")  return OrderStatus::RELEASED;
    return OrderStatus::RESERVED;
}
```

### Data/OrderStore.h

```cpp
#pragma once
#include "DataStore.h"
#include "../Model/OrderModel.h"

class OrderStore {
public:
    explicit OrderStore(const std::string& filePath);

    void load(OrderModel& model);
    void save(const OrderModel& model) const;

private:
    DataStore store_;
};
```

> `OrderModel`의 `lastDate_`, `dailySeq_`를 load/save하려면  
> `OrderModel`에 getter/setter 추가가 필요하다:
> ```cpp
> // OrderModel.h 추가
> std::string lastDate() const;
> int         dailySeq() const;
> void        setLastDate(const std::string& d);
> void        setDailySeq(int seq);
> ```

---

## 5. MainController 설계

### Controller/MainController.h (수정)

```cpp
#pragma once
#include "../Model/SampleModel.h"
#include "../Model/OrderModel.h"
#include "../Model/ProductionLine.h"
#include "../View/SampleView.h"
#include "../View/OrderView.h"
#include "../View/ProductionView.h"
#include "../View/MonitorView.h"
#include "../Controller/SampleController.h"
#include "../Controller/OrderController.h"
#include "../Controller/ApprovalController.h"
#include "../Controller/ProductionController.h"
#include "../Controller/ReleaseController.h"
#include "../Controller/MonitorController.h"
#include "../Data/SampleStore.h"
#include "../Data/OrderStore.h"
#include <string>

class MainController {
public:
    void run();

private:
    void loadData();
    void saveData() const;
    void showMainMenu() const;
    void showSummary()  const;

    // Models
    SampleModel    sampleModel_;
    OrderModel     orderModel_;
    ProductionLine productionLine_;

    // Views
    SampleView     sampleView_;
    OrderView      orderView_;
    ProductionView productionView_;
    MonitorView    monitorView_;

    // Sub-Controllers
    SampleController     sampleCtrl_{ sampleModel_, sampleView_ };
    OrderController      orderCtrl_{ orderModel_, sampleModel_, orderView_ };
    ApprovalController   approvalCtrl_{ orderModel_, sampleModel_,
                                        productionLine_, orderView_ };
    ProductionController productionCtrl_{ orderModel_, sampleModel_,
                                          productionLine_, productionView_ };
    ReleaseController    releaseCtrl_{ orderModel_, sampleModel_, orderView_ };
    MonitorController    monitorCtrl_{ orderModel_, sampleModel_, monitorView_ };

    // Stores (파일 경로)
    static constexpr const char* SAMPLE_FILE = "Data/samples.txt";
    static constexpr const char* ORDER_FILE  = "Data/orders.txt";
};
```

### Controller/MainController.cpp — 흐름 명세

**`run()`**:
```cpp
loadData();

bool running = true;
while (running) {
    showMainMenu();
    // 메뉴 선택 입력
    std::string line; std::getline(std::cin, line);
    int choice = -1;
    try { choice = std::stoi(line); } catch (...) {}

    switch (choice) {
    case 1: sampleCtrl_.run();     break;
    case 2: orderCtrl_.run();      break;
    case 3: approvalCtrl_.run();   break;
    case 4: monitorCtrl_.run();    break;
    case 5: productionCtrl_.run(); break;
    case 6: releaseCtrl_.run();    break;
    case 0:
        saveData();
        running = false;
        break;
    default:
        std::cout << "[오류] 잘못된 선택입니다.\n";
    }
}
std::cout << "데이터가 저장되었습니다. 프로그램을 종료합니다.\n";
```

**`loadData()`**:
```cpp
SampleStore sampleStore(SAMPLE_FILE);
sampleStore.load(sampleModel_);

OrderStore orderStore(ORDER_FILE);
orderStore.load(orderModel_);
```

**`saveData()`**:
```cpp
SampleStore sampleStore(SAMPLE_FILE);
sampleStore.save(sampleModel_);

OrderStore orderStore(ORDER_FILE);
orderStore.save(orderModel_);
```

**`showMainMenu()`** 출력:
```
============================================================
   반도체 시료 생산주문관리 시스템
============================================================
 시스템 현황  2026-04-16 09:32:15
------------------------------------------------------------
 등록 시료     12종    총 재고    2,840 ea
 전체 주문     36건    생산라인      3건 대기
------------------------------------------------------------
 [1] 시료 관리        [2] 시료 주문
 [3] 주문 승인/거절   [4] 모니터링
 [5] 생산라인 조회    [6] 출고 처리
 [0] 종료
------------------------------------------------------------
선택 > 
```

**`showSummary()`**:
- 현재 시각: `GetLocalTime()` + `std::format`
- 등록 시료 수: `sampleModel_.count()`
- 총 재고: `sampleModel_.totalStock()`
- 전체 주문 수: `orderModel_.totalCount()`
- 생산라인 대기: `productionLine_.pendingCount()` + (isRunning이면 +1)

---

## 6. main.cpp 수정

```cpp
#include "Controller/MainController.h"

int main() {
    MainController controller;
    controller.run();
    return 0;
}
```

> Phase 1에서 작성한 내용과 동일. 수정 불필요.

---

## 7. 저장 파일 위치

```
SampleOrderSystem_project/
└── Data/
    ├── samples.txt   ← 시료 목록 저장
    └── orders.txt    ← 주문 목록 저장
```

- 파일이 없으면 `load()` 시 무시 (빈 상태로 시작)
- 첫 실행 시 더미 데이터는 Phase 9에서 별도 처리

---

## 8. vcxproj 등록

### ClCompile 추가
```xml
<ClCompile Include="Data\DataStore.cpp" />
<ClCompile Include="Data\SampleStore.cpp" />
<ClCompile Include="Data\OrderStore.cpp" />
```

### ClInclude 추가
```xml
<ClInclude Include="Data\SampleStore.h" />
<ClInclude Include="Data\OrderStore.h" />
```

---

## 9. OrderModel 수정 사항

`OrderStore`에서 채번 상태 복원을 위해 `OrderModel`에 아래 메서드 추가:

```cpp
// OrderModel.h 추가
std::string lastDate() const { return lastDate_; }
int         dailySeq() const { return dailySeq_; }
void        setLastDate(const std::string& d) { lastDate_ = d; }
void        setDailySeq(int seq)              { dailySeq_ = seq; }
```

---

## 10. Phase 8 완료 조건

- [ ] `DataStore`: TXT 형식 load/save 구현 완료 (DataPersistence 참고 저장소 기반)
- [ ] `SampleStore`: SampleModel 직렬화/역직렬화 구현 완료
- [ ] `OrderStore`: OrderModel 직렬화/역직렬화 및 채번 상태 복원 구현 완료
- [ ] `MainController`: 전체 메뉴 루프, 시스템 현황 요약, load/save 호출 구현 완료
- [ ] `OrderModel`: lastDate/dailySeq getter/setter 추가 완료
- [ ] 시작 시 데이터 복원, 종료 시 데이터 저장 동작 확인
- [ ] vcxproj / vcxproj.filters 등록 완료
- [ ] 빌드 성공 (경고 없음)
- [ ] 사용자 검토 완료 후 commit
