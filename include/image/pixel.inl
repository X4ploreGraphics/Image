

#include "pixel.hpp"

namespace nitros::utils
{
    namespace pixel
    {
        inline constexpr auto operator==(const Layout &lhs, const Layout &rhs) -> bool { return (lhs.bytes == rhs.bytes) && (lhs.channels == rhs.channels) && (lhs.group_pixels == rhs.group_pixels) && (lhs.normalized == rhs.normalized); }
        inline constexpr auto operator!=(const Layout &lhs, const Layout &rhs) -> bool { return !(lhs == rhs); }

        inline constexpr auto operator==(const Planar &lhs, const Planar &rhs) noexcept -> bool { return lhs.pixel_bits == rhs.pixel_bits && lhs.is_planar == rhs.is_planar; }
        inline constexpr auto operator!=(const Planar  &lhs, const Planar  &rhs) noexcept -> bool { return !(lhs == rhs); }

        inline constexpr auto operator==(const Format &lhs, const Format &rhs) -> bool { return (lhs.pixel_layout == rhs.pixel_layout) && (lhs.pixel_type == rhs.pixel_type) && (lhs.planar_info == rhs.planar_info) && (lhs.planes == rhs.planes); }
        inline constexpr auto operator!=(const Format &lhs, const Format &rhs) -> bool { return !(lhs == rhs); }

        inline constexpr auto operator<(const Plane::factor &lhs, const Plane::factor &rhs) noexcept -> bool {
            return lhs.num * rhs.den < rhs.num * lhs.den;
        }
        inline constexpr auto operator>(const Plane::factor &lhs, const Plane::factor &rhs) noexcept -> bool {
            return rhs < lhs;
        }

        inline auto operator<(const Plane &lhs, const Plane &rhs) noexcept -> bool {
            auto ar1 = std::array<std::uint32_t, 5>{lhs.width_factor.num, lhs.width_factor.den, lhs.height_factor.num, lhs.height_factor.den, lhs.row_alignment };
            auto ar2 = std::array<std::uint32_t, 5>{rhs.width_factor.num, rhs.width_factor.den, rhs.height_factor.num, rhs.height_factor.den, rhs.row_alignment };
            return  ::nitros::utils::lexicographical_compare(ar1.begin(), ar1.end(), ar2.begin(), ar2.end());
        }

        inline auto operator>(const Plane &lhs, const Plane &rhs) noexcept -> bool {
            return  rhs < lhs;
        }

        inline auto operator==(const Plane  &lhs, const Plane  &rhs) noexcept -> bool {
            return !(lhs < rhs) && !(rhs < lhs);
        }
    } // namespace pixel
} // namespace nitros::utils
