# 반도체 시료 생산주문관리 시스템 - 구현 계획 (PLAN)

> PRD: [`.claude/docs/PRD.md`](PRD.md) 기반  
> 참고 저장소: ConsoleMVC / DataPersistence / DataMonitor / DummyDataGenerator

---

## 전체 Phase 구성 요약

| Phase | 내용 | 핵심 산출물 |
|-------|------|------------|
| Phase 1 | 프로젝트 뼈대 구성 | 디렉터리 구조, 도메인 모델 헤더, main.cpp |
| Phase 2 | 시료 관리 | SampleModel / SampleView / SampleController |
| Phase 3 | 주문 접수 | OrderModel / OrderView / OrderController |
| Phase 4 | 주문 승인/거절 | ApprovalController, 재고 확인 + 상태 전환 로직 |
| Phase 5 | 생산 라인 | ProductionLine(FIFO 큐), 생산량 계산, 상태 전환 |
| Phase 6 | 출고 처리 | ReleaseController, CONFIRMED → RELEASED |
| Phase 7 | 모니터링 | MonitorView, 상태별 집계·재고 상태 판정 |
| Phase 8 | 메인 통합 & 영속성 | MainController, 파일 저장/복원 |
| Phase 9 | 더미 데이터 & 테스트 | 초기 데이터, gMock 단위 테스트 |

---

## Phase 1 — 프로젝트 뼈대 구성

### 목표
MVC 디렉터리 구조를 세팅하고, 전체 시스템이 공유할 도메인 모델 헤더를 정의한다.  
ConsoleMVC 참고 저장소의 디렉터리 레이아웃을 기반으로 한다.

### 디렉터리 구조
```
SampleOrderSystem_project/
├── main.cpp
├── Model/
│   ├── Sample.h          ← 시료 엔티티
│   ├── SampleModel.h/.cpp
│   ├── Order.h           ← 주문 엔티티 + OrderStatus enum
│   ├── OrderModel.h/.cpp
│   ├── ProductionJob.h   ← 생산 작업 엔티티
│   └── ProductionLine.h/.cpp
├── View/
│   ├── SampleView.h/.cpp
│   ├── OrderView.h/.cpp
│   ├── ProductionView.h/.cpp
│   └── MonitorView.h/.cpp
├── Controller/
│   ├── MainController.h/.cpp
│   ├── SampleController.h/.cpp
│   ├── OrderController.h/.cpp
│   ├── ApprovalController.h/.cpp
│   ├── ReleaseController.h/.cpp
│   └── MonitorController.h/.cpp
└── Data/
    ├── DataStore.h/.cpp      ← DataPersistence 참고 (key-value, TXT/JSON)
    └── RecordStore.h/.cpp    ← DataPersistence 참고 (ID 기반 CRUD)
```

### 도메인 모델 정의

**Sample.h**
```cpp
struct Sample {
    std::string id;            // "S-001"
    std::string name;          // "실리콘 웨이퍼-8인치"
    double      avgProdTime;   // min/ea
    double      yield;         // 0.0 ~ 1.0
    int         stock;         // 현재 재고 (ea)
};
```

**Order.h**
```cpp
enum class OrderStatus { RESERVED, REJECTED, PRODUCING, CONFIRMED, RELEASED };

struct Order {
    std::string  orderId;      // "ORD-20260416-0043"
    std::string  sampleId;
    std::string  customer;
    int          quantity;
    OrderStatus  status;
    std::string  createdAt;    // "YYYY-MM-DD HH:MM:SS"
};
```

**ProductionJob.h**
```cpp
struct ProductionJob {
    std::string orderId;
    std::string sampleId;
    int         shortage;       // 부족분
    int         actualQty;      // 실 생산량 = ceil(부족분 / (수율 × 0.9))
    double      totalMinutes;   // 총 생산시간 = avgProdTime × actualQty
};
```

### main.cpp 진입점
```cpp
// ConsoleMVC 패턴 그대로 적용
SampleModel    sampleModel;
OrderModel     orderModel;
ProductionLine productionLine;
MainController controller{ sampleModel, orderModel, productionLine };
controller.run();
```

---

## Phase 2 — 시료 관리

### 목표
시료 등록·조회·검색 기능을 MVC로 구현한다.

### SampleModel
| 메서드 | 설명 |
|--------|------|
| `add(Sample)` | 시료 등록 (중복 ID 거부) |
| `findById(id)` | ID로 단건 조회 |
| `findByName(keyword)` | 이름 부분 검색 |
| `all()` | 전체 목록 반환 |
| `updateStock(id, delta)` | 재고 증감 |
| `totalStock()` | 전체 재고 합산 |

### SampleView
- `showSampleMenu()` — 서브메뉴 출력
- `promptSampleInput()` → `Sample` 반환
- `showSampleList(samples)` — 페이지 단위 목록 표시 (ID / 이름 / 평균생산시간 / 수율 / 재고)
- `showSampleDetail(sample)` — 단건 상세
- `promptSearchKeyword()` → `std::string`

### SampleController
- 서브메뉴 루프: `[1] 등록` `[2] 목록` `[3] 검색` `[0] 위로`
- 등록 시 중복 ID 검증 → 오류 메시지 표시

---

## Phase 3 — 주문 접수

### 목표
고객 주문을 접수하여 `RESERVED` 상태의 주문을 생성한다.

### OrderModel
| 메서드 | 설명 |
|--------|------|
| `reserve(sampleId, customer, qty)` | 주문 생성, 주문번호 자동 채번 |
| `findById(orderId)` | 주문번호로 조회 |
| `findByStatus(status)` | 상태별 목록 |
| `updateStatus(orderId, status)` | 상태 전환 |
| `all()` | 전체 주문 목록 |
| `totalCount()` | 전체 주문 수 |

**주문번호 채번 규칙**
```
ORD-{YYYYMMDD}-{NNNN}
NNNN: 당일 일련번호 (0001부터 시작, 자릿수 4)
```

### OrderView
- `promptOrderInput(samples)` → 시료 ID·고객명·수량 입력, 내용 확인 후 Y/N
- `showOrderResult(order)` — 접수 완료 결과 (주문번호 / 상태 표시)

### OrderController
- SampleModel에서 시료 ID 유효성 확인 후 주문 생성
- 접수 직후 재고 확인 없이 RESERVED만 생성

---

## Phase 4 — 주문 승인/거절

### 목표
RESERVED 주문 목록을 보고 승인 또는 거절을 처리한다.  
승인 시 재고 상황에 따라 자동으로 CONFIRMED / PRODUCING으로 분기한다.

### 승인 로직 흐름
```
재고 = SampleModel.findById(sampleId).stock
유효 재고 = 재고 - (이미 CONFIRMED 상태인 동일 시료의 주문 수량 합산)  ← 출고 대기분 차감

유효 재고 >= 주문 수량
  → CONFIRMED 전환
  → SampleModel.updateStock(sampleId, -주문수량)

유효 재고 < 주문 수량
  → 부족분 = 주문 수량 - 유효 재고
  → ProductionLine.enqueue(ProductionJob) 등록
  → PRODUCING 전환
```

> CONFIRMED 대기 중인 주문의 재고를 유효 재고에서 차감하는 것이 핵심.  
> 이 처리가 없으면 동일 재고를 중복으로 승인하는 버그 발생.

### 거절 로직
- RESERVED → REJECTED 즉시 전환

### ApprovalController
- `[1] 승인` `[2] 거절` 선택 후 처리
- 승인 시 재고 정보와 부족분·실생산량·예상 생산시간 사전 표시

---

## Phase 5 — 생산 라인

### 목표
FIFO 큐 기반 단일 생산 라인을 구현한다.  
생산 완료 시 재고를 보충하고 주문 상태를 CONFIRMED으로 전환한다.

### ProductionLine
| 구성요소 | 설명 |
|---------|------|
| `std::queue<ProductionJob> queue_` | FIFO 대기 큐 |
| `std::optional<ProductionJob> current_` | 현재 처리 중인 작업 |

**생산량 계산 공식**
```
부족분      = 주문 수량 - 현재 유효 재고
실 생산량   = ceil(부족분 / (수율 × 0.9))   // 0.9: 고정 보정 상수
총 생산시간 = 평균 생산시간(min/ea) × 실 생산량
```

| 메서드 | 설명 |
|--------|------|
| `enqueue(job)` | 생산 큐에 작업 추가 |
| `complete(orderId)` | 현재 작업 완료 처리 (재고 보충 + CONFIRMED 전환) |
| `currentJob()` | 현재 처리 중 작업 반환 |
| `pendingJobs()` | 대기 큐 전체 반환 |
| `pendingCount()` | 대기 건수 |

### ProductionView
- 현재 처리 중 작업: 주문번호 / 시료 / 주문량 / 부족분 / 실생산량 / 총 생산시간
- 대기 목록 (FIFO 순): 순서 / 주문번호 / 시료 / 주문량 / 부족분 / 실생산량 / 예상 완료

---

## Phase 6 — 출고 처리

### 목표
`CONFIRMED` 상태 주문에 대해 출고를 실행하고 `RELEASED`로 전환한다.

### ReleaseController
1. `OrderModel.findByStatus(CONFIRMED)` 목록 표시
2. 출고할 번호 입력
3. `OrderModel.updateStatus(orderId, RELEASED)`
4. 출고 결과 표시 (주문번호 / 출고수량 / 처리일시 / 상태 변경)

> 출고 시 재고 차감은 Phase 4 승인 시점에 이미 처리되었으므로 중복 차감하지 않는다.

---

## Phase 7 — 모니터링

### 목표
시스템 전체 상태를 한눈에 파악할 수 있는 현황 화면을 제공한다.  
DataMonitor의 컬러 출력·바 그래프 패턴을 참고한다.

### [1] 주문량 확인
- 상태별 주문 건수: `RESERVED` / `CONFIRMED` / `PRODUCING` / `RELEASED`
- `REJECTED` 제외

### [2] 재고량 확인
| 상태 | 판정 조건 |
|------|-----------|
| 고갈 | `stock == 0` |
| 부족 | `stock > 0` && CONFIRMED + RESERVED 주문 수량 합 > stock |
| 여유 | 그 외 |

- 시료별 재고 수량 + 상태 + 잔여율(%) + 바 그래프 출력
- Win32 콘솔 컬러: 여유=초록 / 부족=노랑 / 고갈=빨강

---

## Phase 8 — 메인 통합 & 데이터 영속성

### 목표
모든 Controller를 MainController로 통합하고, 시작/종료 시 데이터를 파일로 복원/저장한다.

### MainController
- 메인 메뉴 루프 (`[1]~[6]`, `[0]`)
- **메인 화면 현황 요약 표시**:
  - 등록 시료 수 / 총 재고 / 전체 주문 수 / 생산라인 대기 수
  - 현재 날짜·시간 표시

### 데이터 영속성 (DataPersistence 참고)
- `DataStore` + `RecordStore` 계층 구조 그대로 적용
- 저장 대상: 시료 목록, 주문 목록
- 저장 포맷: TXT (기본) — 확장자 변경만으로 JSON 전환 가능
- **시작 시**: `load()` → 저장된 데이터 복원
- **종료 시**: `save()` → 현재 상태 파일에 기록

```
Data/
  samples.txt    ← 시료 목록 저장
  orders.txt     ← 주문 목록 저장
```

---

## Phase 9 — 더미 데이터 & gMock 테스트

### 목표
개발·시연용 초기 데이터를 제공하고, gMock 기반 단위 테스트를 작성한다.

### 초기 더미 데이터 (DummyDataGenerator 참고)
PRD 예시 데이터 5종을 초기 로드 또는 별도 초기화 함수로 제공:

| ID | 시료명 | 생산시간 | 수율 | 초기 재고 |
|----|--------|---------|------|-----------|
| S-001 | 실리콘 웨이퍼-8인치 | 0.5 min/ea | 0.92 | 480 ea |
| S-002 | GaN 에피택셀-4인치 | 0.3 min/ea | 0.78 | 220 ea |
| S-003 | SiC 파워기판-6인치 | 0.8 min/ea | 0.92 | 30 ea |
| S-004 | 포토레지스트-PR7 | 0.2 min/ea | 0.95 | 910 ea |
| S-005 | 산화막 웨이퍼-SiO2 | 0.6 min/ea | 0.88 | 0 ea |

### gMock 테스트 대상

| 테스트 클래스 | 검증 항목 |
|--------------|-----------|
| `SampleModelTest` | 등록·중복 거부·검색·재고 증감 |
| `OrderModelTest` | 주문번호 채번 규칙·상태 전환·상태별 조회 |
| `ProductionLineTest` | FIFO 순서·생산량 공식(ceil)·완료 시 CONFIRMED 전환 |
| `ApprovalControllerTest` | 재고 충분/부족 분기·CONFIRMED 대기분 차감 로직 |

---

## 구현 시 주의사항

1. **CONFIRMED 대기분 차감** (Phase 4): 승인 시점에 이미 CONFIRMED된 동일 시료 주문의 수량을 재고에서 차감한 후 비교해야 이중 승인 버그를 방지할 수 있다.
2. **생산 공식 상수 0.9 고정**: 수율 보정 비율은 PRD 명세상 상수이며 변경하지 않는다.
3. **주문번호 채번**: 날짜가 바뀌면 일련번호를 0001부터 재시작한다.
4. **출고 시 재고 차감 금지**: 재고 차감은 승인(Phase 4) 시점에만 수행한다.
5. **모든 파일 인코딩**: UTF-8 with BOM (코드 페이지 65001) 저장 필수.
