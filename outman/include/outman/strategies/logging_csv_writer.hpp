#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <ctime>
#include <thread>

#include "outman/strategies/base_saving_strategy.hpp"

class LoggingCsvFileWriter : public BaseAsyncSavingStrategy<std::string> {
public:
    explicit LoggingCsvFileWriter(const std::string& file_name, std::shared_ptr<BaseAsyncSavingStrategy<std::string>> strategy)
        : file_name_(file_name), strategy_(strategy) {}

    void SaveAsync(const std::string& data, const void* sender) override {
        auto start_time = std::chrono::system_clock::now();
        auto start_time_c = std::chrono::system_clock::to_time_t(start_time);
        std::thread::id thread_id = std::this_thread::get_id();

        {
            std::lock_guard<std::mutex> lock(log_mutex_);
            std::cout << "Saving started at " << std::ctime(&start_time_c) << " in thread " << thread_id << " for file " << file_name_ << std::endl;
        }

        strategy_->SaveAsync(data, this);

        auto end_time = std::chrono::system_clock::now();
        auto end_time_c = std::chrono::system_clock::to_time_t(end_time);

        {
            std::lock_guard<std::mutex> lock(log_mutex_);
            std::cout << "Saving finished at " << std::ctime(&end_time_c) << " in thread " << thread_id << " for file " << file_name_ << std::endl;
        }
    }

private:
    std::string file_name_;
    std::shared_ptr<BaseAsyncSavingStrategy<std::string>> strategy_;
    std::mutex log_mutex_;
};
