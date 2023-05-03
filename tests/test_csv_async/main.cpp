#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <fstream>
#include <sstream>
#include <random>
#include <thread>
#include <future>
#include <filesystem>

#include <outman/outman.hpp>
#include "outman/all_strats.hpp"
#include "outman/outloggers/cout_logger.hpp"

std::string generate_csv_data(size_t rows, size_t columns) {
    std::ostringstream oss;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);
    
    int col1 = columns - 1;

	for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < col1; ++j) {
            oss << dis(gen) << ",";
        }
        oss << dis(gen) << "\n";
    }

    return oss.str();
}

int main() {
    outman::OutputManager& manager = outman::OutputManager::Instance();
	auto logger = std::make_shared<CoutLogger>();
	manager.SetLogger(logger);

    for (int i = 1; i <= 10; ++i) {
        // Delete the file if it exists
        std::filesystem::path file_path("big_csv_file_" + std::to_string(i) + ".csv");
        std::filesystem::remove(file_path);

        auto csv_writer = std::make_shared<AsyncCsvFileStrat<std::string>>(file_path.string());
        auto logging_csv_writer = std::make_shared<AsyncSaveStratLogs<std::string>>(csv_writer, logger);
        auto csv_strategy = std::static_pointer_cast<ISavingStrategy<std::string>>(logging_csv_writer);
        manager.AddStrategy(csv_strategy);
    }

    std::vector<std::string> csv_data;
    for (int i = 0; i < 3; ++i) {
        csv_data.push_back(generate_csv_data(50000, 30));
		logger->Log("Generated csv data " + std::to_string(i),
             new std::string("test_main_" +
                 std::to_string(std::hash<std::thread::id>{}(
                     std::this_thread::get_id()))));
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    // Save the big CSV files asynchronously using OutputManager
	for (const auto& data : csv_data) {
		//save_futures.emplace_back(std::async(std::launch::async, [&manager, &data] {
		manager.SaveAsync(data, new std::string("test_main_" + std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id()))));
		auto timenow_ = std::chrono::high_resolution_clock::now();
		logger->Log("SaveAsync called", static_cast<const void*>(&manager));
	}

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

    logger->Log("Asynchronously called to save big CSV files in " +
        std::to_string(duration) + " us", static_cast<const void*>(&manager));

    for (int i = 0; i < 5; ++i) {
        csv_data.push_back(generate_csv_data(10000, 20));
		logger->Log("Generated csv data " + std::to_string(i),
             new std::string("test_main_" +
                 std::to_string(std::hash<std::thread::id>{}(
                     std::this_thread::get_id()))));
    }

    return 0;
}
