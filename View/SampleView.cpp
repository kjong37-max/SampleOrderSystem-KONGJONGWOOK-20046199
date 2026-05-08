#include "SampleView.h"
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

// ============================================================
// showMenu
// ============================================================
void SampleView::showMenu() const
{
    std::cout << "============================================================\n";
    std::cout << " [1] 시료 관리\n";
    std::cout << "------------------------------------------------------------\n";
    std::cout << " [1] 시료 등록   [2] 시료 목록   [3] 시료 검색   [0] 위로\n";
    std::cout << "선택 > ";
}

// ============================================================
// promptSampleInput
// ============================================================
Sample SampleView::promptSampleInput() const
{
    while (true) {
        Sample s;

        // 시료 ID
        std::cout << "시료 ID > ";
        std::cin >> s.id;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (s.id.empty()) {
            showError("시료 ID를 입력해 주세요.");
            continue;
        }

        // 시료명
        std::cout << "시료명 > ";
        std::getline(std::cin, s.name);
        if (s.name.empty()) {
            showError("시료명을 입력해 주세요.");
            continue;
        }

        // 평균 생산시간 (0 초과)
        bool avgOk = false;
        while (!avgOk) {
            std::cout << "평균 생산시간 (min/ea, 0 초과) > ";
            std::string line;
            std::getline(std::cin, line);
            std::istringstream iss(line);
            double v{};
            if ((iss >> v) && v > 0.0) {
                s.avgProdTime = v;
                avgOk = true;
            } else {
                showError("0보다 큰 숫자를 입력해 주세요.");
            }
        }

        // 수율 (0 초과 1 이하)
        bool yieldOk = false;
        while (!yieldOk) {
            std::cout << "수율 (0 초과 1 이하) > ";
            std::string line;
            std::getline(std::cin, line);
            std::istringstream iss(line);
            double v{};
            if ((iss >> v) && v > 0.0 && v <= 1.0) {
                s.yield = v;
                yieldOk = true;
            } else {
                showError("0 초과 1 이하의 숫자를 입력해 주세요.");
            }
        }

        // 초기 재고 (0 이상)
        bool stockOk = false;
        while (!stockOk) {
            std::cout << "초기 재고 (0 이상) > ";
            std::string line;
            std::getline(std::cin, line);
            std::istringstream iss(line);
            int v{};
            if ((iss >> v) && v >= 0) {
                s.stock = v;
                stockOk = true;
            } else {
                showError("0 이상의 정수를 입력해 주세요.");
            }
        }

        // 확인 화면
        std::cout << "------------------------------------------------------------\n";
        std::cout << "  시료 ID      : " << s.id       << "\n";
        std::cout << "  시료명       : " << s.name     << "\n";
        std::cout << "  평균 생산시간: " << s.avgProdTime << " min/ea\n";
        std::cout << "  수율         : " << s.yield    << "\n";
        std::cout << "  초기 재고    : " << s.stock    << " ea\n";
        std::cout << "------------------------------------------------------------\n";
        std::cout << "등록하시겠습니까? (Y/N) > ";
        std::string confirm;
        std::getline(std::cin, confirm);
        if (!confirm.empty() && (confirm[0] == 'Y' || confirm[0] == 'y'))
            return s;
        // N이면 처음부터 재입력
    }
}

// ============================================================
// 내부 헬퍼: 목록을 5개씩 페이지로 출력
// ============================================================
namespace {
void printSampleListPaged(const Sample* data, std::size_t size)
{
    constexpr int PAGE_SIZE = 5;
    int total = static_cast<int>(size);
    int page  = 0;

    while (true) {
        int start = page * PAGE_SIZE;
        int end   = std::min(start + PAGE_SIZE, total);

        std::cout << "============================================================\n";
        std::cout << std::left
                  << std::setw(10) << "ID"
                  << std::setw(20) << "시료명"
                  << std::setw(14) << "생산시간"
                  << std::setw(10) << "수율"
                  << std::setw(8)  << "재고"
                  << "\n";
        std::cout << "------------------------------------------------------------\n";

        for (int i = start; i < end; ++i) {
            const Sample& s = data[i];
            std::cout << std::left
                      << std::setw(10) << s.id
                      << std::setw(20) << s.name
                      << std::setw(14) << s.avgProdTime
                      << std::setw(10) << s.yield
                      << std::setw(8)  << s.stock
                      << "\n";
        }

        std::cout << "------------------------------------------------------------\n";
        std::cout << "페이지 " << (page + 1) << " / "
                  << ((total + PAGE_SIZE - 1) / PAGE_SIZE) << "\n";

        bool hasNext = (end < total);
        if (hasNext)
            std::cout << "[N] 다음 페이지   [0] 위로\n선택 > ";
        else
            std::cout << "[0] 위로\n선택 > ";

        std::string line;
        std::getline(std::cin, line);
        if (!line.empty() && (line[0] == 'N' || line[0] == 'n') && hasNext)
            ++page;
        else if (line == "0")
            break;
        else if (!hasNext)
            break;
    }
}
} // namespace

// ============================================================
// showSampleList
// ============================================================
void SampleView::showSampleList(std::span<const Sample> samples) const
{
    printSampleListPaged(samples.data(), samples.size());
}

// ============================================================
// showSearchResult
// ============================================================
void SampleView::showSearchResult(const std::vector<Sample>& samples) const
{
    printSampleListPaged(samples.data(), samples.size());
}

// ============================================================
// showSampleDetail
// ============================================================
void SampleView::showSampleDetail(const Sample& sample) const
{
    std::cout << "============================================================\n";
    std::cout << "  시료 ID      : " << sample.id          << "\n";
    std::cout << "  시료명       : " << sample.name        << "\n";
    std::cout << "  평균 생산시간: " << sample.avgProdTime << " min/ea\n";
    std::cout << "  수율         : " << sample.yield       << "\n";
    std::cout << "  재고         : " << sample.stock       << " ea\n";
    std::cout << "============================================================\n";
}

// ============================================================
// promptSearchKeyword
// ============================================================
std::string SampleView::promptSearchKeyword() const
{
    std::cout << "검색어 입력 > ";
    std::string kw;
    std::getline(std::cin, kw);
    return kw;
}

// ============================================================
// showMessage / showError
// ============================================================
void SampleView::showMessage(const std::string& msg) const
{
    std::cout << msg << "\n";
}

void SampleView::showError(const std::string& msg) const
{
    std::cout << "[오류] " << msg << "\n";
}

// ============================================================
// promptMenuChoice
// ============================================================
int SampleView::promptMenuChoice() const
{
    std::string line;
    std::getline(std::cin, line);
    std::istringstream iss(line);
    int choice{};
    if (iss >> choice)
        return choice;
    return -1;
}
