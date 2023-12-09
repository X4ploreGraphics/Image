

#include "image/logger.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/null_sink.h>
#include <string_view>
#include "log.hpp"

namespace nitros::utils
{
    std::shared_ptr<spdlog::logger> setup_logger(spdlog::sinks_init_list sinks)
    {
        const auto logger_string = std::string{ "image_utils" };
        auto logger = spdlog::get(logger_string);
        if(!logger)
        {
            if(sinks.size() > 0) {
                logger = std::make_shared<spdlog::logger>(logger_string, sinks.begin(), sinks.end());
                spdlog::register_logger(logger);
            }
            else {
                logger = spdlog::null_logger_mt(logger_string);
            }
        }

        return logger;
    }

    namespace log
    {
        auto Logger() -> std::shared_ptr<spdlog::logger>{
            auto logger = spdlog::get( std::string{"image_utils"} );
            if(logger)
                return logger;
            else
                return setup_logger({});
        }
    }

}
