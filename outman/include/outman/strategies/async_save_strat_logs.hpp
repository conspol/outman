#pragma once

#include "isaving_strategy.hpp"
#include "../outloggers/iout_logger.hpp"

template <typename TData>
class AsyncSaveStratLogs : public IAsyncSavingStrategy<TData> {
public:
    AsyncSaveStratLogs(std::shared_ptr<IAsyncSavingStrategy<TData>> inner_strategy,
                       std::shared_ptr<IOutLogger> logger)
        : inner_strategy_(inner_strategy), logger_(logger) {}

    void SaveAsync(const TData& data, const void* sender) override {
        inner_strategy_->SaveAsync(data, sender);
        logger_->Log("Data saved", sender);
    }

private:
    std::shared_ptr<IAsyncSavingStrategy<TData>> inner_strategy_;
    std::shared_ptr<IOutLogger> logger_;
};

