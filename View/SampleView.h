#pragma once
#include "../Model/Sample.h"
#include <span>
#include <string>
#include <vector>

class SampleView {
public:
    void showMenu() const;
    Sample promptSampleInput() const;
    void showSampleList(std::span<const Sample> samples) const;
    void showSearchResult(const std::vector<Sample>& samples) const;
    void showSampleDetail(const Sample& sample) const;
    std::string promptSearchKeyword() const;
    void showMessage(const std::string& msg) const;
    void showError(const std::string& msg) const;
    int promptMenuChoice() const;
};
