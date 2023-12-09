

#include "image/fileio.hpp"
#include "image/pixel/read_write.hpp"
#include "image/process/conversion.hpp"
#include"utilities/files/files.hpp"
#include <iostream>

inline void help() {
    std::cout<<"Usage: Exec <image>"<<std::endl;
}

int main(int argv, char *argc[])
{
    if(argv < 2) {
        help();
        return 0;
    }

    std::string source_file{argc[1]};

    std::string target_file_png = "result_image.png";
    std::string target_file_jpg = "result_image.jpg";
    std::string target_file_bmp = "result_image.bmp";

    using namespace nitros;

    auto encoded_data = nitros::utils::reader::read_binary(source_file);

    //auto image = read_image(source_file);
    auto image = utils::image::decode_image(encoded_data);
    auto yuv_image = utils::image::create_cpu( image.meta_data().size, utils::pixel::YUV420p::value );
    auto rgb_image = utils::image::create_cpu( image.meta_data().size, utils::pixel::RGB8::value );
    image::color_convert(image, yuv_image);
    image::color_convert(yuv_image, rgb_image);

    utils::image::write_image_png(target_file_png, image);
    utils::image::write_image_jpg(target_file_jpg, image);
    utils::image::write_image_bmp(target_file_bmp, image);

    return 0;
}
