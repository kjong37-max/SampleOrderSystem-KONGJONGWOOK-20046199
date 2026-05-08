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
