
#include "image/image.hpp"
#include "image/pixel/read_write.hpp"
#include "image/process/conversion.hpp"
#include "conversion_internal.hpp"
#include <gsl/gsl>
#include <stdexcept>
#include <assert.h>
#include <utility>
#include <algorithm>

namespace nitros::image
{
    auto color_convert(const utils::ImageCpu  &src_image, utils::ImageCpu  &dest_image) -> bool
    {
        assert( src_image.meta_data().size == dest_image.meta_data().size );

        if(src_image.meta_data().format == utils::pixel::RGBA8::value &&
           dest_image.meta_data().format == utils::pixel::YUV444p::value)
        {
            rgb_to_yuv444<std::uint8_t, 4>(src_image.meta_data(), dest_image.meta_data(), src_image.buffer().data(), dest_image.buffer().data());
            return true;
        }
        else if(src_image.meta_data().format == utils::pixel::RGB8::value &&
           dest_image.meta_data().format == utils::pixel::YUV444p::value)
        {
            rgb_to_yuv444<std::uint8_t, 3>(src_image.meta_data(), dest_image.meta_data(), src_image.buffer().data(), dest_image.buffer().data());
            return true;
        }
        else if(src_image.meta_data().format == utils::pixel::YUV444p::value &&
                dest_image.meta_data().format == utils::pixel::RGBA8::value)
        {
            yuv444_to_rgb<std::uint8_t, 4>(src_image.meta_data(), dest_image.meta_data(), src_image.buffer().data(), dest_image.buffer().data());
            return true;
        }
        else if(src_image.meta_data().format == utils::pixel::YUV444p::value &&
                dest_image.meta_data().format == utils::pixel::RGB8::value)
        {
            yuv444_to_rgb<std::uint8_t, 3>(src_image.meta_data(), dest_image.meta_data(), src_image.buffer().data(), dest_image.buffer().data());
            return true;
        }

        if((src_image.meta_data().format.pixel_type == utils::pixel::type::rgba || src_image.meta_data().format.pixel_type == utils::pixel::type::rgb)
            && dest_image.meta_data().format.pixel_type == utils::pixel::type::yuv)
        {
            auto reader = utils::pixel::ReaderRGBA<std::uint8_t>{src_image};
            auto writer = utils::pixel::WriterYUV<std::uint8_t>{dest_image};

            while (auto px = reader.next())
            {
                auto yuv_px = utils::pixel::PixelYUV<std::uint8_t>{};
                rbg_yuv(px->r, px->g, px->b, &yuv_px.y, &yuv_px.cb, &yuv_px.cr);
                yuv_px.a = px->a;
                writer.next(yuv_px);
            }

            return true;
        }
        else if(src_image.meta_data().format.pixel_type == utils::pixel::type::yuv
            && (dest_image.meta_data().format.pixel_type == utils::pixel::type::rgba || dest_image.meta_data().format.pixel_type == utils::pixel::type::rgb))
        {
            auto reader = utils::pixel::ReaderYUV<std::uint8_t>{src_image};
            auto writer = utils::pixel::WriterRGBA<std::uint8_t>{dest_image};

            while (auto px = reader.next())
            {
                auto rgba_px = utils::pixel::PixelRGBA<std::uint8_t>{};
                yuv_to_rgb(px->y, px->cb, px->cr, &rgba_px.r, &rgba_px.g, &rgba_px.b);
                rgba_px.a = px->a;
                auto result = writer.next(rgba_px);
            }

            return true;
        }        

        return false;
    }

    auto flip_y(const utils::ImageCpu  &src_image, utils::ImageCpu  &dest_image) -> bool 
    {
        Expects( src_image.meta_data() == dest_image.meta_data() );

        auto img_size = src_image.meta_data().size;
        for(auto y = 0; y < src_image.meta_data().size.height; y++ )
        {
            auto dst_ptr = dest_image.buffer().data() + dest_image.meta_data().steps[0] * ( (dest_image.meta_data().size.height-1) - y  );
            auto src_ptr = src_image.buffer().data()  + src_image.meta_data().steps[0] * y ;

            memcpy( dst_ptr, src_ptr, src_image.meta_data().steps[0] );
        }
        return true;
    }
} // namespace nitros::image
