#ifndef OUTPUT_MANAGER_HPP
#define OUTPUT_MANAGER_HPP

#ifndef BOOST_ALL_NO_LIB 
#define BOOST_ALL_NO_LIB
#endif // !BOOST_ALL_NO_LIB 


#include <memory>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <typeinfo>
#include <shared_mutex>
#include <boost/asio.hpp>
#include <boost/asio/thread_pool.hpp>

#include "outloggers/iout_logger.hpp"
#include "strategies/isaving_strategy.hpp"

namespace outman {
    class OutputManager {
    public:
        static OutputManager& Instance();

        template <typename TData>
        void AddStrategy(std::shared_ptr<ISavingStrategy<TData>> strategy);

        template <typename TData>
        void Save(const TData& data, const void* sender);

        template <typename TData>
        void SaveAsync(const TData& data, const void* sender);

        template <typename TData>
        void RegisterFlushableStrategy(const std::shared_ptr<IFlushableSavingStrategy<TData>>& strategy, std::chrono::milliseconds interval);

        void FlushTimerCallback();
        void SetLogger(const std::shared_ptr<IOutLogger>& logger);
        void Log(const std::string& message, const void* sender);

    private:
        OutputManager();
        ~OutputManager();

        OutputManager(const OutputManager&) = delete;
        OutputManager& operator=(const OutputManager&) = delete;
        OutputManager(OutputManager&&) = delete;
        OutputManager& operator=(OutputManager&&) = delete;

        static inline std::once_flag _instance_flag;

        std::unordered_map<
            std::type_index,
            std::vector<std::shared_ptr<ISavingStrategyBase>>
        > _strategies;
        std::shared_mutex _strategies_mutex;

        std::unordered_map<
            std::type_index,
            std::vector<
            std::tuple<
            std::chrono::steady_clock::time_point,
            std::chrono::milliseconds,
            std::shared_ptr<ISavingStrategyBase>
            >>> _flushable_strategies;
        std::shared_mutex _flushable_strategies_mutex;

        boost::asio::io_context _io_context;
        boost::asio::executor_work_guard<boost::asio::io_context::executor_type> _work_guard;
        std::thread _io_thread;
        boost::asio::thread_pool _thread_pool;
        boost::asio::steady_timer _timer;

        template <typename TData>
        void ExecuteStrategyAsync(
            const std::shared_ptr<ISavingStrategy<TData>>& strategy,
            const TData& data,
            const void* sender
        );

        std::shared_ptr<IOutLogger> logger_;
    };

    OutputManager::OutputManager()
        : _io_context(),
        _work_guard(boost::asio::make_work_guard(_io_context)),
        _timer(_io_context),
        _thread_pool(3)
    {
        for (std::size_t i = 0; i < 3; ++i) {
            boost::asio::post(_thread_pool, [&]() { _io_context.run(); });
        }
        FlushTimerCallback();
    }

    // Destructor: clean up resources
    OutputManager::~OutputManager() {
        _work_guard.reset();
        _io_context.stop();
        if (_io_thread.joinable()) {
            _io_thread.join();
        }
    }

    // Corrected Instance() method
    OutputManager& OutputManager::Instance() {
        static OutputManager instance;
        std::call_once(_instance_flag, []() {
            new (&instance) OutputManager();
            });
        return instance;
    }

    template <typename TData>
    void OutputManager::AddStrategy(std::shared_ptr<ISavingStrategy<TData>> strategy) {
        auto index = std::type_index(typeid(TData));
        std::unique_lock lock(_strategies_mutex);
        _strategies[index].push_back(std::static_pointer_cast<ISavingStrategyBase>(strategy));
    }

    template <typename TData>
    void OutputManager::RegisterFlushableStrategy(
        const std::shared_ptr<IFlushableSavingStrategy<TData>>& strategy,
        std::chrono::milliseconds interval
    ) {
        auto key = std::type_index(typeid(TData));
        std::unique_lock<std::shared_mutex> lock(_flushable_strategies_mutex);
        auto& strategy_list = _flushable_strategies[key];
        strategy_list.emplace_back(
            std::make_tuple(std::chrono::steady_clock::now() + interval,
                interval,
                std::static_pointer_cast<ISavingStrategyBase>(strategy)
            ));
    }

    template <typename TData>
    void OutputManager::Save(const TData& data, const void* sender) {
        auto index = std::type_index(typeid(TData));
        std::shared_lock lock(_strategies_mutex);
        auto it = _strategies.find(index);
        if (it != _strategies.end()) {
            for (const auto& strategy_ptr : it->second) {
                auto strategy = std::static_pointer_cast<ISavingStrategy<TData>>(strategy_ptr);
                strategy->Save(data, sender);
            }
        }
    }

    template <typename TData>
    void OutputManager::SaveAsync(const TData& data, const void* sender) {
        auto index = std::type_index(typeid(TData));
        std::shared_lock lock(_strategies_mutex);
        auto it = _strategies.find(index);
        if (it != _strategies.end()) {
            for (const auto& strategy_ptr : it->second) {
                auto strategy = std::static_pointer_cast<ISavingStrategy<TData>>(strategy_ptr);
                ExecuteStrategyAsync(strategy, data, sender);
            }
        }
    }

    template <typename TData>
    void OutputManager::ExecuteStrategyAsync(
        const std::shared_ptr<ISavingStrategy<TData>>& strategy,
        const TData& data,
        const void* sender
    ) {
        auto save_handler = [strategy, data, sender](const boost::system::error_code& ec) {
            if (ec) {
                // Handle boost errors here
                std::cerr << "Boost error: " << ec << " - " << ec.message() << std::endl;
            }
            else {
                try {
                    if (auto wrapped_sync_strategy = std::dynamic_pointer_cast<IWrappedSyncSavingStrategy<TData>>(strategy)) {
                        wrapped_sync_strategy->SaveAsync(data, sender);
                    }
                    else if (auto async_strategy = std::dynamic_pointer_cast<IAsyncSavingStrategy<TData>>(strategy)) {
                        async_strategy->SaveAsync(data, sender);
                    }
                    else if (auto flushable_strategy = std::dynamic_pointer_cast<IFlushableSavingStrategy<TData>>(strategy)) {
                        flushable_strategy->AddAsync(data, sender);
                        flushable_strategy->FlushAsync(sender);
                    }
                    else {
                        // Unsupported strategy type
                    }
                }
                catch (const std::exception& e) {
                    // Handle other exceptions here
                    std::cerr << "Error: " << e.what() << std::endl;
                }
                catch (...) {
                    // Handle unknown exceptions here
                    std::cerr << "Unknown error occurred" << std::endl;
                }
            }
        };

        _io_context.post([save_handler]() {
            save_handler(boost::system::error_code());
		});
    }


    void OutputManager::FlushTimerCallback() {
        auto now = std::chrono::steady_clock::now();
        std::chrono::milliseconds min_interval = std::chrono::milliseconds::max();
        {
            std::unique_lock<std::shared_mutex> lock(_flushable_strategies_mutex);
            for (auto& item : _flushable_strategies) {
                for (auto& strategy_tuple : item.second) {
                    auto& next_flush_time = std::get<0>(strategy_tuple);
                    auto& interval = std::get<1>(strategy_tuple);
                    auto& strategy_ptr = std::get<2>(strategy_tuple);

                    if (now >= next_flush_time) {
                        auto strategy = std::dynamic_pointer_cast<
                            IFlushableSavingStrategy<std::remove_reference_t<decltype(item.first)>>
                        >(strategy_ptr);

                        if (strategy != nullptr) {
                            strategy->FlushAsync(nullptr);
                        }
                        next_flush_time = now + interval;
                    }

                    min_interval = std::min(min_interval, std::chrono::duration_cast<std::chrono::milliseconds>(next_flush_time - now));
                }
            }
        }

        // Schedule the next flush.
        _timer.expires_after(min_interval);
        _timer.async_wait([this](const boost::system::error_code error) {
            if (!error) {
                FlushTimerCallback();
            }
            });
    }

    void OutputManager::SetLogger(const std::shared_ptr<IOutLogger>& logger) {
        logger_ = logger;
    }

    void OutputManager::Log(const std::string& message, const void* sender) {
        if (logger_) {
            logger_->Log(message, sender);
        } 
    }
}

#endif // OUTPUT_MANAGER_HPP