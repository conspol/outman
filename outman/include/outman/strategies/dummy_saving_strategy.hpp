#ifndef DUMMY_SAVING_STRATEGY_HPP
#define DUMMY_SAVING_STRATEGY_HPP

#include <iostream>
#include <thread>
#include <chrono>
#include "outman/isaving_strategy.hpp"

class DummySavingStrategy : public ISavingStrategy<int> {
public:
    void Save(const int& data) override {
        auto thread_id = std::this_thread::get_id();
        std::cout << "Executing strategy on thread: " << thread_id << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::cout << "Strategy completed on thread: " << thread_id << std::endl;
    }
};

#endif  // DUMMY_SAVING_STRATEGY_HPP
