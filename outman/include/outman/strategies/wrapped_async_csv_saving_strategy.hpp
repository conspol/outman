#ifndef OUTMAN_ASYNC_CSV_SAVING_STRATEGY_HPP
#define OUTMAN_ASYNC_CSV_SAVING_STRATEGY_HPP

#include <memory>
#include <future>
#include <deque>
#include <condition_variable>
#include "iwrapped_sync_saving_strategy.hpp"
#include "csv_saving_strategy.hpp"

template <typename TData>
class WrappedAsyncCsvSavingStrategy : public IWrappedSyncSavingStrategy<TData> {
public:
    explicit WrappedAsyncCsvSavingStrategy(const std::string& outputFilePath);
    virtual ~WrappedAsyncCsvSavingStrategy();

    // Save the data asynchronously, wrapping the CsvSavingStrategy
    void SaveAsync(const TData& data, void* sender) override;

private:
    // Worker function to process the save queue
    void ProcessSaveQueue();

    // Wrapped CsvSavingStrategy
    CsvSavingStrategy<TData> _csvSavingStrategy;

    // Save queue and related synchronization primitives
    std::deque<std::pair<TData, void*>> _saveQueue;
    std::mutex _queueMutex;
    std::condition_variable _queueCondition;

    // Thread to process the save queue
    std::thread _workerThread;

    // Flag to control the worker thread's lifetime
    std::atomic<bool> _stopWorkerThread;
};

// #include "async_csv_saving_strategy.tpp"

template <typename TData>
WrappedAsyncCsvSavingStrategy<TData>::WrappedAsyncCsvSavingStrategy(const std::string& outputFilePath)
    : _csvSavingStrategy(outputFilePath), _stopWorkerThread(false) {
    _workerThread = std::thread(&AsyncCsvSavingStrategy<TData>::ProcessSaveQueue, this);
}

template <typename TData>
WrappedAsyncCsvSavingStrategy<TData>::~WrappedAsyncCsvSavingStrategy() {
    // Signal the worker thread to stop and wait for it to finish
    _stopWorkerThread.store(true);
    _queueCondition.notify_one();
    if (_workerThread.joinable()) {
        _workerThread.join();
    }
}

template <typename TData>
void WrappedAsyncCsvSavingStrategy<TData>::SaveAsync(const TData& data, void* sender) {
    // Add the data to the save queue and notify the worker thread
    std::unique_lock<std::mutex> lock(_queueMutex);
    _saveQueue.emplace_back(data, sender);
    lock.unlock();
    _queueCondition.notify_one();
}

template <typename TData>
void WrappedAsyncCsvSavingStrategy<TData>::ProcessSaveQueue() {
while (!_stopWorkerThread.load()) {
std::pair<TData, void*> dataToSave;
    {
        std::unique_lock<std::mutex> lock(_queueMutex);
        
        // Wait for data in the queue or a stop signal
        _queueCondition.wait(lock, [this]() {
            return !_saveQueue.empty() || _stopWorkerThread.load();
        });

        // Stop the worker thread if requested
        if (_stopWorkerThread.load()) {
            break;
        }

        // Get the data from the front of the queue and remove it
        dataToSave = _saveQueue.front();
        _saveQueue.pop_front();
    }

    // Save the data using the wrapped CsvSavingStrategy
    _csvSavingStrategy.Save(dataToSave.first, dataToSave.second);

#endif  // OUTMAN_ASYNC_CSV_SAVING_STRATEGY_HPP
