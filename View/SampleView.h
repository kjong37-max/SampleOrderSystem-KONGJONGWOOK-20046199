#pragma once
#include "../Model/Sample.h"
#include <span>
#include <string>
#include <vector>

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
