
#ifndef CONVERSION_INTERNAL_PIXEL_HPP
#define CONVERSION_INTERNAL_PIXEL_HPP

#include <type_traits>
#include <cstdint>

namespace nitros::image
{
    template <typename Num_type, typename = std::enable_if_t<std::is_integral_v<Num_type> && std::is_unsigned_v<Num_type> >>
    void rbg_yuv(Num_type r_, Num_type g_, Num_type b_, std::uint8_t *y, std::uint8_t *cb, std::uint8_t *cr)
    {
        auto r = r_;
        auto g = g_;
        auto b = b_;

        if constexpr ( !std::is_same_v<std::uint8_t, Num_type> ) {
            r = r_ >> (sizeof(Num_type)*8 - 8) ;
            g = g_ >> (sizeof(Num_type)*8 - 8) ;
            b = b_ >> (sizeof(Num_type)*8 - 8) ;
        }
    
        auto Y = (77 * r + 150 * g + 29 * b);
        auto U = -43 * r + (-84) * g + 127 * b;
        auto V = 127 * r + (-106) * g + (-21) * b;

        auto Yt = (Y + 128) >> 8;
        auto Ut = (U + 128) >> 8;
        auto Vt = (V + 128) >> 8;

        *y = Yt;
        *cb = Ut + 128;
        *cr = Vt + 128;
    }

    template <typename Num_Type, typename = std::enable_if_t<std::is_integral_v<Num_Type>>>
    void yuv_to_rgb(uint8_t yValue, uint8_t uValue, uint8_t vValue, Num_Type *r, Num_Type *g, Num_Type *b)
    {
        auto r_ = yValue + (1.370705 * (vValue - 128));
        auto g_ = yValue - (0.698001 * (vValue - 128)) - (0.337633 * (uValue - 128));
        auto b_ = yValue + (1.732446 * (uValue - 128));
        *r = std::clamp<Num_Type>(r_, 0, std::numeric_limits<Num_Type>::max());
        *g = std::clamp<Num_Type>(g_, 0, std::numeric_limits<Num_Type>::max());
        *b = std::clamp<Num_Type>(b_, 0, std::numeric_limits<Num_Type>::max());

        if constexpr(!std::is_same_v<Num_Type, std::uint8_t>) {
            *r = *r << sizeof(Num_Type)*8 - 8;
            *g = *g << sizeof(Num_Type)*8 - 8;
            *b = *b << sizeof(Num_Type)*8 - 8;
        }
    }


    //Assertion should be ensured that src_meta size and dst_meta size are equal
    template<typename NumType, std::size_t  N>
    inline void rgb_to_yuv444(const utils::ImageMetaData  &src_meta, const utils::ImageMetaData  &dst_meta, const std::uint8_t  *src_st, std::uint8_t  *dst_st)
    {
        static_assert( std::is_unsigned_v<NumType> );
        auto dst_plane_0 = dst_st;
        auto dst_plane_1 = dst_plane_0 + dst_meta.steps[0] * dst_meta.size.height ;
        auto dst_plane_2 = dst_plane_1 + dst_meta.steps[1] * dst_meta.size.height ;

        for(auto y = 0; y < src_meta.size.height  ; y++)
        {
            for(auto x = 0; x < src_meta.size.width  ; x++)
            {
                auto px = src_st +  y*src_meta.steps[0] + x*N ;
                auto px_y  = dst_plane_0 + y*dst_meta.steps[0] + x;
                auto px_cb = dst_plane_1 + y*dst_meta.steps[1] + x;
                auto px_cr = dst_plane_2 + y*dst_meta.steps[2] + x;
                rbg_yuv<NumType>(px[0], px[1], px[2], px_y, px_cb, px_cr);
            }
        }
    }

    template<typename NumType, std::size_t  N>
    inline void yuv444_to_rgb(const utils::ImageMetaData  &src_meta, const utils::ImageMetaData  &dst_meta, const std::uint8_t  *src_st, std::uint8_t  *dst_st)
    {
        static_assert( std::is_unsigned_v<NumType> );
        auto st_plane_0 = src_st;
        auto st_plane_1 = st_plane_0 + src_meta.steps[0] * src_meta.size.height ;
        auto st_plane_2 = st_plane_1 + src_meta.steps[1] * src_meta.size.height ;

        for(auto y = 0; y < src_meta.size.height  ; y++)
        {
            for(auto x = 0; x < src_meta.size.width  ; x++)
            {
                auto px = dst_st +  y*dst_meta.steps[0] + x*N ;
                auto px_y  = st_plane_0 + y * src_meta.steps[0] + x;
                auto px_cb = st_plane_1 + y * src_meta.steps[1] + x;
                auto px_cr = st_plane_2 + y * src_meta.steps[2] + x;
                yuv_to_rgb<NumType>( *px_y, *px_cb, *px_cr, px, px + 1, px + 2 );
            }
        }
    }
}

#endif