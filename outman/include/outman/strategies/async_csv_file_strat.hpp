#pragma once

#include <string>
#include <fstream>
#include "outman/strategies/isaving_strategy.hpp"
#include "outman/outman.hpp"

template <typename TData>
class AsyncCsvFileStrat : public IAsyncSavingStrategy<TData> {
public:
    AsyncCsvFileStrat(const std::string& file_name) : file_name_(file_name) {}

    void SaveAsync(const TData& data, const void* sender) override { 
        outman::om::Log("SaveAsync", static_cast<const void*>(&file_name_));
        std::ofstream file(file_name_, std::ios_base::app);
        file << data << std::endl;
    }

private:
    std::string file_name_;
};
