

//#include "image.hpp"

namespace nitros::utils
{
    template <typename buffer_type>
    Image<buffer_type>::Image(ImageMetaData  &&metaData, buffer_type &&buffer)
        :_data{std::move(buffer)}
        ,_metaData{std::move(metaData)}
    {}

    template <typename buffer_type>
    Image<buffer_type>::Image(const ImageMetaData  &metaData, const buffer_type &buffer)
        :_data{buffer}
        ,_metaData{metaData}
    {}

    template <typename buffer_type>
    buffer_type     &Image<buffer_type>::buffer() noexcept
    {
        return _data;
    }
    
    template <typename buffer_type>
    const buffer_type     &Image<buffer_type>::buffer() const noexcept
    {
        return _data;
    }

    template <typename buffer_type>
    ImageMetaData   &Image<buffer_type>::meta_data() noexcept
    {
        return _metaData;
    }

    template <typename buffer_type>
    const ImageMetaData   &Image<buffer_type>::meta_data() const noexcept
    {
        return _metaData;
    }
    
    namespace image
    {
        template<typename Image>
        Image  create(ImgSize size, pixel::Format  format)
        {
            ImageMetaData   metaData{size, format};
            auto s_it = metaData.steps.begin();
            auto plane_it = metaData.format.planes.begin();

            auto buf_size = std::size_t{};
            for( ; s_it != metaData.steps.end() ; s_it++, plane_it++) 
            {
                buf_size += (*s_it) * (size.height * plane_it->height_factor.num / plane_it->height_factor.den);
            }

            typename Image::buffer_type   buffer( buf_size );
            return Image{std::move(metaData), std::move(buffer)};
        }

        inline ImageCpu create_cpu(ImgSize size, pixel::Format format) 
        {
            return create<ImageCpu>(size, format);
        }
    }
    
}