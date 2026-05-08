#pragma once
#include "../Model/ProductionJob.h"
#include <optional>
#include <string>
#include <vector>

class ProductionView {
public:
    void showMenu() const;
    void showProductionStatus(
        const std::optional<ProductionJob>& current,
        const std::vector<ProductionJob>&   pending) const;
    void showCompleteResult(const ProductionJob& job) const;
    void showMessage(const std::string& msg) const;
    void showError(const std::string& msg)   const;
    int  promptMenuChoice()                  const;
};
