#include "ProductionLine.h"

void ProductionLine::enqueue(const ProductionJob& job) {
    if (!current_) {
        current_ = job;
    } else {
        queue_.push(job);
    }
}

std::optional<ProductionJob> ProductionLine::currentJob() const {
    return current_;
}

std::vector<ProductionJob> ProductionLine::pendingJobs() const {
    std::vector<ProductionJob> result;
    std::queue<ProductionJob> tmp = queue_;
    while (!tmp.empty()) {
        result.push_back(tmp.front());
        tmp.pop();
    }
    return result;
}

int ProductionLine::pendingCount() const {
    return static_cast<int>(queue_.size());
}

std::optional<ProductionJob> ProductionLine::completeCurrentJob() {
    if (!current_) return std::nullopt;
    ProductionJob done = *current_;
    if (!queue_.empty()) {
        current_ = queue_.front();
        queue_.pop();
    } else {
        current_ = std::nullopt;
    }
    return done;
}

bool ProductionLine::isRunning() const {
    return current_.has_value();
}
