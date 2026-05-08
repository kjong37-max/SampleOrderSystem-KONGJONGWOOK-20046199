# Step 2 — 매직 넘버 상수화 설계

> 참고: [PLAN.md](../PLAN.md)  
> 브랜치: `refactoring`  
> 구현 전 **사용자 검토 후 commit** 규칙 준수

---

## 목표

코드에 흩어진 매직 넘버를 네임드 상수로 중앙화한다.  
**동작 변경 없음. 기존 테스트 모두 계속 통과해야 한다.**

---

## 대상 매직 넘버

| 값 | 위치 | 의미 |
|----|------|------|
| `0.9` | `Controller/ApprovalController.cpp:79` | 생산량 보정 비율 (고정 상수) |
| `5` | `View/SampleView.cpp:116` | 목록 페이지 크기 |
| `20` | `View/MonitorView.h` 기본 인자 / `MonitorView.cpp:103` | 모니터링 바 그래프 너비 |

---

## 생성/수정 파일 목록

| 구분 | 파일 | 작업 |
|------|------|------|
| 신규 | `Model/Constants.h` | 프로젝트 공용 상수 정의 |
| 수정 | `Controller/ApprovalController.cpp` | `0.9` → `SemiConst::PRODUCTION_CORRECTION` |
| 수정 | `View/SampleView.cpp` | `PAGE_SIZE = 5` → `SemiConst::PAGE_SIZE` |
| 수정 | `View/MonitorView.h` | 기본 인자 `20` → `SemiConst::BAR_WIDTH` |
| 수정 | `View/MonitorView.cpp` | `width` 사용부는 이미 매개변수로 전달되므로 변경 불필요 |
| 수정 | `SampleOrderSystem_project.vcxproj` | ClInclude 추가 |
| 수정 | `SampleOrderSystem_project.vcxproj.filters` | 헤더 파일 필터 추가 |

---

## Model/Constants.h

```cpp
#pragma once

namespace SemiConst {
    // 생산량 보정 비율 (PRD 고정 상수)
    // 실 생산량 = ceil(부족분 / (수율 × PRODUCTION_CORRECTION))
    constexpr double PRODUCTION_CORRECTION = 0.9;

    // 시료 목록 페이지 크기
    constexpr int PAGE_SIZE = 5;

    // 모니터링 재고 바 그래프 너비 (칸 수)
    constexpr int BAR_WIDTH = 20;
}
```

---

## 수정 명세

### Controller/ApprovalController.cpp

```cpp
// 변경 전
int actualQty = static_cast<int>(std::ceil(
                    static_cast<double>(shortage) / (sample.yield * 0.9)));

// 변경 후
int actualQty = static_cast<int>(std::ceil(
                    static_cast<double>(shortage) /
                    (sample.yield * SemiConst::PRODUCTION_CORRECTION)));
```

include 추가: `#include "../Model/Constants.h"`

### View/SampleView.cpp

```cpp
// 변경 전
constexpr int PAGE_SIZE = 5;

// 변경 후
// (지역 constexpr 제거, SemiConst::PAGE_SIZE 사용)
```

include 추가: `#include "../Model/Constants.h"`

### View/MonitorView.h

```cpp
// 변경 전
void printBar(int stock, int maxStock, int width = 20) const;

// 변경 후
void printBar(int stock, int maxStock,
              int width = SemiConst::BAR_WIDTH) const;
```

include 추가: `#include "../Model/Constants.h"`

---

## Step 2 완료 조건

- [ ] `Model/Constants.h` 생성 완료
- [ ] 3개 파일 수정 완료 (ApprovalController.cpp, SampleView.cpp, MonitorView.h)
- [ ] 빌드 성공 (경고 없음)
- [ ] Debug 빌드 전체 테스트 통과
- [ ] 사용자 검토 완료 후 commit
