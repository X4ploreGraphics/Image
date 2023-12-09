

#ifndef IMAGE_PIXEL_READER_WRITER_HPP
#define IMAGE_PIXEL_READER_WRITER_HPP
#include "image/image.hpp"
#include "utilities/data/vecs.hpp"
#include <optional>

namespace nitros::utils
{
    namespace pixel
    {
        template <typename NumType>
        struct PixelRGBA{
            NumType r;
            NumType g;
            NumType b;
            NumType a;
        };

        template <typename NumType>
        struct PixelYUV{
            NumType y;
            NumType cb;
            NumType cr;
            NumType a;
        };


        template <typename NumType>
        using PixelData = FixedSizeVec<NumType, 5>;

        template<typename num_type_>
        struct Reader
        {
            public:
            explicit Reader(const ImageCpu  &image) noexcept;

            [[nodiscard]] auto at(const utils::vec2Ui  &pt) const noexcept -> PixelData<num_type_>;
            [[nodiscard]] auto next() noexcept -> std::optional<PixelData<num_type_>>;
            void reset() noexcept;

            private:

            auto at_packed(const utils::vec2Ui  &pt) const noexcept -> PixelData<num_type_>;
            auto at_planar(const utils::vec2Ui  &pt) const noexcept -> PixelData<num_type_>;
            void increment_point() noexcept;

            const ImageCpu&  _image;
            std::uint32_t  _x, _y;
            FixedSizeVec<const std::uint8_t*, 5>  _start_address;
        };

        template<typename num_type_>
        struct Writer
        {
            public:
            explicit Writer(ImageCpu  &image) noexcept;

            void at(const PixelData<num_type_>  &px_data, const utils::vec2Ui  &pt) noexcept;
            [[nodiscard]] auto next(const PixelData<num_type_> &px_data) noexcept -> bool;
            void reset() noexcept;

            private:

            void at_packed(const PixelData<num_type_>  &px_data, const utils::vec2Ui  &pt) noexcept;
            void at_planar(const PixelData<num_type_>  &px_data, const utils::vec2Ui  &pt) noexcept;
            void increment_point() noexcept;

            ImageCpu&  _image;
            std::uint32_t  _x, _y;
            FixedSizeVec<const std::uint8_t*, 5>  _start_address;
        };

        template <typename num_type_>
        struct RGBInterpreter
        {
            public:
            using Pixel_t = PixelRGBA<num_type_>;
            explicit RGBInterpreter(const Format  &format_) noexcept;

            protected:
            auto get_pixel(const PixelData<num_type_>  &data) const noexcept -> Pixel_t;
            auto get_pixel(const Pixel_t  &data) const noexcept -> PixelData<num_type_>;

            private:
            Format  _format;
        };

        template <typename num_type_>
        struct YUVInterpreter
        {
            public:
            using Pixel_t = PixelYUV<num_type_>;
            explicit YUVInterpreter(const Format  &format_) noexcept;

            protected:
            auto get_pixel(const PixelData<num_type_>  &data) const noexcept -> Pixel_t;
            auto get_pixel(const Pixel_t  &data) const noexcept -> PixelData<num_type_>;

            private:
            Format  _format;
        };

        template<template <typename> typename format_interpret_, typename num_type_ >
        struct ReaderWrap : public format_interpret_<num_type_>
        {
            public:
            using Pixel_t = typename format_interpret_<num_type_>::Pixel_t;
            explicit ReaderWrap(const ImageCpu  &image) noexcept;

            [[nodiscard]] auto at(const utils::vec2Ui  &pt) const noexcept -> Pixel_t;
            [[nodiscard]] auto next() noexcept -> std::optional<Pixel_t>;
            void reset() noexcept;

            private:
            Reader<num_type_>   _reader;
        };

        template<template <typename> typename format_interpret_, typename num_type_ >
        struct WriterWrap : public format_interpret_<num_type_>
        {
            public:
            using Pixel_t = typename format_interpret_<num_type_>::Pixel_t;
            explicit WriterWrap(ImageCpu  &image) noexcept;

            void at(const Pixel_t  &px_data, const utils::vec2Ui  &pt) noexcept;
            [[nodiscard]] auto next(const Pixel_t  &px_data) noexcept -> bool;
            void reset() noexcept;

            private:
            Writer<num_type_>   _writer;
        };

        template <typename num_type_>
        using ReaderRGBA = ReaderWrap<RGBInterpreter, num_type_>;
        template <typename num_type_>
        using ReaderYUV = ReaderWrap<YUVInterpreter, num_type_>;

        template <typename num_type_>
        using WriterRGBA = WriterWrap<RGBInterpreter, num_type_>;
        template <typename num_type_>
        using WriterYUV = WriterWrap<YUVInterpreter, num_type_>;
    }
} // namespace nitros::utils

#include "read_write.inl"

#endif