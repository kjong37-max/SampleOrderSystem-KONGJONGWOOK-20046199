#pragma once
#include "Sample.h"
#include <optional>
#include <span>
#include <string>
#include <vector>

class SampleModel {
public:
    bool add(const Sample& sample);
    std::optional<Sample> findById(const std::string& id) const;
    std::vector<Sample> findByName(const std::string& keyword) const;
    std::span<const Sample> all() const;
    bool updateStock(const std::string& id, int delta);
    int totalStock() const;
    int count() const;

private:
    std::vector<Sample> samples_;
};
