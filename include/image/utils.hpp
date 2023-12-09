

#ifndef NITROS_IMAGE_UTILS_HPP
#define NITROS_IMAGE_UTILS_HPP

#include <cstdint>
#include <array>
#include <type_traits>
#include <algorithm>
#include "utilities/containers/fixed_vec.hpp"
#include "utilities/data/vecs.hpp"
#include "image/image.hpp"

namespace nitros::utils
{
    template <std::size_t N>
    inline constexpr auto create_bit_mask2(std::size_t offset, std::size_t bit_depth) -> std::array<std::uint8_t, N>
    {
        auto arr = std::array<std::uint8_t, N>{};
        for (auto i = 0; i < bit_depth; i++)
        {
            arr[(i + offset) / 8] |= 1 << (i + offset) % 8;
        }
        return arr;
    }

    template <typename NumType>
    inline constexpr auto create_bit_mask(std::size_t offset, std::size_t bit_depth) -> NumType
    {
        static_assert(std::is_integral_v<NumType> && std::is_unsigned_v<NumType>);
        bit_depth = std::clamp(bit_depth, std::size_t{0}, sizeof(NumType)*8 );

        return  static_cast<NumType>( (static_cast<NumType>(-(bit_depth != 0))
                & (static_cast<NumType>(-1) >> ((sizeof(NumType) * 8) - bit_depth)) ) << offset );
    }

    namespace internal
    {
        template <typename T>
        using FloatSizeT = std::conditional_t<std::is_floating_point_v<T>,
                                              std::conditional_t<std::is_same_v<T, float>, std::uint32_t, std::uint64_t>,
                                              std::void_t<T>>;

        template <typename T>
        using NumT = std::conditional_t<std::is_integral_v<T>, T, FloatSizeT<T>>;
    } // namespace internal

    template <typename NumType, typename = std::enable_if_t<std::is_floating_point_v<NumType> || std::is_integral_v<NumType>>>
    inline constexpr auto get_data_masked(const std::uint8_t *data, std::size_t offset, std::size_t bit_depth) -> NumType
    {
        using NumT_I = internal::NumT<NumType>;

        auto num = NumT_I{};
        auto num_t_bits = sizeof(NumT_I)*8;
            
        auto ptr = reinterpret_cast<const NumT_I*>(data) + (offset / num_t_bits);
        auto ptr_offset = offset % num_t_bits;
        num |= (create_bit_mask<NumT_I>( ptr_offset, bit_depth ) & *ptr) >> ptr_offset;

        if(ptr_offset + bit_depth > num_t_bits) {
            num |= (create_bit_mask<NumT_I>( 0, bit_depth - (num_t_bits - ptr_offset) ) & *(ptr + 1) ) << (num_t_bits - ptr_offset);
        }
        return *reinterpret_cast<NumType *>(&num);
    }

    template <typename NumType, typename = std::enable_if_t<std::is_floating_point_v<NumType> || std::is_integral_v<NumType>>>
    inline constexpr void write_data_masked(std::uint8_t *data, std::size_t offset, std::size_t bit_depth, const NumType &num_)
    {
        using NumT_I = internal::NumT<NumType>;
        auto masked_num = *reinterpret_cast<const NumT_I*>(&num_) & create_bit_mask<NumT_I>(0, bit_depth) ;

        auto num_t_bits = sizeof(NumT_I)*8;
            
        auto ptr = reinterpret_cast<NumT_I*>(data) + (offset / num_t_bits);
        auto ptr_offset = offset % num_t_bits;
        auto bit_mask = create_bit_mask<NumT_I>( ptr_offset, bit_depth );
        *ptr = *ptr & ~(bit_mask) | (bit_mask & (masked_num << ptr_offset) );

        if(ptr_offset + bit_depth > num_t_bits) {
            auto bit_mask2 = create_bit_mask<NumT_I>( 0, bit_depth - (num_t_bits - ptr_offset) );
            *(ptr + 1) = *(ptr + 1) & ~(bit_mask2) | (bit_mask2 & ( masked_num >> (num_t_bits - ptr_offset) ) );
        }
    }

    /**
     * If format is packed, always returns the Plane 0 interpretation
     * 
     * @returns interpreted point for the Plane
     * */
    inline auto interpreted_point(const vec2Ui  &pt, const ImageMetaData  &meta_data, std::size_t  index) noexcept -> vec2Ui 
    {
        if(!meta_data.format.planar_info.is_planar) {
            return pt;
        }
        
        auto step  = meta_data.steps.at(index);
        auto plane = meta_data.format.planes.at(index);

        auto x = pt[0] * plane.width_factor.num/plane.width_factor.den;
        auto y = pt[1] * plane.height_factor.num/plane.height_factor.den;
        return {x, y};
    }

    inline auto interpreted_plane_img_size(const ImageMetaData  &meta_data, std::size_t index) -> ImgSize
    {
        if(!meta_data.format.planar_info.is_planar) {
            return meta_data.size;
        }

        auto step  = meta_data.steps[index];
        auto plane = meta_data.format.planes[index];

        auto x = meta_data.size.width * plane.width_factor.num/plane.width_factor.den;
        auto y = meta_data.size.height * plane.height_factor.num/plane.height_factor.den;
        return {x, y};
    }

    inline auto plane_start_address(const ImageCpu  &image_, std::size_t  index) -> const std::uint8_t*
    {
        auto&  meta_data = image_.meta_data();
        if(!meta_data.format.planar_info.is_planar) {
            return image_.buffer().data();
        }

        if(index > 0) {
            auto [w, h] = interpreted_plane_img_size(meta_data, index - 1);
            return plane_start_address(image_, index -1) + h * meta_data.steps.at(index-1);
        }
        else {
            return image_.buffer().data();
        }
    }

    struct PixelOffset
    {
        const std::uint8_t*   px_addr;
        std::uint32_t   bit_offset;
    };

    /**
     * @param pt pixel point (Non interpreted)
     * @param plane_start starting address of Plane 
     * @param meta_data Image Meta Data
     * @param index Index of the Plane
     * @returns pixel address of the point
     * */
    inline auto plane_pixel_address(const utils::vec2Ui  &pt, const std::uint8_t*  plane_start, const ImageMetaData  &meta_data, std::size_t  index) -> PixelOffset
    {
        if(!meta_data.format.planar_info.is_planar) {
            return { .px_addr = plane_start + meta_data.steps[0]*pt[1] + (pt[0] * meta_data.format.pixel_layout.bytes / meta_data.format.pixel_layout.group_pixels),
                    .bit_offset = 0 };
        }
        else
        {
            auto st = plane_start;
            auto [x, y] = interpreted_point(pt, meta_data, index);
            auto& format = meta_data.format;

            //Increment pixel Bits upto plane index
            auto pre_bits_index = 0u;;
            for(auto i = 0; i < index; i++) {
                pre_bits_index += format.planes[i].channels;
            }

            auto plane_pixel_bit_size = std::uint8_t{0};
            for(auto i = 0; i < format.planes[index].channels; i++) {
                plane_pixel_bit_size += format.planar_info.pixel_bits[ pre_bits_index + i ];
            }
            
            return {.px_addr = st + meta_data.steps[index] * y + (x * plane_pixel_bit_size)/8,
                    .bit_offset = (x * plane_pixel_bit_size) % 8 };
        }
    }

    /**
     * @param pt pixel point (Non interpreted)
     * @param image 
     * @param index Index of the Plane
     * @returns pixel address of the point
     * */
    inline auto plane_pixel_address(const utils::vec2Ui  &pt, const ImageCpu  &image_, std::size_t  index) -> PixelOffset
    {
        auto&   meta_data =image_.meta_data();
        if(!meta_data.format.planar_info.is_planar) {
            return plane_pixel_address(pt, image_.buffer().data(), meta_data, index );
        }
        else
        {
            auto st = plane_start_address(image_, index);
            return plane_pixel_address(pt, st, meta_data, index );
        }
    }
} // namespace nitros::utils

#endif
