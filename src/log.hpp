

#ifndef _NITROS_UTILS_INTERNAL_LOG_HPP
#define _NITROS_UTILS_INTERNAL_LOG_HPP

#include <spdlog/spdlog.h>

namespace nitros::utils::log
{
    auto Logger() -> std::shared_ptr<spdlog::logger>;
} // namespace nitros::utils::log


#endif