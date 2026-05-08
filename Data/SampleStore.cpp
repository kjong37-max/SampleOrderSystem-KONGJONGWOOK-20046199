#include "SampleStore.h"

SampleStore::SampleStore(const std::string& filePath)
    : filePath_(filePath)
{
}

void SampleStore::load(SampleModel& model) {
    DataStore store(filePath_);
    store.load();

    int count = 0;
    try { count = std::stoi(store.get("sample.count", "0")); } catch (...) {}

    for (int i = 0; i < count; ++i) {
        std::string prefix = "sample." + std::to_string(i);
        Sample s;
        s.id   = store.get(prefix + ".id");
        s.name = store.get(prefix + ".name");
        try { s.avgProdTime = std::stod(store.get(prefix + ".avgProdTime", "0")); } catch (...) {}
        try { s.yield       = std::stod(store.get(prefix + ".yield",       "0")); } catch (...) {}
        try { s.stock       = std::stoi(store.get(prefix + ".stock",       "0")); } catch (...) {}
        if (!s.id.empty()) model.add(s);
    }
}

void SampleStore::save(const SampleModel& model) const {
    DataStore store(filePath_);
    auto samples = model.all();
    store.set("sample.count", std::to_string(samples.size()));
    for (int i = 0; i < static_cast<int>(samples.size()); ++i) {
        std::string prefix = "sample." + std::to_string(i);
        store.set(prefix + ".id",          samples[i].id);
        store.set(prefix + ".name",        samples[i].name);
        store.set(prefix + ".avgProdTime", std::to_string(samples[i].avgProdTime));
        store.set(prefix + ".yield",       std::to_string(samples[i].yield));
        store.set(prefix + ".stock",       std::to_string(samples[i].stock));
    }
    store.save();
}
