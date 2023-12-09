

#include <catch2/catch.hpp>
#include "image/image.hpp"
#include "image/process/conversion.hpp"

TEST_CASE("Row Alignment", "[image align]")
{
    using namespace nitros;

    REQUIRE( utils::image::memory_aligned_step(0, 16) == 0 );
    REQUIRE( utils::image::memory_aligned_step(1, 16) == 2 );
    REQUIRE( utils::image::memory_aligned_step(10, 16) == 2 );
    REQUIRE( utils::image::memory_aligned_step(16, 16) == 2 );
    REQUIRE( utils::image::memory_aligned_step(23, 16) == 4 );
    REQUIRE( utils::image::memory_aligned_step(24, 16) == 4 );
    REQUIRE( utils::image::memory_aligned_step(25, 16) == 4 );
    REQUIRE( utils::image::memory_aligned_step(32, 16) == 4 );
    REQUIRE( utils::image::memory_aligned_step(33, 16) == 6 );
}

TEST_CASE("Image Create", "[create]")
{
    using namespace nitros;

    auto image = utils::image::create_cpu( {100, 200}, utils::pixel::RGBA8::value );
    REQUIRE( image.buffer().size() == 100 * 200 * 4 );
    REQUIRE( image.meta_data().steps.at(0) == 100 * 4 );
    REQUIRE( image.meta_data().steps.size() == 1 );

    auto image2 = utils::image::create_cpu( {100, 200}, utils::pixel::YUV420p::value );
    REQUIRE( image2.meta_data().steps.at(0) == 100 );
    REQUIRE( image2.meta_data().steps.at(1) ==  50 );
    REQUIRE( image2.meta_data().steps.at(2) ==  50 );
    REQUIRE( image2.buffer().size() == 100*200 + 50*100 + 50*100 );

    auto image3 = utils::image::create_cpu( {100, 200}, utils::pixel::YUV422p::value );
    REQUIRE( image3.meta_data().steps.at(0) == 100 );
    REQUIRE( image3.meta_data().steps.at(1) ==  50 );
    REQUIRE( image3.meta_data().steps.at(2) ==  50 );
    REQUIRE( image3.buffer().size() == 100*200 + 50*200 + 50*200 );

    auto image4 = utils::image::create_cpu( {100, 200}, utils::pixel::YUV444p::value );
    REQUIRE( image4.meta_data().steps.at(0) == 100 );
    REQUIRE( image4.meta_data().steps.at(1) == 100 );
    REQUIRE( image4.meta_data().steps.at(2) == 100 );
    REQUIRE( image4.buffer().size() == 100*200 + 100*200 + 100*200 );

    auto image5 = utils::image::create_cpu( {100, 200}, utils::pixel::YUVA420p::value );
    REQUIRE( image5.meta_data().steps.at(0) == 200 );
    REQUIRE( image5.meta_data().steps.at(1) ==  50 );
    REQUIRE( image5.meta_data().steps.at(2) ==  50 );
    REQUIRE( image5.buffer().size() == 200*200 + 50*100 + 50*100 );

    //REQUIRE( image.buffer().size() == 100 )
}

TEST_CASE("Image Flip Y", "[image flip]")
{
    using namespace nitros;

    auto rgba_image = utils::image::create_cpu( {2, 3}, utils::pixel::RGBA8::value );
    auto st = reinterpret_cast<std::array<std::uint8_t, 4 * 2>*>(rgba_image.buffer().data());
    st[0] = {0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11};
    st[1] = {0x11, 0x11, 0x11, 0x11, 0xdd, 0xdd, 0xdd, 0xdd};
    st[2] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    auto rgba_image2 = utils::image::create_cpu( {2, 3}, utils::pixel::RGBA8::value );
    image::flip_y(rgba_image, rgba_image2);

    auto st2 = reinterpret_cast<std::array<std::uint8_t, 4 * 2>*>(rgba_image2.buffer().data());
    
    REQUIRE(st2[0] == st[2]);
    REQUIRE(st2[1] == st[1]);
    REQUIRE(st2[2] == st[0]);
}