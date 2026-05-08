# Step 5 — promptMenuChoice() 공통화 설계

> 참고: [PLAN.md](../PLAN.md) | 브랜치: `refactoring`  
> 구현 전 **사용자 검토 후 commit** 규칙 준수

---

## 목표

4개 View에 동일하게 구현된 `promptMenuChoice()` 중복 제거 → `Utils/ConsoleUtils`로 통합한다.  
**동작 변경 없음. 기존 테스트 모두 계속 통과해야 한다.**

---

## 현재 중복 위치

모든 View의 `promptMenuChoice()` 구현이 완전히 동일:
```cpp
int XxxView::promptMenuChoice() const {
    std::string line;
    std::getline(std::cin, line);
    std::istringstream iss(line);
    int choice{};
    if (iss >> choice)
        return choice;
    return -1;
}
```

| 파일 | 라인 |
|------|------|
| `View/SampleView.cpp` | 225~233 |
| `View/OrderView.cpp` | ~163 |
| `View/ProductionView.cpp` | ~80 |
| `View/MonitorView.cpp` | ~115 |

---

## 생성/수정 파일 목록

| 구분 | 파일 | 작업 |
|------|------|------|
| 신규 | `Utils/ConsoleUtils.h` | 공통 콘솔 유틸리티 선언 |
| 신규 | `Utils/ConsoleUtils.cpp` | 구현 |
| 수정 | `View/SampleView.cpp` | promptMenuChoice() 구현 → ConsoleUtils 위임 |
| 수정 | `View/OrderView.cpp` | 동일 |
| 수정 | `View/ProductionView.cpp` | 동일 |
| 수정 | `View/MonitorView.cpp` | 동일 |
| 수정 | `SampleOrderSystem_project.vcxproj` | ClCompile/ClInclude 추가 |
| 수정 | `SampleOrderSystem_project.vcxproj.filters` | 필터 추가 |

> 각 View의 헤더(.h)는 변경 없음 — 인터페이스는 그대로 유지하고 구현만 위임한다.

---

## Utils/ConsoleUtils.h

```cpp
#pragma once

namespace ConsoleUtils {
    // 숫자 메뉴 입력. 비숫자 입력 시 -1 반환
    int promptMenuChoice();
}
```

## Utils/ConsoleUtils.cpp

```cpp
#include "ConsoleUtils.h"
#include <iostream>
#include <sstream>
#include <string>

namespace ConsoleUtils {

int promptMenuChoice() {
    std::string line;
    std::getline(std::cin, line);
    std::istringstream iss(line);
    int choice{};
    if (iss >> choice)
        return choice;
    return -1;
}

} // namespace ConsoleUtils
```

---

## 수정 명세 (4개 View 동일 패턴)

각 View의 `promptMenuChoice()` 구현을 아래와 같이 교체:

```cpp
// 변경 전 (각 View.cpp의 구현)
int XxxView::promptMenuChoice() const {
    std::string line;
    std::getline(std::cin, line);
    std::istringstream iss(line);
    int choice{};
    if (iss >> choice) return choice;
    return -1;
}

// 변경 후
int XxxView::promptMenuChoice() const {
    return ConsoleUtils::promptMenuChoice();
}
```

각 View.cpp에 `#include "../Utils/ConsoleUtils.h"` 추가.  
불필요해진 `#include <sstream>` 은 해당 View에서 다른 곳에서도 사용 중인지 확인 후 제거.

---

## vcxproj 등록

```xml
<ClCompile Include="Utils\ConsoleUtils.cpp" />
<ClInclude Include="Utils\ConsoleUtils.h" />
```

---

## Step 5 완료 조건

- [ ] `Utils/ConsoleUtils.h/.cpp` 생성 완료
- [ ] 4개 View.cpp 수정 완료
- [ ] 빌드 성공 (경고 없음)
- [ ] 전체 테스트 통과
- [ ] 사용자 검토 완료 후 commit
