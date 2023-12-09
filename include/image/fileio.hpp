

#ifndef IMAGE_FILEIO_HPP
#define IMAGE_FILEIO_HPP

#include "image.hpp"
#include "image/image_export.h"
#include <gsl/span>

namespace nitros::utils
{
    namespace image
    {
        NIMAGE_EXPORT ImageCpu read_image(const std::string& full_source_path);
        NIMAGE_EXPORT ImageCpu decode_image(const gsl::span<std::uint8_t>  &data);

        NIMAGE_EXPORT void write_image_png(const std::string& full_target_path, const ImageCpu& source_image);
        NIMAGE_EXPORT void write_image_jpg(const std::string& full_target_path, const ImageCpu& source_image);
        NIMAGE_EXPORT void write_image_bmp(const std::string& full_target_path, const ImageCpu& source_image);
    }

}

#endif