

#include "image/image.hpp"
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include "image/fileio.hpp"
#include <gsl/gsl>

namespace nitros::utils::image
{
    // read image
    ImageCpu read_image(const std::string& full_source_path)
    {
        int width, height, channels;

        //read the image from the given source path
        auto *image_data_ptr = stbi_load(full_source_path.c_str(), &width, &height, &channels, 4);

        //create an image that is of the same size as the source image
        // it returns a buffer of the same size
        auto created_image = nitros::utils::image::create_cpu({gsl::narrow_cast<std::size_t>(width), gsl::narrow_cast<std::size_t>(height)}, nitros::utils::pixel::RGBA8::value);

        auto rgba8_format = pixel::RGBA8::value;

        //assign the contents of the read image to the newly created empty image of the same size
        nitros::utils::image::assign({gsl::narrow_cast<std::size_t>(width), gsl::narrow_cast<std::size_t>(height)}, 
                                      width * rgba8_format.pixel_layout.bytes/rgba8_format.pixel_layout.group_pixels, 
                                      {image_data_ptr, gsl::narrow_cast<std::ptrdiff_t>(width * height * rgba8_format.pixel_layout.bytes/rgba8_format.pixel_layout.group_pixels)},
                                      created_image.meta_data().steps.at(0),
                                      created_image.buffer() );
        delete image_data_ptr;
        return created_image;
    }

    ImageCpu  decode_image(const gsl::span<std::uint8_t>  &data)
    {
        int width, height, channels;
        auto *image_data_ptr = stbi_load_from_memory(data.data(), data.size_bytes(), &width, &height, &channels, 4);

        auto created_image = nitros::utils::image::create_cpu({gsl::narrow_cast<std::size_t>(width), gsl::narrow_cast<std::size_t>(height)}, nitros::utils::pixel::RGBA8::value);

        auto rgba8_format = pixel::RGBA8::value;

        //assign the contents of the read image to the newly created empty image of the same size
        nitros::utils::image::assign({gsl::narrow_cast<std::size_t>(width), gsl::narrow_cast<std::size_t>(height)}, 
                                      width * rgba8_format.pixel_layout.bytes/rgba8_format.pixel_layout.group_pixels, 
                                      {image_data_ptr, gsl::narrow_cast<std::ptrdiff_t>(width * height * rgba8_format.pixel_layout.bytes/rgba8_format.pixel_layout.group_pixels)},
                                      created_image.meta_data().steps.at(0),
                                      created_image.buffer() );
        delete image_data_ptr;
        return created_image;
    }

    // write image in png format
    void write_image_png(const std::string& full_target_path, const ImageCpu& source_image)
    {
        stbi_write_png(full_target_path.c_str(), 
                       source_image.meta_data().size.width, 
                       source_image.meta_data().size.height,
                       source_image.meta_data().format.pixel_layout.channels,
                       source_image.buffer().data(), 
                       source_image.meta_data().step);
    } 

    // write image in jpg format
    void write_image_jpg(const std::string& full_target_path, const ImageCpu& source_image)
    {
        stbi_write_jpg(full_target_path.c_str(), 
                       source_image.meta_data().size.width, 
                       source_image.meta_data().size.height,
                       source_image.meta_data().format.pixel_layout.channels,
                       source_image.buffer().data(), 
                       100);
    }

    // write image in bmp format
    void write_image_bmp(const std::string& full_target_path, const ImageCpu& source_image)
    {
        stbi_write_bmp(full_target_path.c_str(), 
                       source_image.meta_data().size.width, 
                       source_image.meta_data().size.height,
                       source_image.meta_data().format.pixel_layout.channels,
                       source_image.buffer().data());
    }

}
