#pragma once

#define BOOST_ALL_NO_LIB
#define BOOST_ASIO_STANDALONE

#include "output_manager.hpp"
#include "all_strats.hpp" 

namespace outman {
    namespace om {
        template <typename TData>
        inline void Save(const TData& data) {
            OutputManager::Instance().Save(data);
        }

		inline void Log(const std::string& message, const void* sender) {
            OutputManager::Instance().Log(message, sender);
        }
    }
}
