#pragma once
#include "ProductionJob.h"
#include <optional>
#include <queue>
#include <vector>

class ProductionLine {
public:
    void enqueue(const ProductionJob& job);
    std::optional<ProductionJob> currentJob()        const;
    std::vector<ProductionJob>   pendingJobs()       const;
    int                          pendingCount()      const;
    std::optional<ProductionJob> completeCurrentJob();
    bool                         isRunning()         const;

private:
    std::optional<ProductionJob> current_;
    std::queue<ProductionJob>    queue_;
};
