#ifndef BASE_SAVING_STRATEGY_HPP
#define BASE_SAVING_STRATEGY_HPP

#include "isaving_strategy.hpp"
#include "outman/output_manager.hpp"

template <typename TData>
class BaseSavingStrategy : public IWrappedSyncSavingStrategy<TData> {
public:
    virtual ~BaseSavingStrategy() = default;

    virtual void Save(const TData& data, const void* sender) override = 0;

    virtual void SaveAsync(const TData& data, const void* sender) override {
        // Use the thread pool from OutputManager to execute Save
        OutputManager::Instance()._thread_pool.post([=] { Save(data, sender); });
    }
};

template <typename TData>
class BaseAsyncSavingStrategy : public IAsyncSavingStrategy<TData> {
public:
    virtual ~BaseAsyncSavingStrategy() = default;

    virtual void SaveAsync(const TData& data, const void* sender) override = 0;

    virtual void Save(const TData& data, const void* sender) {
        SaveAsync(data, sender); // Synchronous call
    }
};

template <typename TData>
class BaseFlushableSavingStrategy : public IFlushableSavingStrategy<TData> {
public:
    virtual ~BaseFlushableSavingStrategy() = default;

    virtual void AddAsync(const TData& data, const void* sender) override = 0;
    virtual void FlushAsync(const void* sender) override = 0;

    virtual void Save(const TData& data, const void* sender) override {
        AddAsync(data, sender);
        FlushAsync(sender);
    }
};

#endif // BASE_SAVING_STRATEGY_HPP
