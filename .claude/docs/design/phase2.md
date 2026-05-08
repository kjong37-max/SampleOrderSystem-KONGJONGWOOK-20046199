# Phase 2 — 시료 관리 설계

> 참고: [PLAN.md](../PLAN.md) | [PRD.md](../PRD.md)  
> 구현 전 **사용자 검토 후 commit** 규칙 준수

---

## 목표

시료(Sample) 등록·조회·검색 기능을 MVC 패턴으로 구현한다.  
ConsoleMVC 참고 저장소의 StudentModel / View / Controller 구조를 `Sample` 도메인에 맞게 적용한다.

---

## 1. 생성 및 수정 파일 목록

| 구분 | 파일 | 작업 |
|------|------|------|
| Model | `Model/SampleModel.h` | 신규 |
| Model | `Model/SampleModel.cpp` | 신규 |
| View | `View/SampleView.h` | 수정 (빈 선언 → 실제 선언) |
| View | `View/SampleView.cpp` | 신규 |
| Controller | `Controller/SampleController.h` | 수정 (빈 선언 → 실제 선언) |
| Controller | `Controller/SampleController.cpp` | 신규 |
| vcxproj | `SampleOrderSystem_project.vcxproj` | ClCompile 3개 추가 |
| vcxproj.filters | `SampleOrderSystem_project.vcxproj.filters` | 소스 파일 필터 3개 추가 |

---

## 2. SampleModel 설계

### Model/SampleModel.h

```cpp
#pragma once
#include "Sample.h"
#include <optional>
#include <span>
#include <string>
#include <vector>

class SampleModel {
public:
    // 시료 등록. 중복 ID면 false 반환
    bool add(const Sample& sample);

    // ID로 단건 조회
    std::optional<Sample> findById(const std::string& id) const;

    // 이름 부분 문자열 검색 (대소문자 무시)
    std::vector<Sample> findByName(const std::string& keyword) const;

    // 전체 목록
    std::span<const Sample> all() const;

    // 재고 증감 (음수 가능 → 차감). stock이 0 미만으로 내려가지 않도록 보장
    bool updateStock(const std::string& id, int delta);

    // 전체 시료 재고 합산
    int totalStock() const;

    // 등록된 시료 수
    int count() const;

private:
    std::vector<Sample> samples_;
};
```

### Model/SampleModel.cpp — 주요 구현 명세

**`add()`**
- `samples_` 내 동일 `id` 존재 여부 확인 → 중복이면 `false` 반환
- 중복 아니면 `samples_.push_back(sample)` 후 `true` 반환

**`findById()`**
- `std::ranges::find_if`로 `id` 일치 항목 탐색
- 없으면 `std::nullopt` 반환

**`findByName()`**
- `keyword`를 소문자로 변환 후, 각 시료 `name` 소문자와 `contains` 비교
- 일치 항목을 `std::vector<Sample>`에 담아 반환

**`updateStock()`**
- `findById`로 대상 탐색 → 없으면 `false`
- `stock + delta < 0`이면 `false` (음수 재고 불허)
- 조건 통과 시 `stock += delta` 후 `true`

**`totalStock()`**
- `std::ranges::fold_left` (또는 `accumulate`)로 전체 `stock` 합산

---

## 3. SampleView 설계

### View/SampleView.h

```cpp
#pragma once
#include "../Model/Sample.h"
#include <span>
#include <string>
#include <vector>

class SampleView {
public:
    // 서브 메뉴 출력
    void showMenu() const;

    // 시료 등록 입력 받기
    Sample promptSampleInput() const;

    // 전체 목록 출력 (페이지 단위)
    void showSampleList(std::span<const Sample> samples) const;

    // 검색 결과 출력
    void showSearchResult(const std::vector<Sample>& samples) const;

    // 단건 상세 출력
    void showSampleDetail(const Sample& sample) const;

    // 검색 키워드 입력
    std::string promptSearchKeyword() const;

    // 공통 메시지 출력
    void showMessage(const std::string& msg) const;
    void showError(const std::string& msg)   const;

    // 메뉴 선택 입력
    int promptMenuChoice() const;
};
```

### View/SampleView.cpp — 화면 구성 명세

**`showMenu()`** 출력 형식:
```
============================================================
 [1] 시료 관리
------------------------------------------------------------
 [1] 시료 등록   [2] 시료 목록   [3] 시료 검색   [0] 위로
선택 > 
```

**`promptSampleInput()`** 입력 순서:
```
시료 ID        > S-006
시료명         > 질화갈륨 웨이퍼-2인치
평균 생산시간  > 0.4
수율 (0~1)     > 0.85
초기 재고      > 100

--- 입력 확인 ---
ID     : S-006
이름   : 질화갈륨 웨이퍼-2인치
생산시간: 0.4 min/ea
수율   : 0.85
재고   : 100 ea
[Y] 등록   [N] 취소 > 
```
- Y 입력 시 `Sample` 반환, N 이면 루프를 통해 재입력 or 취소

**`showSampleList()`** 출력 형식 (페이지: 5개씩):
```
등록 시료 목록  (총 12종)
------------------------------------------------------------
 ID      시료명                    생산시간   수율    재고
------------------------------------------------------------
 S-001   실리콘 웨이퍼-8인치       0.5 min   0.92   480 ea
 S-002   GaN 에피택셀-4인치        0.3 min   0.78   220 ea
 ...외 7종   [N] 다음페이지   [0] 위로
선택 > 
```

**`showSampleDetail()`** 출력 형식:
```
------------------------------------------------------------
 ID        : S-003
 시료명    : SiC 파워기판-6인치
 생산시간  : 0.8 min/ea
 수율      : 0.92  (92%)
 현재 재고 : 30 ea
------------------------------------------------------------
```

**`promptSearchKeyword()`**:
```
검색어 입력 > 
```

---

## 4. SampleController 설계

### Controller/SampleController.h

```cpp
#pragma once
#include "../Model/SampleModel.h"
#include "../View/SampleView.h"

class SampleController {
public:
    SampleController(SampleModel& model, SampleView& view);
    void run();

private:
    void handleAdd();
    void handleList();
    void handleSearch();

    SampleModel& model_;
    SampleView&  view_;
};
```

### Controller/SampleController.cpp — 흐름 명세

**`run()`**:
```
while (true) {
    view_.showMenu();
    switch (view_.promptMenuChoice()) {
        case 1: handleAdd();    break;
        case 2: handleList();   break;
        case 3: handleSearch(); break;
        case 0: return;         // 메인으로 복귀
        default: view_.showError("잘못된 선택입니다.");
    }
}
```

**`handleAdd()`**:
```
Sample s = view_.promptSampleInput();
if (model_.add(s))
    view_.showMessage("시료가 등록되었습니다. (ID: " + s.id + ")");
else
    view_.showError("이미 존재하는 시료 ID입니다: " + s.id);
```

**`handleList()`**:
```
view_.showSampleList(model_.all());
```

**`handleSearch()`**:
```
keyword = view_.promptSearchKeyword();
results = model_.findByName(keyword);
if (results.empty())
    view_.showError("검색 결과가 없습니다.");
else
    view_.showSearchResult(results);
```

---

## 5. vcxproj 등록

### ClCompile 추가 (SampleOrderSystem_project.vcxproj)

```xml
<ClCompile Include="Model\SampleModel.cpp" />
<ClCompile Include="View\SampleView.cpp" />
<ClCompile Include="Controller\SampleController.cpp" />
```

### vcxproj.filters 추가

```xml
<ClCompile Include="Model\SampleModel.cpp">
  <Filter>소스 파일</Filter>
</ClCompile>
<ClCompile Include="View\SampleView.cpp">
  <Filter>소스 파일</Filter>
</ClCompile>
<ClCompile Include="Controller\SampleController.cpp">
  <Filter>소스 파일</Filter>
</ClCompile>
```

---

## 6. 유효성 검사 규칙

| 필드 | 규칙 |
|------|------|
| 시료 ID | 비어있으면 안 됨. 중복 불허 |
| 시료명 | 비어있으면 안 됨 |
| 평균 생산시간 | 0.0 초과 |
| 수율 | 0.0 초과 1.0 이하 |
| 초기 재고 | 0 이상 |

유효성 검사는 `SampleView::promptSampleInput()` 내부에서 재입력 루프로 처리한다.  
Model은 중복 ID만 검사하고 나머지 범위 검증은 View에 위임한다.

---

## 7. Phase 2 완료 조건

- [ ] `SampleModel`: add / findById / findByName / all / updateStock / totalStock 구현 완료
- [ ] `SampleView`: 메뉴·입력·목록·검색·상세 화면 구현 완료
- [ ] `SampleController`: 서브메뉴 루프 및 handle* 메서드 구현 완료
- [ ] vcxproj / vcxproj.filters 등록 완료
- [ ] 빌드 성공 (경고 없음)
- [ ] 시료 등록 → 목록 조회 → 이름 검색 동작 확인
- [ ] 사용자 검토 완료 후 commit
