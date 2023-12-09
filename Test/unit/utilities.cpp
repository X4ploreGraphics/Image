

#include <catch2/catch.hpp>
#include "image/utils.hpp"

TEST_CASE("Image Utilities", "[get_data]")
{
    using namespace nitros;
    std::uint32_t  n = 0b0011'0010'1111'0000'0000'1111'0011'1100;
    auto rgba_16 = std::array<std::uint16_t, 4>{0x0000, 0x000F, 0xF001, 0XF001};

    static_assert(0b00'0000'1111'0011'11 == 975);

    SECTION("Get Masked")
    {
        auto r_1 = utils::get_data_masked<std::uint16_t>((std::uint8_t*)&rgba_16, 0, 8);
        auto r_2 = utils::get_data_masked<std::uint16_t>((std::uint8_t*)&rgba_16, 16, 16);
        auto r_3 = utils::get_data_masked<std::uint16_t>((std::uint8_t*)&rgba_16, 32, 16);
        REQUIRE(r_1 == 0x0000);
        REQUIRE(r_2 == 0x000F);
        REQUIRE(r_3 == 0xF001);

        auto r_4 = utils::get_data_masked<std::uint16_t>((std::uint8_t*)&rgba_16, 44, 4);
        auto r_5 = utils::get_data_masked<std::uint16_t>((std::uint8_t*)&rgba_16, 45, 8);
        auto r_6 = utils::get_data_masked<std::uint16_t>((std::uint8_t*)&rgba_16, 17, 2);
        auto r_7 = utils::get_data_masked<std::uint16_t>((std::uint8_t*)&rgba_16, 16, 4);
        REQUIRE(r_4 == 0x000F);
        REQUIRE(r_5 == 0x000F);
        REQUIRE(r_6 == 0x0003);
        REQUIRE(r_7 == 0x000F);

        auto rgba_f = std::array<float, 4>{1.f, 2.f, 3.f, 4.f};
        auto r_f = utils::get_data_masked<float>((std::uint8_t*)&rgba_f, 32, 32);
        REQUIRE(r_f == 2.f);
    }

    auto ar_data = std::array<std::uint8_t, 4>{0x11, 0x22, 0x33, 0x44};
    auto ff_data = std::array<float, 4>{1.f, 2.f, 3.f, 4.f};
    SECTION("Write Masked")
    {
        auto st = ar_data.data();

        SECTION("Uint 8 Test 1") {
            utils::write_data_masked(st, 0, 8, std::uint8_t{0xff});
            REQUIRE( st[0] == 0xff );
            REQUIRE( st[1] == 0x22 );
        }
        SECTION("Uint 8 Test 2") {
            utils::write_data_masked(st, 0, 4, std::uint8_t{0xff});
            REQUIRE( st[0] == 0x1f );
            REQUIRE( st[1] == 0x22 );
            utils::write_data_masked(st+1, 4, 4, std::uint8_t{0xff});
            REQUIRE( st[1] == 0xf2 );
            REQUIRE( st[2] == 0x33 );
        }
        SECTION("Uint 8 Test 3") {
            utils::write_data_masked(st, 1, 5, std::uint8_t{0xff});
            REQUIRE( st[0] == 0b0011'1111 );
            REQUIRE( st[1] == 0x22 );
        }
        SECTION("Uint 8 Test 4") {
            utils::write_data_masked(st, 4, 8, std::uint8_t{0xee});
            REQUIRE( st[0] == 0xe1 );
            REQUIRE( st[1] == 0x2e );
        }
        SECTION("Uint 16 Test 1") {
            utils::write_data_masked(st, 4, 16, std::uint16_t{0xffee});
            REQUIRE( st[0] == 0xe1 );
            REQUIRE( st[1] == 0xfe );
            REQUIRE( st[2] == 0x3f );
        }
        SECTION("Uint 16 Test 1") {
            utils::write_data_masked(st, 4, 20, std::uint16_t{0xffee});
            REQUIRE( st[0] == 0xe1 );
            REQUIRE( st[1] == 0xfe );
            REQUIRE( st[2] == 0x0f );
        }
        SECTION("Float 32 Test") {
            utils::write_data_masked((std::uint8_t*)ff_data.data(), 0, 32, float{12.f});
            REQUIRE( ff_data[0] == 12.f );
        }
    }
}

TEST_CASE("Bit Mask", "[bit mask]")
{
    using namespace nitros;

    REQUIRE( utils::create_bit_mask<std::uint16_t>(0, 16) == 0xFFFF );
    REQUIRE( utils::create_bit_mask<std::uint16_t>(0, 0) == 0x0000 );
    REQUIRE( utils::create_bit_mask<std::uint16_t>(0, 8) == 0x00FF );
    REQUIRE( utils::create_bit_mask<std::uint16_t>(4, 8) == 0x0FF0 );
    REQUIRE( utils::create_bit_mask<std::uint16_t>(4, 16) == 0xFFF0 );

    REQUIRE( utils::create_bit_mask<std::uint32_t>(8, 8) == 0x0000'FF00 );
    REQUIRE( utils::create_bit_mask<std::uint8_t>(0, 20) == 0xff );
    REQUIRE( utils::create_bit_mask<std::uint64_t>(8, 8) == 0x0000'0000'0000'FF00 );
}

inline constexpr auto equal(const nitros::utils::PixelOffset  &px, const nitros::utils::PixelOffset  &px2) -> bool {
    return px.px_addr == px2.px_addr && px.bit_offset == px2.bit_offset;
}

TEST_CASE("Point Interpretation", "[interpret]")
{
    using namespace nitros;

    auto rgba_meta = utils::ImageMetaData{ {800, 600}, utils::pixel::RGBA8::value };

    REQUIRE( utils::interpreted_point( {0, 0}, rgba_meta, 0 ) == utils::vec2Ui{0, 0} );
    REQUIRE( utils::interpreted_point( {400, 300}, rgba_meta, 0 ) == utils::vec2Ui{400, 300} );


    auto yuv_meta = utils::ImageMetaData{ {800, 600}, utils::pixel::YUV420p::value };

    REQUIRE( utils::interpreted_point( {400, 300}, yuv_meta, 0 ) == utils::vec2Ui{400, 300} );
    REQUIRE( utils::interpreted_point( {400, 300}, yuv_meta, 1 ) == utils::vec2Ui{200, 150} );
    REQUIRE( utils::interpreted_point( {401, 301}, yuv_meta, 1 ) == utils::vec2Ui{200, 150} );
    REQUIRE( utils::interpreted_point( {402, 302}, yuv_meta, 1 ) == utils::vec2Ui{201, 151} );
    REQUIRE( utils::interpreted_point( {401, 301}, yuv_meta, 2 ) == utils::vec2Ui{200, 150} );

    REQUIRE( utils::interpreted_plane_img_size( yuv_meta, 0 ) == utils::ImgSize{800, 600} );
    REQUIRE( utils::interpreted_plane_img_size( yuv_meta, 1 ) == utils::ImgSize{400, 300} );

    auto yuv_image = utils::image::create_cpu({800, 600}, yuv_meta.format);
    auto rgba_image = utils::image::create_cpu({800, 600}, rgba_meta.format);

    REQUIRE( utils::plane_start_address(yuv_image, 0) == yuv_image.buffer().data() );
    REQUIRE( utils::plane_start_address(yuv_image, 1) == yuv_image.buffer().data() + 800*600 );
    REQUIRE( utils::plane_start_address(yuv_image, 2) == yuv_image.buffer().data() + 800*600 + 400*300 );

    REQUIRE( utils::plane_start_address(rgba_image, 0) == rgba_image.buffer().data() );

    REQUIRE( equal( utils::plane_pixel_address( {400, 300}, yuv_image, 0 ) , { yuv_image.buffer().data() + 800*300 + 400, 0 } ) );
    REQUIRE( equal( utils::plane_pixel_address( {400, 300}, yuv_image, 1 ) , { yuv_image.buffer().data() + 800*600 + 400*150 + 200, 0 } ) );
    REQUIRE( equal( utils::plane_pixel_address( {400, 300}, yuv_image, 2 ) , { yuv_image.buffer().data() + 800*600 + 400*300 + 400*150 + 200, 0 } ) );

    REQUIRE( equal( utils::plane_pixel_address( {400, 300}, rgba_image, 0 ) ,  { rgba_image.buffer().data() + 800*4*300 + 4*400, 0 } ));
    
}