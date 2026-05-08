#include "SampleController.h"

SampleController::SampleController(SampleModel& model, SampleView& view)
    : model_(model), view_(view)
{
}

void SampleController::run()
{
    while (true) {
        view_.showMenu();
        int choice = view_.promptMenuChoice();
        switch (choice) {
        case 1:
            handleAdd();
            break;
        case 2:
            handleList();
            break;
        case 3:
            handleSearch();
            break;
        case 0:
            return;
        default:
            view_.showError("유효하지 않은 메뉴 선택입니다.");
            break;
        }
    }
}

void SampleController::handleAdd()
{
    Sample s = view_.promptSampleInput();
    if (model_.add(s))
        view_.showMessage("시료가 등록되었습니다. (ID: " + s.id + ")");
    else
        view_.showError("이미 존재하는 시료 ID입니다: " + s.id);
}

void SampleController::handleList()
{
    if (model_.count() == 0)
        view_.showMessage("등록된 시료가 없습니다.");
    else
        view_.showSampleList(model_.all());
}

void SampleController::handleSearch()
{
    std::string kw = view_.promptSearchKeyword();
    auto results = model_.findByName(kw);
    if (results.empty())
        view_.showError("검색 결과가 없습니다.");
    else
        view_.showSearchResult(results);
}
