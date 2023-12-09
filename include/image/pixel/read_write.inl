

#include "read_write.hpp"
#include "image/utils.hpp"
#include <optional>

namespace nitros::utils
{
    namespace pixel
    {
        template <typename num_type_>
        Reader<num_type_>::Reader(const ImageCpu  &image) noexcept
            :_image{image}
            ,_x{0}
            ,_y{0}
            ,_start_address{}
        {
            for(auto i = 0; i < image.meta_data().format.planes.size(); i++) {
                _start_address.push_back( plane_start_address(_image, i) );
            }
        }

        template <typename num_type_>
        auto Reader<num_type_>::at(const utils::vec2Ui  &pt) const noexcept -> PixelData<num_type_>
        {
            using Pixel_t = PixelData<num_type_>;

            auto meta = _image.meta_data();
            auto format = meta.format;

            //If Planar
            if(!format.planar_info.is_planar) {
                return at_packed(pt);
            }
            else {
                return at_planar(pt);
            }
        }   

        template <typename num_type_>
        auto Reader<num_type_>::next() noexcept -> std::optional<PixelData<num_type_>>
        {
            auto pt = utils::vec2Ui{_x, _y};
            auto img_size = _image.meta_data().size;
            
            if(!(pt[0] < img_size.width) || !(pt[1] < img_size.height)) {
                return std::nullopt;
            }
            increment_point();
            return at(pt);
        }

        template <typename num_type_>
        void Reader<num_type_>::reset() noexcept
        {
            _x = 0;
            _y = 0;
        }

        template <typename num_type_>
        auto Reader<num_type_>::at_packed(const utils::vec2Ui  &pt) const noexcept -> PixelData<num_type_>
        {   
            auto& meta = _image.meta_data();
            auto& format = _image.meta_data().format;
            auto start_addr = _image.buffer().data() + pt[1] * meta.steps[0] + (pt[0] * format.pixel_layout.bytes / format.pixel_layout.group_pixels );
            
            auto ar = PixelData<num_type_>{};
            auto offset = std::size_t{};
            for(auto i = 0; i < format.planar_info.pixel_bits.size(); i++) 
            {
                ar.push_back( get_data_masked<num_type_>(start_addr, offset, format.planar_info.pixel_bits[i]) );
                offset += format.planar_info.pixel_bits[i];
            }

            return ar;
        }

        template <typename num_type_>
        auto Reader<num_type_>::at_planar(const utils::vec2Ui  &pt) const noexcept -> PixelData<num_type_>
        {
            auto& meta = _image.meta_data();
            auto& format = _image.meta_data().format;

            auto ar = PixelData<num_type_>{};
            auto pre_bits_index = 0u;
            for(auto i = 0; i < format.planes.size(); i++)
            {
                auto px_offset = plane_pixel_address(pt, _start_address[i], _image.meta_data(), i );

                for(auto j = 0, px_channel_offset = 0; j < format.planes[i].channels; j++) {
                    auto pixel_bit_size = format.planar_info.pixel_bits[pre_bits_index + j];
                    ar.push_back( get_data_masked<num_type_>(px_offset.px_addr, px_offset.bit_offset + px_channel_offset, pixel_bit_size ) );
                    px_channel_offset += pixel_bit_size;
                }
                
                pre_bits_index += format.planes[i].channels;
            }
            return ar;
        }

        template <typename num_type_>
        void Reader<num_type_>::increment_point() noexcept
        {
            auto [width, height] = _image.meta_data().size;
            _x += 1;
            if(_x >= width) {
                _x = 0;
                _y += 1;
            }
        }


        template <typename num_type_>
        Writer<num_type_>::Writer(ImageCpu  &image) noexcept
            :_image{image}
            ,_x{0}
            ,_y{0}
            ,_start_address{}
        {
            for(auto i = 0; i < image.meta_data().format.planes.size(); i++) {
                _start_address.push_back( plane_start_address(_image, i) );
            }
        }

        template <typename num_type_>
        void Writer<num_type_>::at(const PixelData<num_type_>  &px_data, const utils::vec2Ui  &pt) noexcept
        {
            auto meta = _image.meta_data();
            auto format = meta.format;

            //If Planar
            if(!format.planar_info.is_planar) {
                at_packed(px_data, pt);
            }
            else {
                at_planar(px_data, pt);
            }
        }

        template <typename num_type_>
        auto Writer<num_type_>::next(const PixelData<num_type_>  &px_data) noexcept -> bool
        {
            auto pt = utils::vec2Ui{_x, _y};
            auto img_size = _image.meta_data().size;
            
            if(!(pt[0] < img_size.width) || !(pt[1] < img_size.height)) {
                return false;
            }
            increment_point();
            at(px_data, pt);
            return true;
        }

        template <typename num_type_>
        void Writer<num_type_>::reset() noexcept
        {
            _x = 0;
            _y = 0;
        }

        template <typename num_type_>
        void Writer<num_type_>::at_packed(const PixelData<num_type_>  &px_data, const utils::vec2Ui  &pt) noexcept
        {
            auto& meta = _image.meta_data();
            auto& format = _image.meta_data().format;

            auto offset = std::size_t{};
            for(auto i = 0; i < format.planar_info.pixel_bits.size(); i++) 
            {
                auto px_offset = plane_pixel_address(pt, _start_address[0], _image.meta_data(), 0 );
                write_data_masked<num_type_>(const_cast<std::uint8_t*>(px_offset.px_addr), offset, format.planar_info.pixel_bits[i] , px_data[i] );
                offset += format.planar_info.pixel_bits[i];
            }
        }

        template <typename num_type_>
        void Writer<num_type_>::at_planar(const PixelData<num_type_>  &px_data, const utils::vec2Ui  &pt) noexcept
        {
            auto& meta = _image.meta_data();
            auto& format = _image.meta_data().format;

            auto pre_bits_index = 0u;
            for(auto i = 0; i < format.planes.size(); i++)
            {
                auto px_offset = plane_pixel_address(pt, _start_address[i], _image.meta_data(), i );

                for(auto j = 0, px_channel_offset = 0; j < format.planes[i].channels; j++)
                {
                    auto pixel_bit_size = format.planar_info.pixel_bits[pre_bits_index + j];
                    write_data_masked<num_type_>( const_cast<std::uint8_t*>(px_offset.px_addr), px_offset.bit_offset + px_channel_offset, pixel_bit_size , px_data[pre_bits_index + j ] );
                    px_channel_offset += pixel_bit_size;
                }

                pre_bits_index += format.planes[i].channels;
            }
        }

        template <typename num_type_>
        void Writer<num_type_>::increment_point() noexcept
        {
            auto [width, height] = _image.meta_data().size;
            _x += 1;
            if(_x >= width) {
                _x = 0;
                _y += 1;
            }
        }

        

        template <typename num_type_>
        RGBInterpreter<num_type_>::RGBInterpreter(const Format  &format_) noexcept
            :_format{format_}
        {}

        template <typename num_type_>
        auto RGBInterpreter<num_type_>::get_pixel(const PixelData<num_type_>  &ar) const noexcept -> Pixel_t
        {
            auto& format = _format;
            if(format.planar_info.pixel_bits.size() == 4) 
            {
                if(format.pixel_type == type::rgba) {
                    return Pixel_t{
                        .r = ar[0],
                        .g = ar[1],
                        .b = ar[2],
                        .a = ar[3]
                    };
                } 
                else {
                    return Pixel_t{
                        .r = ar[2],
                        .g = ar[1],
                        .b = ar[0],
                        .a = ar[3]
                    };
                }
            }
            else if(format.planar_info.pixel_bits.size() == 3) 
            {
                if(format.pixel_type == type::rgb) {
                    return Pixel_t{
                        .r = ar[0],
                        .g = ar[1],
                        .b = ar[2],
                        .a = std::numeric_limits<num_type_>::max() 
                    };
                } 
                else {
                    return Pixel_t{
                        .r = ar[2],
                        .g = ar[1],
                        .b = ar[0],
                        .a = std::numeric_limits<num_type_>::max() 
                    };
                }
            }
            else if(format.planar_info.pixel_bits.size() == 2) {
                return Pixel_t{
                        .r = ar[0],
                        .g = ar[1],
                        .b = 0,
                        .a = std::numeric_limits<num_type_>::max() 
                    };
            }
            //else if(format.planar_info.pixel_bits.size() == 1) 
            else {
                  return Pixel_t{
                        .r = ar[0],
                        .g = 0,
                        .b = 0,
                        .a = std::numeric_limits<num_type_>::max() 
                    };
            }
        }

        template <typename num_type_>
        auto RGBInterpreter<num_type_>::get_pixel(const Pixel_t  &data) const noexcept -> PixelData<num_type_>
        {
            auto& format = _format;
            if(format.planar_info.pixel_bits.size() == 4) 
            {
                if(format.pixel_type == type::rgba) {
                    return PixelData<num_type_>{
                        data.r,
                        data.g,
                        data.b,
                        data.a,
                    };
                } 
                else {
                    return PixelData<num_type_>{
                        data.b,
                        data.g,
                        data.r,
                        data.a,
                    };
                }
            }
            else if(format.planar_info.pixel_bits.size() == 3) 
            {
                if(format.pixel_type == type::rgb) {
                    return PixelData<num_type_>{
                        data.r,
                        data.g,
                        data.b,
                    };
                } 
                else {
                    return PixelData<num_type_>{
                        data.b,
                        data.g,
                        data.r,
                    };
                }
            }
            else if(format.planar_info.pixel_bits.size() == 2) {
                return PixelData<num_type_>{
                        data.r,
                        data.g,
                    };
            }
            //else if(format.planar_info.pixel_bits.size() == 1) 
            else {
                  return PixelData<num_type_>{
                        data.r
                    };
            }
        }
        
        template <typename num_type_>
        YUVInterpreter<num_type_>::YUVInterpreter(const Format  &format_) noexcept
            :_format{format_}
        {}

        template <typename num_type_>
        auto YUVInterpreter<num_type_>::get_pixel(const PixelData<num_type_>  &ar) const noexcept -> Pixel_t
        {
            auto& format = _format;
            if(format.pixel_type == type::yuv) {
                    return Pixel_t{
                    .y  = ar[0],
                    .cb = ar[1],
                    .cr = ar[2],
                    .a  = std::numeric_limits<num_type_>::max() 
                };
            }
            else {
                return Pixel_t{
                    .y  = ar[0],
                    .cb = ar[2],
                    .cr = ar[3],
                    .a  = ar[1]
                };
            }
        }

        template <typename num_type_>
        auto YUVInterpreter<num_type_>::get_pixel(const Pixel_t  &data) const noexcept -> PixelData<num_type_>
        {
            auto& format = _format;
            if(format.pixel_type == type::yuv) { 
                return PixelData<num_type_>{ data.y, data.cb, data.cr};
            }
            else {
                return PixelData<num_type_>{ data.y, data.a, data.cb, data.cr };
            }
        }

        template<template <typename> typename format_interpret_, typename num_type_ >
        ReaderWrap<format_interpret_, num_type_>::ReaderWrap(const ImageCpu  &image) noexcept
            :format_interpret_<num_type_>{image.meta_data().format}
            ,_reader{image}
        {}

        template<template <typename> typename format_interpret_, typename num_type_ >
        auto ReaderWrap<format_interpret_, num_type_>::at(const utils::vec2Ui  &pt) const noexcept -> Pixel_t
        {
            return format_interpret_<num_type_>::get_pixel( _reader.at(pt) );
        }

        template<template <typename> typename format_interpret_, typename num_type_ >
        auto ReaderWrap<format_interpret_, num_type_>::next() noexcept -> std::optional<Pixel_t>
        {
            if(auto px = _reader.next() ) {
                return format_interpret_<num_type_>::get_pixel(*px);
            }
            else {
                return std::nullopt;
            }
        }

        template<template <typename> typename format_interpret_, typename num_type_ >
        void ReaderWrap<format_interpret_, num_type_>::reset() noexcept
        {
            return _reader.reset();
        }

    //-----------------------------------------------------------------------------------------------

        template<template <typename> typename format_interpret_, typename num_type_ >
        WriterWrap<format_interpret_, num_type_>::WriterWrap(ImageCpu  &image) noexcept
            :format_interpret_<num_type_>{image.meta_data().format}
            ,_writer{image}
        {}

        template<template <typename> typename format_interpret_, typename num_type_ >
        void WriterWrap<format_interpret_, num_type_>::at(const Pixel_t  &px_data, const utils::vec2Ui  &pt) noexcept
        {
            _writer.at(format_interpret_<num_type_>::get_pixel(px_data) , pt);
        }

        template<template <typename> typename format_interpret_, typename num_type_ >
        auto WriterWrap<format_interpret_, num_type_>::next(const Pixel_t  &px_data) noexcept -> bool
        {
            return _writer.next( format_interpret_<num_type_>::get_pixel(px_data) ) ; 
        }

        template<template <typename> typename format_interpret_, typename num_type_ >
        void WriterWrap<format_interpret_, num_type_>::reset() noexcept
        {
            return _writer.reset() ; 
        }
    } // namespace pixel
}