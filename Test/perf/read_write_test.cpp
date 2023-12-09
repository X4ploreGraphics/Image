

#include <catch2/catch.hpp>
#include "image/pixel/read_write.hpp"
#include "image/process/conversion.hpp"
#include <chrono>
#include <iostream>

TEST_CASE("Pixel Reader", "[benchmark Pixel Readers]")
{
    using namespace nitros;

    auto rgba_image = utils::image::create_cpu({1920 , 1080 }, utils::pixel::RGBA8::value);

    SECTION("Benchmark 1") {

        using namespace std::chrono_literals;

        auto t1 = std::chrono::steady_clock::now();
        auto reader = utils::pixel::ReaderRGBA<std::uint8_t>{rgba_image};
        auto writer = utils::pixel::WriterRGBA<std::uint8_t>{rgba_image};

        for(auto i = 0; i < 100; i++) {    

            while (auto px = reader.next())
            {
                px->r = 0;
                writer.next(*px);
            }
            
            if( i % 30 == 0 ) {
                std::cout<<i<<std::endl;
            }

            reader.reset();
            writer.reset();
        }
        
        auto t2 = std::chrono::steady_clock::now();
        auto dur = t2 - t1;
        std::cout << std::chrono::duration_cast<std::chrono::microseconds>(dur).count() << std::endl;
        std::cout << std::chrono::duration_cast<std::chrono::microseconds>(dur).count()/100 << std::endl;
    }

    SECTION("Benchmark 2") {

        using namespace std::chrono_literals;
        auto yuv_image = utils::image::create_cpu( rgba_image.meta_data().size, utils::pixel::YUV444p::value );

        auto t1 = std::chrono::steady_clock::now();
        for(auto i = 0; i < 100; i++) {    
            
            image::color_convert(rgba_image, yuv_image);
        }
        
        auto t2 = std::chrono::steady_clock::now();
        auto dur = t2 - t1;
        std::cout << std::chrono::duration_cast<std::chrono::microseconds>(dur).count() << std::endl;
        std::cout << std::chrono::duration_cast<std::chrono::microseconds>(dur).count()/100 << std::endl;
    }
}