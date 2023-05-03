#pragma once

#include <iostream>
#include <chrono>
#include <mutex>
#include <iomanip>
#include <iostream> 
#include <ctime>
#include "iout_logger.hpp"

class CoutLogger : public IOutLogger {
public:
    CoutLogger() = default;
    virtual ~CoutLogger() = default;

    void Log(const std::string& message, const void* sender) override {
        static constexpr std::hash<std::thread::id> hash{};
        std::ostringstream ss_hash;
        auto thread_id_hash = hash(std::this_thread::get_id());
        int num_digits = 4; // The number of digits of thread id hash to display

        ss_hash << "0x" << std::hex << std::setw(num_digits) << std::setfill('0')
            << (thread_id_hash & ((1 << (num_digits * 4)) - 1)) << std::dec;

        std::string hash_str = ss_hash.str();

        std::string sender_info = "sender_unknown_";

        if (sender == static_cast<const void*>(&outman::OutputManager::Instance())) {
            sender_info = "outman_";
        }
        else {
            const std::string* sender_string = static_cast<const std::string*>(sender);
            if (sender_string) {
                sender_info = *sender_string;
            }
        }

        sender_info += hash_str;

        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        auto epoch_time = now.time_since_epoch();
        auto micros = std::chrono::duration_cast<std::chrono::microseconds>(epoch_time);
        std::time_t current_time = std::chrono::system_clock::to_time_t(now);
        std::tm local_time;

#if defined(_WIN32) || defined(_WIN64)
        localtime_s(&local_time, &current_time);
#else
        localtime_r(&current_time, &local_time);
#endif

        char buffer[80];
        strftime(buffer, 80, "%d%m%Y_%H%M%S", &local_time);

		std::lock_guard<std::mutex> lock(mutex_); 
        std::cout << buffer << "_" << std::setfill('0') << std::setw(6) << micros.count() % 1000000
            << ": " << sender_info << " - " << message << std::endl;
    }

private:
    std::mutex mutex_;
};

