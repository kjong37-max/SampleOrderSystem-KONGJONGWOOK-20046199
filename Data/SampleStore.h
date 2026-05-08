#pragma once
#include "DataStore.h"
#include "../Model/SampleModel.h"

class SampleStore {
public:
    explicit SampleStore(const std::string& filePath);

    void load(SampleModel& model);
    void save(const SampleModel& model) const;

private:
    std::string filePath_;
};
