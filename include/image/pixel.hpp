

#ifndef IMAGE_PIXEL_HPP
#define IMAGE_PIXEL_HPP

#include "utilities/algorithm.hpp"
#include "utilities/containers.hpp"
#include <cstdint>
#include <ratio>
#include <initializer_list>
#include <vector>
#include <array>

namespace nitros::utils
{
   namespace pixel
   {
        template<typename T>
        using Array = FixedSizeVec<T, 5>;
       
        enum class type {
            r, rg, rgb, rgba, bgr, bgra, grey, stencil, grey_stencil, yuv, yuva
        };
    
        struct Layout 
        {
            constexpr Layout(std::uint32_t  bytes_, std::uint32_t channels_, std::uint32_t group_pixels_, bool normalized_) noexcept
                :bytes{bytes_}
                ,channels{channels_}
                ,group_pixels{group_pixels_}
                ,normalized{normalized_}
                {}

            std::uint32_t bytes, channels, group_pixels;
            bool normalized;
        };

        template <std::uint32_t bytes_, std::uint32_t channels_, std::uint32_t grp_pixels_, bool normalized_>
        struct layout_factory
        {
            static constexpr auto bytes = bytes_;
            static constexpr auto channels = channels_;
            static constexpr auto grp_pixels = grp_pixels_;
            static constexpr auto normalized = normalized_;

            static constexpr auto value = Layout{bytes_, channels_, grp_pixels_, normalized_};
        };

        struct Planar
        {
            constexpr Planar(const Array<std::uint8_t>  &bits_, bool is_planar_) noexcept
                :pixel_bits{bits_}
                ,is_planar{is_planar_}
            {}

            Array<std::uint8_t>  pixel_bits;
            bool    is_planar;
        };

        template <bool is_planar_, std::uint8_t  ... Bits_>
        struct planar_factory
        {
            static constexpr auto is_planar = is_planar_;
            using Bits = std::integer_sequence<std::uint8_t, Bits_... >;
            static constexpr auto value = Planar{Array<std::uint8_t>{Bits_...}, is_planar_};
        };

        struct Plane
        {
            //Facor By which Plane is 
            struct factor {
                std::uint32_t    num;
                std::uint32_t    den;
            };

            factor  width_factor;
            factor  height_factor;
            std::uint32_t    row_alignment;
            std::uint32_t    channels;
        };

        inline auto operator<(const Plane &lhs, const Plane &rhs) noexcept -> bool;
        inline auto operator>(const Plane &lhs, const Plane &rhs) noexcept -> bool;
        inline auto operator==(const Plane  &lhs, const Plane  &rhs) noexcept -> bool;

        template <typename width_ratio_, typename height_ratio_, std::uint32_t row_alignment_, std::uint32_t channels_>
        struct plane_factory
        {
            using width_ratio  = width_ratio_;
            using height_ratio = height_ratio_;
            static constexpr auto row_alignment = row_alignment_;
            static constexpr auto value = Plane{ .width_factor = { width_ratio_::num, width_ratio_::den }, .height_factor = { height_ratio_::num, height_ratio_::den }, .row_alignment = row_alignment_, .channels = channels_ };
        };
    
        struct Format
        {   
            constexpr Format(type pixel_type_, const Layout &pixel_layout_, const Planar &planar_, Array<Plane>  planes_) noexcept
                :pixel_type{pixel_type_}
                ,pixel_layout{pixel_layout_}
                ,planar_info{planar_}
                ,planes{planes_}
            {}

            [[deprecated]] constexpr Format(type pixel_type_, const Layout &pixel_layout_) noexcept
                :pixel_type{pixel_type_}
                ,pixel_layout{pixel_layout_}
                ,planar_info{{8}, false}
                ,planes{}
            {}

            type    pixel_type;
            Layout  pixel_layout;
            Planar  planar_info;
            Array<Plane>  planes;
        };

        template <type pix_type_, typename Layout_, typename Planar_, typename ... Plane_>
        struct format_factory {
            static constexpr auto pix_type = pix_type_;
            using Layout = Layout_;
            using Planar = Planar_;
            using Planes = std::tuple<Plane_...>;
            
            static constexpr Format value{pix_type_, Layout_::value, Planar_::value, Array<Plane>{Plane_::value ...}};
        };

        template <std::uint8_t ... Bits>
        using interleaved_planar_t = planar_factory<false, Bits ... >;
        template <std::uint32_t  channels_>
        using interleaved_plane_t = plane_factory<std::ratio<1, 1>, std::ratio<1, 1>, 8, channels_>;
    
        using R8    = format_factory<type::r , layout_factory<1*1, 1, 1, false>, interleaved_planar_t<8 >, interleaved_plane_t<1> >;
        using R16   = format_factory<type::r , layout_factory<1*2, 1, 1, false>, interleaved_planar_t<16>, interleaved_plane_t<1> >;
        using R32f  = format_factory<type::r , layout_factory<1*4, 1, 1, true> , interleaved_planar_t<32>, interleaved_plane_t<1> >;
        using R64f  = format_factory<type::r , layout_factory<1*8, 1, 1, true> , interleaved_planar_t<64>, interleaved_plane_t<1> >;

        using RG8    = format_factory<type::rg , layout_factory<2*1, 2, 1, false>,interleaved_planar_t<8, 8 >, interleaved_plane_t<2> >;
        using RG16   = format_factory<type::rg , layout_factory<2*2, 2, 1, false>,interleaved_planar_t<16,16>, interleaved_plane_t<2> >;
        using RG32f  = format_factory<type::rg , layout_factory<2*4, 2, 1, true> ,interleaved_planar_t<32,32>, interleaved_plane_t<2> >;
        using RG64f  = format_factory<type::rg , layout_factory<2*8, 2, 1, true> ,interleaved_planar_t<64,64>, interleaved_plane_t<2> >;

        using RGB8  = format_factory<type::rgb , layout_factory<3, 3, 1, false>, interleaved_planar_t<8,8,8>, interleaved_plane_t<3> >;
        using RGBA8 = format_factory<type::rgba, layout_factory<4, 4, 1, false>, interleaved_planar_t<8,8,8,8>, interleaved_plane_t<4> >;
        using BGR8  = format_factory<type::bgr , layout_factory<3, 3, 1, false>, interleaved_planar_t<8,8,8>, interleaved_plane_t<3> >;
        using BGRA8 = format_factory<type::bgra, layout_factory<4, 4, 1, false>, interleaved_planar_t<8,8,8,8>, interleaved_plane_t<4> >;

        using RGB16  = format_factory<type::rgb , layout_factory<3*2, 3, 1, false>, interleaved_planar_t<16,16,16>, interleaved_plane_t<3> >;
        using RGBA16 = format_factory<type::rgba, layout_factory<4*2, 4, 1, false>, interleaved_planar_t<16,16,16,16>, interleaved_plane_t<4> >;
        using BGR16  = format_factory<type::bgr , layout_factory<3*2, 3, 1, false>, interleaved_planar_t<16,16,16>, interleaved_plane_t<3> >;
        using BGRA16 = format_factory<type::bgra, layout_factory<4*2, 4, 1, false>, interleaved_planar_t<16,16,16,16>, interleaved_plane_t<4> >;

        using RGB32f  = format_factory<type::rgb , layout_factory<3*4, 3, 1, true>, interleaved_planar_t<32,32,32>, interleaved_plane_t<3> >;
        using RGBA32f = format_factory<type::rgba, layout_factory<4*4, 4, 1, true>, interleaved_planar_t<32,32,32,32>, interleaved_plane_t<4> >;
        using BGR32f  = format_factory<type::bgr , layout_factory<3*4, 3, 1, true>, interleaved_planar_t<32,32,32>, interleaved_plane_t<3> >;
        using BGRA32f = format_factory<type::bgra, layout_factory<4*4, 4, 1, true>, interleaved_planar_t<32,32,32,32>, interleaved_plane_t<4> >;

        using RGB64f  = format_factory<type::rgb , layout_factory<3*8, 3, 1, true>, interleaved_planar_t<64,64,64>, interleaved_plane_t<3> >;
        using RGBA64f = format_factory<type::rgba, layout_factory<4*8, 4, 1, true>, interleaved_planar_t<64,64,64,64>, interleaved_plane_t<4> >;
        using BGR64f  = format_factory<type::bgr , layout_factory<3*8, 3, 1, true>, interleaved_planar_t<64,64,64>, interleaved_plane_t<3> >;
        using BGRA64f = format_factory<type::bgra, layout_factory<4*8, 4, 1, true>, interleaved_planar_t<64,64,64,64>, interleaved_plane_t<4> >;

        using GREY8   = format_factory<type::grey, layout_factory<1, 1, 1, false>, interleaved_planar_t<8 >, interleaved_plane_t<1> >;
        using GREY16  = format_factory<type::grey, layout_factory<2, 1, 1, false>, interleaved_planar_t<16>, interleaved_plane_t<1> >;
        using GREY32f = format_factory<type::grey, layout_factory<4, 1, 1, true >, interleaved_planar_t<32>, interleaved_plane_t<1> >;
        using GREY64f = format_factory<type::grey, layout_factory<8, 1, 1, true >, interleaved_planar_t<64>, interleaved_plane_t<1> >;

        using STENCIL8   = format_factory<type::stencil, layout_factory<1, 1, 1, false>, interleaved_planar_t<8 >, interleaved_plane_t<1> >;
    
        using GREY_STENCIL_16_8  = format_factory<type::grey_stencil, layout_factory<3, 2, 1, false>, interleaved_planar_t<16,8 >, interleaved_plane_t<2> >;
        using GREY_STENCIL_24_8  = format_factory<type::grey_stencil, layout_factory<4, 2, 1, false>, interleaved_planar_t<24,8 >, interleaved_plane_t<2> >;
        using GREY_STENCIL_32f_8 = format_factory<type::grey_stencil, layout_factory<8, 2, 1, true> , interleaved_planar_t<32,8 >, interleaved_plane_t<2> >;

        using YUV420p = format_factory<type::yuv, layout_factory<6, 3, 4, false>, planar_factory<true, 8, 8, 8> , plane_factory< std::ratio<1, 1>, std::ratio<1, 1>, 8, 1>
                                                                                                                , plane_factory< std::ratio<1, 2>, std::ratio<1, 2>, 8, 1>
                                                                                                                , plane_factory< std::ratio<1, 2>, std::ratio<1, 2>, 8, 1> >;

        using YUVA420p = format_factory<type::yuva, layout_factory<10, 4, 4, false>, planar_factory<true, 8, 8, 8, 8> , plane_factory< std::ratio<1, 1>, std::ratio<1, 1>, 8, 2>
                                                                                                                    , plane_factory< std::ratio<1, 2>, std::ratio<1, 2>, 8, 1>
                                                                                                                    , plane_factory< std::ratio<1, 2>, std::ratio<1, 2>, 8, 1> >;

        using YUV422p = format_factory<type::yuv, layout_factory<4, 3, 2, false>, planar_factory<true, 8, 8, 8> , plane_factory< std::ratio<1, 1>, std::ratio<1, 1>, 8, 1>
                                                                                                                , plane_factory< std::ratio<1, 2>, std::ratio<1, 1>, 8, 1>
                                                                                                                , plane_factory< std::ratio<1, 2>, std::ratio<1, 1>, 8, 1> >;

        using YUVA422p = format_factory<type::yuva, layout_factory<6, 3, 2, false>, planar_factory<true, 8, 8, 8, 8>  , plane_factory< std::ratio<1, 1>, std::ratio<1, 1>, 8, 2>
                                                                                                                    , plane_factory< std::ratio<1, 2>, std::ratio<1, 1>, 8, 1>
                                                                                                                    , plane_factory< std::ratio<1, 2>, std::ratio<1, 1>, 8, 1> >;

        using YUV444p = format_factory<type::yuv, layout_factory<3, 3, 1, false>, planar_factory<true, 8, 8, 8> , plane_factory< std::ratio<1, 1>, std::ratio<1, 1>, 8, 1>
                                                                                                                , plane_factory< std::ratio<1, 1>, std::ratio<1, 1>, 8, 1>
                                                                                                                , plane_factory< std::ratio<1, 1>, std::ratio<1, 1>, 8, 1> >;

        using YUVA444p = format_factory<type::yuva, layout_factory<4, 3, 1, false>, planar_factory<true, 8, 8, 8, 8>  , plane_factory< std::ratio<1, 1>, std::ratio<1, 1>, 8, 2>
                                                                                                                    , plane_factory< std::ratio<1, 1>, std::ratio<1, 1>, 8, 1>
                                                                                                                    , plane_factory< std::ratio<1, 1>, std::ratio<1, 1>, 8, 1> >;
        }
}

#include "pixel.inl"

#endif