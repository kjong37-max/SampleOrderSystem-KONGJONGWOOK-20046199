# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 프로젝트 개요

반도체 시료 생산주문관리 시스템(SampleOrderSystem) - 반도체 공정에서 사용되는 시료의 생산 주문을 등록·추적·관리하는 Win32 콘솔 애플리케이션입니다.

> 전체 기능 요구사항은 [`.claude/docs/PRD.md`](.claude/docs/PRD.md)를 참조하세요.  
> 구현 계획은 [`.claude/docs/PLAN.md`](.claude/docs/PLAN.md)를 참조하세요.

## 빌드 환경

- **IDE**: Visual Studio 2022 (toolset v145)
- **언어**: C++20 (`/std:c++20`)
- **플랫폼**: Windows 10, Win32 및 x64 지원
- **출력 유형**: Console Application (`_CONSOLE`)
- **문자셋**: Unicode

### 빌드 명령 (MSBuild / 개발자 명령 프롬프트)

```bat
# Debug x64 빌드
msbuild SampleOrderSystem_project.vcxproj /p:Configuration=Debug /p:Platform=x64

# Release x64 빌드
msbuild SampleOrderSystem_project.vcxproj /p:Configuration=Release /p:Platform=x64

# 정리
msbuild SampleOrderSystem_project.vcxproj /t:Clean /p:Configuration=Debug /p:Platform=x64
```

Visual Studio에서는 `F5`(디버그 실행) 또는 `Ctrl+Shift+B`(빌드)를 사용합니다.

## 프로젝트 구조 규칙

새 파일을 추가할 때는 `SampleOrderSystem_project.vcxproj`의 `<ItemGroup>`과 `.vcxproj.filters`의 해당 필터에 함께 등록해야 합니다.

| 필터명 | 경로 용도 |
|--------|-----------|
| 소스 파일 | `.cpp` 구현 파일 |
| 헤더 파일 | `.h` / `.hpp` 선언 파일 |
| 리소스 파일 | `.rc`, 아이콘, 이미지 등 |

## 아키텍처 지침

이 시스템은 반도체 시료의 **주문 등록 → 생산 배정 → 진행 추적 → 완료 처리** 흐름을 담당합니다. 구현 시 다음 구조를 권장합니다.

- **도메인 모델 (헤더)**: `Order`, `Sample`, `ProductionLine` 등 핵심 엔티티를 struct/class로 정의
- **관리자 클래스**: 주문 목록 CRUD, 생산 배정 로직을 담당하는 Manager/Repository 계층
- **UI 계층**: 콘솔 메뉴 루프 — 도메인 로직과 분리 유지
- **데이터 영속성**: 파일 I/O(CSV, 바이너리) 또는 SQLite 등 선택에 따라 별도 모듈로 격리

## 개발 참고 저장소

| 용도 | 저장소 |
|------|--------|
| MVC 스켈레톤 코드 | https://github.com/kjong37-max/ConsoleMVC-KONGJONGWOOK-20046199.git |
| 데이터 영속성 처리 | https://github.com/kjong37-max/DataPersistence-KONGJONGWOOK-20046199.git |
| 데이터 모니터링 Tool | https://github.com/kjong37-max/DataMonitor-KONGJONGWOOK-20046199.git |
| Dummy 데이터 생성 Tool | https://github.com/kjong37-max/DummyDataGenerator-KONGJONGWOOK-20046199.git |

## 워크플로우 규칙

> **IMPORTANT — phase 문서 기반 개발 시 commit 금지**
>
> phase 문서를 기반으로 코드를 개발한 후, **절대로 즉시 commit하지 말 것.**
> 반드시 사용자의 검토(코드 확인 및 승인)가 완료된 이후에만 commit을 진행한다.
> 검토 요청 없이 commit을 먼저 실행하는 것은 금지된 행동이다.

## 코딩 규칙

- C++20 기능(`std::ranges`, `std::format`, structured bindings 등) 적극 활용
- `using namespace std;` 헤더 파일에서 금지 — 소스 파일 내 지역 범위만 허용
- 문자열은 `std::wstring` 또는 `std::string` + UTF-8 정책을 프로젝트 전체에서 통일
- 메모리 관리: raw pointer 대신 `std::unique_ptr` / `std::shared_ptr` 사용
- 빌드 경고 레벨 Level3(`/W3`) — 경고를 오류처럼 취급하여 무시하지 않음

## 테스트 규칙

- 테스트 코드는 **gMock** (Google Mock)을 활용하여 작성한다.

## 파일 인코딩 규칙

- 모든 소스 파일은 **유니코드 (서명 있는 UTF-8) — 코드 페이지 65001** 로 저장한다.
- Visual Studio 기준: 파일 저장 시 `파일 > 다른 이름으로 저장 > 저장` 옆 드롭다운 → `인코딩하여 저장` → `유니코드 (서명 있는 UTF-8) - 코드 페이지 65001` 선택.
- BOM(Byte Order Mark)이 포함된 UTF-8(`UTF-8 with BOM`)이며, MSVC가 소스 파일을 UTF-8로 올바르게 인식하기 위해 필수다.
- **BOM 없이 저장된 파일이 있을 경우**: MSVC가 시스템 코드페이지(CP949)로 파일을 읽어 한글 바이트 시퀀스를 오파싱한다. 이로 인해 `C1075`(브레이스 불일치), `C2001`(문자열 내 줄바꿈), `C2039`(멤버 인식 실패) 등 연쇄 오류가 발생한다.
- 이를 방지하기 위해 `vcxproj`의 모든 빌드 구성 `<ClCompile>`에 `/utf-8` 플래그가 설정되어 있다(`<AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>`). BOM이 없는 파일도 MSVC가 UTF-8로 강제 인식한다.
