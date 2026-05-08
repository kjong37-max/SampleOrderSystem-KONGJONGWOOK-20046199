# Step 3 — 날짜/시간 유틸리티 추출 설계

> 참고: [PLAN.md](../PLAN.md) | 브랜치: `refactoring`  
> 구현 전 **사용자 검토 후 commit** 규칙 준수

---

## 목표

동일한 날짜/시간 포맷 코드가 4곳에 중복 → `Utils/TimeUtils`로 통합한다.  
**동작 변경 없음. 기존 테스트 모두 계속 통과해야 한다.**

---

## 현재 중복 위치

| 파일 | 라인 | 용도 |
|------|------|------|
| `Model/OrderModel.cpp:12` | `std::format("{:04d}{:02d}{:02d}", ...)` | generateOrderId() 날짜 |
| `Model/OrderModel.cpp:35` | `std::format("{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}", ...)` | createdAt 설정 |
| `Controller/MainController.cpp:78` | 동일 포맷 | 메인 메뉴 시각 표시 |
| `Controller/ReleaseController.cpp:87` | 동일 포맷 | 출고 처리 일시 |

---

## 생성/수정 파일 목록

| 구분 | 파일 | 작업 |
|------|------|------|
| 신규 | `Utils/TimeUtils.h` | 날짜/시간 유틸리티 선언 |
| 신규 | `Utils/TimeUtils.cpp` | 구현 |
| 수정 | `Model/OrderModel.cpp` | TimeUtils 사용, Windows.h 의존성 제거 |
| 수정 | `Controller/MainController.cpp` | TimeUtils 사용 |
| 수정 | `Controller/ReleaseController.h` | getCurrentDateTime() private 선언 제거 |
| 수정 | `Controller/ReleaseController.cpp` | getCurrentDateTime() 구현 제거 → TimeUtils 사용 |
| 수정 | `SampleOrderSystem_project.vcxproj` | ClCompile/ClInclude 추가 |
| 수정 | `SampleOrderSystem_project.vcxproj.filters` | 필터 추가 |

---

## Utils/TimeUtils.h

```cpp
#pragma once
#include <string>

namespace TimeUtils {
    // "YYYY-MM-DD HH:MM:SS" 형식 현재 시각 반환
    std::string nowDateTime();

    // "YYYYMMDD" 형식 현재 날짜 반환
    std::string nowDate();
}
```

## Utils/TimeUtils.cpp

```cpp
#define NOMINMAX
#include <Windows.h>
#include "TimeUtils.h"
#include <format>

namespace TimeUtils {

std::string nowDateTime() {
    SYSTEMTIME st{};
    GetLocalTime(&st);
    return std::format("{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond);
}

std::string nowDate() {
    SYSTEMTIME st{};
    GetLocalTime(&st);
    return std::format("{:04d}{:02d}{:02d}",
        st.wYear, st.wMonth, st.wDay);
}

} // namespace TimeUtils
```

---

## 수정 명세

### Model/OrderModel.cpp

- `#include "../Utils/TimeUtils.h"` 추가
- `#define NOMINMAX` / `#include <Windows.h>` 제거 (TimeUtils.cpp가 담당)
- `generateOrderId()`: `std::format(...)` 직접 호출 → `TimeUtils::nowDate()`
- `reserve()`: `std::format(...)` 직접 호출 → `TimeUtils::nowDateTime()`

### Controller/MainController.cpp

- `#include "../Utils/TimeUtils.h"` 추가
- `showMainMenu()`: `std::format(...)` 날짜 코드 → `TimeUtils::nowDateTime()`
- `#define NOMINMAX` / `#include <Windows.h>` 는 유지 (GetLocalTime 외 용도 없으면 제거 가능하나 안전하게 유지)

### Controller/ReleaseController.h

```cpp
// 삭제할 private 선언
std::string getCurrentDateTime() const;
```

### Controller/ReleaseController.cpp

- `#include "../Utils/TimeUtils.h"` 추가
- `run()` 내 `getCurrentDateTime()` 호출 → `TimeUtils::nowDateTime()` 직접 사용
- `getCurrentDateTime()` 구현 전체 삭제

---

## vcxproj 등록

```xml
<ClCompile Include="Utils\TimeUtils.cpp" />
<ClInclude Include="Utils\TimeUtils.h" />
```

---

## Step 3 완료 조건

- [ ] `Utils/TimeUtils.h/.cpp` 생성 완료
- [ ] OrderModel.cpp / MainController.cpp / ReleaseController.h/.cpp 수정 완료
- [ ] 빌드 성공 (경고 없음)
- [ ] 전체 테스트 통과
- [ ] 사용자 검토 완료 후 commit
