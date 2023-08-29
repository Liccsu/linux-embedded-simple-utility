/*
 * Created by Administrator on 2023/8/27.
 */

#ifndef GREEDYSNAKE_TEXTURE_HPP
#define GREEDYSNAKE_TEXTURE_HPP


#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include <jpeglib.h>
#include <jerror.h>
#include <memory>

class Texture {
private:
    int32_t width; /* 图片宽度 */
    int32_t height; /* 图片高度 */
    int32_t line_width; /* 数组宽度 */
    int16_t depth; /* 图片色深 */
    std::unique_ptr <uint32_t> texture; /* 图片颜色数组 */
public:
    Texture() = delete;

    explicit Texture(const std::string &path);

    Texture(const Texture &other);

    const std::unique_ptr <uint32_t> &get_texture() const;

    const int32_t &get_width() const;

    const int32_t &get_height() const;

    const int16_t &get_depth() const;

    const int32_t &get_line_width() const;
};


#endif //GREEDYSNAKE_TEXTURE_HPP
