

#include "image/image.hpp"
#include <stdexcept>
#include <assert.h>

namespace nitros::utils
{
    namespace 
    {
        auto find_row_step(const FixedSizeVec<std::size_t, 5>  &step_, const pixel::Format  &format_, const ImgSize  &size_) -> FixedSizeVec<std::size_t, 5>
        {
            if ( step_.size() == 0 ) 
            {    
                if(format_.planar_info.is_planar)
                {
                    auto r_steps = FixedSizeVec<std::size_t, 5>{};        
                    auto plane_it = format_.planes.begin();
                    auto pix_it = format_.planar_info.pixel_bits.begin();
                    for( ; plane_it != format_.planes.end() ; plane_it++)
                    {
                        //Find Plane Pixel Bit Size
                        auto px_bits_size = std::uint8_t{0};
                        for(auto i = 0; i < plane_it->channels; i++) {
                            px_bits_size += (*pix_it);
                            pix_it++;
                        }

                        auto r_bits = plane_it->width_factor.num * size_.width * px_bits_size / plane_it->width_factor.den;
                        r_steps.push_back( image::memory_aligned_step( r_bits, plane_it->row_alignment ) );
                    }
                    return r_steps;
                }
                else
                {
                    auto r_step = image::memory_aligned_step( (format_.pixel_layout.bytes * size_.width * 8)/ format_.pixel_layout.group_pixels , format_.planes.at(0).row_alignment );
                    return {r_step};
                }
            }
            else 
            {
                if(step_.size() != format_.planes.size())
                    throw std::invalid_argument("Step Array and No of Planes Doesn't match");
                if(format_.planar_info.is_planar)
                {
                    auto plane_it = format_.planes.begin();
                    auto pix_it = format_.planar_info.pixel_bits.begin();
                    auto step_it = step_.begin();
                    for( ; plane_it != format_.planes.end() ; plane_it++, pix_it++, step_it++)
                    {
                        //Find Plane Pixel Bit Size
                        auto px_bits_size = std::uint8_t{0};
                        for(auto i = 0; i < plane_it->channels; i++) {
                            px_bits_size += (*pix_it);
                            pix_it++;
                        }

                        auto r_bits = plane_it->width_factor.num * size_.width * px_bits_size / plane_it->width_factor.den;
                        auto r_step = image::memory_aligned_step( r_bits, plane_it->row_alignment );
                        if( *step_it < r_step)
                            throw std::invalid_argument("Image Width Greater than Row Step");
                    }
                }   
                else
                {
                    auto r_step = image::memory_aligned_step( (format_.pixel_layout.bytes * size_.width * 8)/ format_.pixel_layout.group_pixels, format_.planes.at(0).row_alignment );
                    if( step_.at(0) < r_step)
                            throw std::invalid_argument("Image Width Greater than Row Step");
                }
                return step_;
            }
        }

        auto find_row_step(const std::size_t &step_, const pixel::Format  &format_, const ImgSize  &size_) -> FixedSizeVec<std::size_t, 5>
        {
            if(format_.planar_info.is_planar)
                throw std::invalid_argument("Call other Constructor ImageMetaData for Planar Image Data");

            auto r_step = image::memory_aligned_step( (format_.pixel_layout.bytes * size_.width * 8)/ format_.pixel_layout.group_pixels, format_.planes.at(0).row_alignment );
            if( step_ < r_step )
                throw std::invalid_argument("Image Width Greater than Row Step");

            return {step_};
        }
    } // namespace 
    

    ImageMetaData::ImageMetaData(const ImgSize &size_, const pixel::Format &format_, const FixedSizeVec<std::size_t, 5> &step_)
        :size{size_}
        ,format{format_}
        ,steps{ find_row_step( step_, format_, size_ ) }
        ,step{steps.at(0)}
    {}

    ImageMetaData::ImageMetaData(const ImageMetaData   &meta_data) noexcept
        :size{meta_data.size}
        ,format{meta_data.format}
        ,steps{meta_data.steps}
        ,step{meta_data.step}
    {}

    ImageMetaData::ImageMetaData(ImageMetaData &&meta_data) noexcept
        :size{std::move(meta_data.size)}
        ,format{std::move(meta_data.format)}
        ,steps{std::move(meta_data.steps)}
        ,step{std::move(meta_data.step)}
    {}

    namespace image 
    {
        auto memory_aligned_step(const std::size_t  &line_step_bits, std::uint32_t  row_alignment_bits) -> std::size_t
        {
            auto row_rem = (line_step_bits) % row_alignment_bits;
            return row_rem ? ( line_step_bits + (row_alignment_bits - row_rem) )/8 : line_step_bits / 8;
        }

        void assign(ImgSize img_size, size_t src_step, const gsl::span<uint8_t> src_ptr, size_t dest_step, gsl::span<uint8_t>  dest_ptr)
        {
            if(dest_ptr.size() < (img_size.height * dest_step) ) {
                throw  std::invalid_argument("Dest Image buffer Not Sufficient");
            }

            if(src_step == dest_step) {
                std::memcpy(dest_ptr.data(), src_ptr.data(), src_step * img_size.height);
            }else {
                auto step = std::min(src_step, dest_step);
                for(size_t i = 0; i < img_size.height; i++) {
                    std::memcpy(dest_ptr.data() + (i*dest_step), src_ptr.data() + (i*src_step), step);
                }
            }
        }
    }
}