#include "SampleModel.h"
#include <algorithm>
#include <cctype>
#include <numeric>
#include <ranges>

bool SampleModel::add(const Sample& sample)
{
    auto it = std::ranges::find_if(samples_,
        [&](const Sample& s) { return s.id == sample.id; });
    if (it != samples_.end())
        return false;
    samples_.push_back(sample);
    return true;
}

std::optional<Sample> SampleModel::findById(const std::string& id) const
{
    auto it = std::ranges::find_if(samples_,
        [&](const Sample& s) { return s.id == id; });
    if (it == samples_.end())
        return std::nullopt;
    return *it;
}

std::vector<Sample> SampleModel::findByName(const std::string& keyword) const
{
    auto toLower = [](std::string s) {
        std::ranges::transform(s, s.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return s;
    };

    const std::string lowerKeyword = toLower(keyword);

    std::vector<Sample> result;
    for (const auto& s : samples_) {
        if (toLower(s.name).find(lowerKeyword) != std::string::npos)
            result.push_back(s);
    }
    return result;
}

std::span<const Sample> SampleModel::all() const
{
    return std::span<const Sample>(samples_);
}

bool SampleModel::updateStock(const std::string& id, int delta)
{
    auto it = std::ranges::find_if(samples_,
        [&](const Sample& s) { return s.id == id; });
    if (it == samples_.end())
        return false;
    if (it->stock + delta < 0)
        return false;
    it->stock += delta;
    return true;
}

int SampleModel::totalStock() const
{
    return std::accumulate(samples_.begin(), samples_.end(), 0,
        [](int sum, const Sample& s) { return sum + s.stock; });
}

int SampleModel::count() const
{
    return static_cast<int>(samples_.size());
}
