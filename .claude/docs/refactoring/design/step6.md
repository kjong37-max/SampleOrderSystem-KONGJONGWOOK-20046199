# Step 6 — View virtual 선언 일관화 설계

> 참고: [PLAN.md](../PLAN.md) | 브랜치: `refactoring`  
> 구현 전 **사용자 검토 후 commit** 규칙 준수

---

## 목표

`OrderView`만 virtual로 선언된 현재 상태를 전체 View에 일관 적용한다.  
이 변경은 이후 Controller 완전 Mock 테스트를 위한 전제 조건이다.  
**동작 변경 없음. 기존 테스트 모두 계속 통과해야 한다.**

---

## 현재 상태

| View | virtual 소멸자 | virtual 메서드 |
|------|--------------|---------------|
| `OrderView` | ✅ | ✅ |
| `SampleView` | ❌ | ❌ |
| `ProductionView` | ❌ | ❌ |
| `MonitorView` | ❌ | ❌ |

---

## 생성/수정 파일 목록

| 파일 | 작업 |
|------|------|
| `View/SampleView.h` | virtual 소멸자 + 모든 public 메서드에 virtual 추가 |
| `View/ProductionView.h` | 동일 |
| `View/MonitorView.h` | 동일 |

---

## 수정 명세

### View/SampleView.h

```cpp
class SampleView {
public:
    virtual ~SampleView() = default;
    virtual void showMenu() const;
    virtual Sample promptSampleInput() const;
    virtual void showSampleList(std::span<const Sample> samples) const;
    virtual void showSearchResult(const std::vector<Sample>& samples) const;
    virtual void showSampleDetail(const Sample& sample) const;
    virtual std::string promptSearchKeyword() const;
    virtual void showMessage(const std::string& msg) const;
    virtual void showError(const std::string& msg) const;
    virtual int  promptMenuChoice() const;
};
```

### View/ProductionView.h

```cpp
class ProductionView {
public:
    virtual ~ProductionView() = default;
    virtual void showMenu() const;
    virtual void showProductionStatus(
        const std::optional<ProductionJob>& current,
        const std::vector<ProductionJob>&   pending) const;
    virtual void showCompleteResult(const ProductionJob& job) const;
    virtual void showMessage(const std::string& msg) const;
    virtual void showError(const std::string& msg)   const;
    virtual int  promptMenuChoice()                  const;
};
```

### View/MonitorView.h

```cpp
class MonitorView {
public:
    MonitorView();
    virtual ~MonitorView();   // ← 기존 소멸자를 virtual로 변경
    virtual void showMenu() const;
    virtual void showOrderStats(int reserved, int confirmed,
                                int producing, int released) const;
    virtual void showStockStats(const std::vector<Sample>& samples,
                                const std::vector<StockStatus>& statuses) const;
    virtual void showMessage(const std::string& msg) const;
    virtual int  promptMenuChoice() const;
    // private 멤버는 변경 없음
};
```

> MonitorView는 생성자/소멸자가 이미 있으므로 `= default` 대신 기존 선언에 `virtual` 추가.

---

## Step 6 완료 조건

- [ ] `SampleView.h`, `ProductionView.h`, `MonitorView.h` 수정 완료
- [ ] 빌드 성공 (경고 없음)
- [ ] 전체 테스트 통과
- [ ] 사용자 검토 완료 후 commit
