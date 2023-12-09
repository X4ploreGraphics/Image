#ifndef NITROS_IMAGE_CONVERSION_HPP
#define NITROS_IMAGE_CONVERSION_HPP

#include "image/image.hpp"
#include "image/image_export.h"
#include <gsl/span>

namespace nitros::image
{
    NIMAGE_EXPORT auto color_convert(const utils::ImageCpu  &src_image, utils::ImageCpu  &dest_image) -> bool;

    //Handles only one plane now
    NIMAGE_EXPORT auto flip_y(const utils::ImageCpu &src_image, utils::ImageCpu  &dest_image) -> bool;
} // namespace nitros::image

#endif