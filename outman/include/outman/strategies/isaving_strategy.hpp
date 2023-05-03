#ifndef I_SAVING_STRATEGY_HPP
#define I_SAVING_STRATEGY_HPP

#include <memory>

class ISavingStrategyBase {
public:
    virtual ~ISavingStrategyBase() = default;
}; 

template <typename TData>
class ISavingStrategy : public ISavingStrategyBase {
public:
    virtual ~ISavingStrategy() = default;
    virtual void Save(const TData& data, const void* sender) = 0;
};

template <typename TData>
class IWrappedSyncSavingStrategy : public ISavingStrategy<TData> {
public:
    virtual ~IWrappedSyncSavingStrategy() = default;

    virtual void SaveAsync(const TData& data, const void* sender) = 0;
};

template <typename TData>
class IAsyncSavingStrategy : public ISavingStrategy<TData> {
public:
    virtual ~IAsyncSavingStrategy() = default;

    virtual void SaveAsync(const TData& data, const void* sender) = 0;

    void Save(const TData& data, const void* sender) override {
        SaveAsync(data, sender); // You can call SaveAsync directly, or you can provide a different implementation for synchronous saving.
    }
};

template <typename TData>
class IFlushableSavingStrategy : public ISavingStrategy<TData> {
public:
    virtual ~IFlushableSavingStrategy() = default;

    virtual void AddAsync(const TData& data, const void* sender) = 0;
    virtual void FlushAsync(const void* sender) = 0;
};

#endif // I_SAVING_STRATEGY_HPP
