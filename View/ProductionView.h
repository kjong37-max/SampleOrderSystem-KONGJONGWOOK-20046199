#pragma once
#include "../Model/ProductionJob.h"
#include <optional>
#include <string>
#include <vector>

class ProductionView {
public:
    virtual ~ProductionView() = default;
    virtual void showMenu() const;
    virtual void showProductionStatus(
        const std::optional<ProductionJob>& current,
        const std::vector<ProductionJob>&   pending) const;
    virtual void showCompleteResult(const ProductionJob& job) const;
    virtual void showMessage(const std::string& msg) const;
    virtual void showError(const std::string& msg)   const;
    virtual int  promptMenuChoice()                  const;
};
