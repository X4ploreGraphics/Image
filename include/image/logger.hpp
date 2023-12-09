

#ifndef NITROS_UTILS_LOGGER_HPP
#define NITROS_UTILS_LOGGER_HPP

#include "image/image_export.h"
#include <string>
#include <spdlog/spdlog.h>

namespace nitros::image
{
	NIMAGE_EXPORT auto setup_logger(spdlog::sinks_init_list sinks) -> std::shared_ptr<spdlog::logger>;	
}

#endif