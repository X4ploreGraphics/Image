

#ifndef NITROS_UITLS_IMAGE_HPP
#define NITROS_UITLS_IMAGE_HPP

#include "pixel.hpp"
#include "image/image_export.h"
#include <gsl/gsl>
#include <vector>
#include <cstdint>

namespace nitros::utils
{
    struct ImgSize 
    {
        std::size_t  width, height;   
    };

    inline constexpr bool operator==(const ImgSize  &lhs, const ImgSize  &rhs) { return (lhs.height == rhs.height) && (lhs.width == rhs.width);  }
    inline constexpr bool operator!=(const ImgSize  &lhs, const ImgSize  &rhs) {  return !(lhs == rhs);  }

    class NIMAGE_EXPORT ImageMetaData final
    {
        public:
        ImageMetaData(const ImgSize   &size_, const pixel::Format &format_, const FixedSizeVec<std::size_t, 5> &step_ = FixedSizeVec<std::size_t, 5>{}); 
        ImageMetaData(const ImageMetaData   &meta_data) noexcept;
        ImageMetaData(ImageMetaData &&meta_data) noexcept;
        ~ImageMetaData() = default;

        auto operator=(const ImageMetaData  &val) noexcept -> ImageMetaData& = default;
        auto operator=(ImageMetaData  &&val) noexcept -> ImageMetaData& = default;
        
        ImgSize          size;
        pixel::Format    format;
        FixedSizeVec<std::size_t, 5>   steps;
        std::size_t       step;
    };

    inline constexpr auto operator==(const ImageMetaData  &lhs, const ImageMetaData  &rhs) -> bool { return lhs.size == rhs.size && lhs.format == rhs.format && lhs.steps == rhs.steps; }
    inline constexpr auto operator!=(const ImageMetaData  &lhs, const ImageMetaData  &rhs) -> bool { return !(lhs == rhs);  }
    
    template<typename buffer_type_>
    class Image
    {
        public:
        using buffer_type = buffer_type_;

        explicit Image() noexcept = default;
        Image(ImageMetaData  &&metaData, buffer_type  &&buffer);
        Image(const ImageMetaData &metaData, const buffer_type &buffer);
        Image(const Image&) = default;
        Image(Image&&) noexcept = default;
        ~Image() = default;

        Image&  operator=(const Image&) = default;
        Image&  operator=(Image&&) noexcept = default;

        buffer_type     &buffer() noexcept;
        const buffer_type     &buffer() const noexcept;

        ImageMetaData   &meta_data() noexcept;
        const ImageMetaData   &meta_data() const noexcept;

        private:
        buffer_type         _data;
        ImageMetaData       _metaData;
    };

    using ImgBufferCpu = std::vector<std::uint8_t>;
    using ImageCpu = Image<ImgBufferCpu>;

    namespace   image
    {
        /**
         * @returns step size in bytes
         * */
        NIMAGE_EXPORT auto memory_aligned_step(const std::size_t  &line_step_bits, std::uint32_t  row_alignment_bits) -> std::size_t;

        template<typename Image>
        Image  create(ImgSize size, pixel::Format  format);
        NIMAGE_EXPORT ImageCpu create_cpu(ImgSize size, pixel::Format format);

        NIMAGE_EXPORT void assign(ImgSize img_size, size_t src_step, const gsl::span<uint8_t> src_ptr, size_t dest_step, gsl::span<uint8_t>  dest_ptr);
    }
}

#include "image.inl"

#endif