

#include <catch2/catch.hpp>
#include "image/image.hpp"
#include "image/pixel/read_write.hpp"
#include <iostream>

TEST_CASE("Pixel Reader", "[read] [pixel]")
{
    using namespace nitros;

    auto rgba_image = utils::image::create_cpu( {2, 2}, utils::pixel::RGBA8::value );
    auto st = reinterpret_cast<std::array<std::uint8_t, 4>*>(rgba_image.buffer().data());

    st[0] = {0xF2, 0x00, 0x11, 0xFF};
    st[1] = {0xE2, 0x03, 0x1E, 0xF1};
    st[2] = {0xE2, 0x13, 0xAE, 0xF1};

    SECTION("Uint 8 Reader") 
    {
        auto reader = utils::pixel::ReaderRGBA<std::uint8_t>{rgba_image};
        {
            auto px = reader.at({0, 0});
            REQUIRE( px.r == 0xF2 );
            REQUIRE( px.g == 0x00 );
            REQUIRE( px.b == 0x11 );
            REQUIRE( px.a == 0xFF );
        }
        {
            auto px = reader.at({1, 0});
            REQUIRE( px.r == 0xE2 );
            REQUIRE( px.g == 0x03 );
            REQUIRE( px.b == 0x1E );
            REQUIRE( px.a == 0xF1 );
        }
        {
            auto px = reader.at({0, 1});
            REQUIRE( px.r == 0xE2 );
            REQUIRE( px.g == 0x13 );
            REQUIRE( px.b == 0xAE );
            REQUIRE( px.a == 0xF1 );
        }

        for (auto i = 0; auto px = reader.next(); i++)
        {
            auto ptr = rgba_image.buffer().data() + i*4;
            //reinterpret_cast<std::uint8_t*>( st + i );
            REQUIRE( px->r ==  ptr[0]);
            REQUIRE( px->g ==  ptr[1]);
            REQUIRE( px->b ==  ptr[2]);
            REQUIRE( px->a ==  ptr[3]);
        }
    }
    SECTION("Uint 8 Writer")
    {
        auto writer = utils::pixel::WriterRGBA<std::uint8_t>{rgba_image};
        writer.at({
            .r = 0x01,
            .g = 0x02,
            .b = 0x11,
            .a = 0x22
        }, {0, 0});
        auto st = rgba_image.buffer().data();

        REQUIRE( st[0] == 0x01 );
        REQUIRE( st[1] == 0x02 );
        REQUIRE( st[2] == 0x11 );
        REQUIRE( st[3] == 0x22 );

        auto p_rgba = utils::pixel::PixelRGBA<std::uint8_t>{.r = 0, .g = 1, .b = 2, .a = 3};
        for(auto i = 0; writer.next(p_rgba); i++)
        {
            REQUIRE( st[i*4 + 0] == i*4 + 0 );
            REQUIRE( st[i*4 + 1] == i*4 + 1 );
            REQUIRE( st[i*4 + 2] == i*4 + 2 );
            REQUIRE( st[i*4 + 3] == i*4 + 3 );
            
            p_rgba.r = (i+1)*4 + 0;
            p_rgba.g = (i+1)*4 + 1;
            p_rgba.b = (i+1)*4 + 2;
            p_rgba.a = (i+1)*4 + 3;
        }
    }

    SECTION("Uint 16 Reader")
    {
        auto reader_2 = utils::pixel::ReaderRGBA<std::uint16_t>{rgba_image};
        for (auto i = 0; auto px = reader_2.next(); i++)
        {
            auto ptr = rgba_image.buffer().data() + i*4;
            //reinterpret_cast<std::uint8_t*>( st + i );
            REQUIRE( px->r ==  ptr[0]);
            REQUIRE( px->g ==  ptr[1]);
            REQUIRE( px->b ==  ptr[2]);
            REQUIRE( px->a ==  ptr[3]);
        }
    }

    SECTION("RGB8 Uint 8 Reader")
    {
        auto rgb_image = utils::image::create_cpu( {2, 2}, utils::pixel::RGB8::value );
        auto st = reinterpret_cast<std::array<std::uint8_t, 3>*>(rgb_image.buffer().data());
        auto st8 = rgb_image.buffer().data();
        st[0] = {0xF2, 0x00, 0x11};
        st[1] = {0xE2, 0x03, 0x1E};
        st[2] = {0xE2, 0x13, 0xAE};

        auto bgr_image = utils::image::create_cpu( {2, 2}, utils::pixel::BGR8::value );
        std::memcpy(bgr_image.buffer().data(), rgb_image.buffer().data(), rgb_image.buffer().size());

        auto reader = utils::pixel::ReaderRGBA<std::uint8_t>(rgb_image);
        for (auto i = 0; auto px = reader.next(); i++)
        {
            auto ptr = st8 + i*3;
            REQUIRE( px->r == ptr[0] );
            REQUIRE( px->g == ptr[1] );
            REQUIRE( px->b == ptr[2] );
            REQUIRE( px->a == 0xff );
        }

        auto reader2 = utils::pixel::ReaderRGBA<std::uint8_t>(bgr_image);
        for (auto i = 0; auto px = reader2.next(); i++)
        {
            auto ptr = st8 + i*3;
            REQUIRE( px->r == ptr[2] );
            REQUIRE( px->g == ptr[1] );
            REQUIRE( px->b == ptr[0] );
            REQUIRE( px->a == 0xff );
        }
    }

    SECTION("YUV Uint8 Reader")
    {
        auto yuv_image = utils::image::create_cpu( {4, 4}, utils::pixel::YUV420p::value );
        auto st = reinterpret_cast<std::array<std::uint8_t, 16>*>( yuv_image.buffer().data() );
        st[0] = {
            0xff, 0xee, 0xdd, 0xcc, 
            0xf1, 0xe1, 0xd1, 0xc1, 
            0xf2, 0xe2, 0xd2, 0xc2, 
            0xf3, 0xe3, 0xd3, 0xc3, 
        };

        auto st_1 = reinterpret_cast<std::array<std::uint8_t, 4>*>( yuv_image.buffer().data() + 16 );
        st_1[0] = {
            0x1f, 0x1e,
            0x0f, 0x0e
        };

        st_1[1] = {
            0x2f, 0x2e,
            0x2f, 0x2e
        };
        
        auto reader = utils::pixel::ReaderYUV<std::uint8_t>(yuv_image);
        auto px = reader.at({0, 0});
        REQUIRE( px.y  == 0xff );
        REQUIRE( px.cb == 0x1f );
        REQUIRE( px.cr == 0x2f );

        for(auto i = 0; auto px = reader.next(); i++)
        {
            auto x = (i%4)/2;
            auto y = (i/4)/2;

            REQUIRE( px->y  == st[0][i] );
            REQUIRE( px->cb == st_1[0][ y*2 + x ] );
            REQUIRE( px->cr == st_1[1][ y*2 + x ] );
        }
        //reader.at(0, 0)
    }

    SECTION("YUVA Uint8 Reader")
    {
        auto yuva_image = utils::image::create_cpu( {4, 4}, utils::pixel::YUVA420p::value );
        auto st = reinterpret_cast<std::array<std::uint8_t, 16*2>*>( yuva_image.buffer().data() );
        st[0] = {
            0xf1, 0xff, 0xee, 0xff, 0xdd, 0xff, 0xcc, 0xff, 
            0xf1, 0xff, 0xe1, 0xff, 0xd1, 0xff, 0xc1, 0xff, 
            0xf2, 0xff, 0xe2, 0xff, 0xd2, 0xff, 0xc2, 0xff, 
            0xf3, 0xff, 0xe3, 0xff, 0xd3, 0xff, 0xc3, 0xff, 
        };

        auto st_1 = reinterpret_cast<std::array<std::uint8_t, 4>*>( yuva_image.buffer().data() + 16*2 );
        st_1[0] = {
            0x1f, 0x1e,
            0x0f, 0x0e
        };

        st_1[1] = {
            0x2f, 0x2e,
            0x2f, 0x2e
        };
        
        auto reader = utils::pixel::ReaderYUV<std::uint8_t>(yuva_image);
        auto px = reader.at({0, 0});
        REQUIRE( px.y  == 0xf1 );
        REQUIRE( px.cb == 0x1f );
        REQUIRE( px.cr == 0x2f );
        REQUIRE( px.a  == 0xff );

        auto px2 = reader.at({1, 0});
        REQUIRE( px2.y  == 0xee );
        REQUIRE( px2.cb == 0x1f );
        REQUIRE( px2.cr == 0x2f );
        REQUIRE( px2.a  == 0xff );

        for(auto i = 0; auto px = reader.next(); i++)
        {
            auto x = (i%4)/2;
            auto y = (i/4)/2;

            REQUIRE( px->y  == st[0][ i*2 ] );
            REQUIRE( px->a  == st[0][ i*2 + 1] );
            REQUIRE( px->cb == st_1[0][ y*2 + x ] );
            REQUIRE( px->cr == st_1[1][ y*2 + x ] );
        }

        auto writer = utils::pixel::WriterYUV<std::uint8_t>(yuva_image);
        writer.at({
            .y  = 0x33,
            .cb = 0x22,
            .cr = 0x11,
            .a  = 0x01 
        }, {0, 0});

        REQUIRE(st[0][0] == 0x33);
        REQUIRE(st[0][1] == 0x01);
        REQUIRE(st_1[0][0] == 0x22);
        REQUIRE(st_1[1][0] == 0x11);

        writer.at({
            .y  = 0x44,
            .cb = 0x22,
            .cr = 0x11,
            .a  = 0x02 
        }, {1, 0});

        REQUIRE(st[0][2] == 0x44);
        REQUIRE(st[0][3] == 0x02);
        REQUIRE(st_1[0][0] == 0x22);
        REQUIRE(st_1[1][0] == 0x11);
    }
}