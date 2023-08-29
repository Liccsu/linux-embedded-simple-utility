/*
 * Created by Administrator on 2023/8/27.
 */

#include <fcntl.h>
#include <unistd.h>
#include "Texture.hpp"
#include "LOG.hpp"
#include "Utils.hpp"

Texture::Texture(const std::string &path) : width(0), height(0), depth(0), texture(nullptr), line_width(0) {
//    atexit(destroy);
    // 打开文件
    std::ifstream file(path, std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error(
                Utils::str_format("%s:%d:%s: %s is open failed.\n",
                                  __FILE__, __LINE__, __FUNCTION__, path.c_str()));
    }
    try {
        if (Utils::is_bmp(file)) {
            // 读取宽高
            file.seekg(0x12);
            file.read(reinterpret_cast<char *>(&this->width), 4);
            LOG(LOG_INFO, "width = %d\n", this->width);

            file.seekg(0x16);
            file.read(reinterpret_cast<char *>(&this->height), 4);
            LOG(LOG_INFO, "height = %d\n", this->height);

            // 读取颜色深度
            file.seekg(0x1C);
            file.read(reinterpret_cast<char *>(&this->depth), 2);
            LOG(LOG_INFO, "depth = %d\n", this->depth);

            // 计算行大小和填充字节
            int32_t filledBytes = 4 - (abs(this->width) * (this->depth / 8)) % 4;
            if (filledBytes == 4) {
                filledBytes = 0;
            }
            this->line_width = (abs(this->width) * (this->depth / 8)) + filledBytes;

            // 读取像素数据到向量
            long size = abs(this->height) * this->line_width;
            std::unique_ptr <uint8_t> pixelData(new(std::nothrow) uint8_t[size]);
            file.seekg(0x36);
            file.read(reinterpret_cast<char *>(pixelData.get()), size);

            // 遍历像素写入纹理
            uint8_t b, g, r, a;
            uint32_t color, index = size - 1;
            int32_t x, y;
            this->texture.reset(new(std::nothrow) uint32_t[this->width * this->height]);
            LOG(LOG_INFO, "texture read ready.\n");
            for (int32_t i = 0; i < abs(this->height); i++) {
                index -= filledBytes;
                for (int32_t j = 0; j < abs(this->width); j++) {
                    // 读取alpha通道
                    if (this->depth == 32) {
                        a = pixelData.get()[index--];
                    }
                    // 读取RGB通道
                    r = pixelData.get()[index--];
                    g = pixelData.get()[index--];
                    b = pixelData.get()[index--];
                    // 生成颜色值写入纹理
                    color = (a << 24) | (r << 16) | (g << 8) | b;
                    x = (this->width < 0) ? j : (abs(this->width) - j - 1);
                    y = (this->height < 0) ? (abs(this->height) - i - 1) : i;
                    this->texture.get()[x + y * this->width] = color;
                }
            }
            LOG(LOG_INFO, "texture read success.\n");
        } else if (Utils::is_jpg(file)) {
            // 1）分配并初始化一个jpeg 解压对象
            struct jpeg_decompress_struct jpeg_decompress{};    //声明一个解压对象的
            struct jpeg_error_mgr jpeg_error{};                //声明一个出错信息对象的

            FILE *infile = fopen(path.c_str(), "r");
            if (infile == nullptr) {
                // 关闭文件
                file.close();
                throw std::runtime_error(
                        Utils::str_format("%s:%d:%s: %s is open failed.\n",
                                          __FILE__, __LINE__, __FUNCTION__, path.c_str()));
            }
            try {
                jpeg_decompress.err = jpeg_std_error(&jpeg_error);    //初始化这个出错对象
                jpeg_create_decompress(&jpeg_decompress);        //初始化jpeg_decompress解压对象

                jpeg_stdio_src(&jpeg_decompress, infile); //指定要解压的jpg图片

                //3）调用 jpeg_read_header() 获取图片的信息
                jpeg_read_header(&jpeg_decompress, TRUE);

                //4）用于设置jpeg解压对象的一些参数，一般采用默认参数

                //5）调用 jpeg_start_decompress() 启动解压过程
                jpeg_start_decompress(&jpeg_decompress);

                this->width = static_cast<int32_t>(jpeg_decompress.output_width);
                this->height = static_cast<int32_t>(jpeg_decompress.output_height);
                this->texture.reset(new(std::nothrow) uint32_t[this->width * this->height]);

                //6）读取一行或多行扫描线上数据 并 处理，显示
                std::unique_ptr <uint8_t> buffer(
                        new uint8_t[jpeg_decompress.output_width * jpeg_decompress.output_components]);

                //jpeg_decompress.output_scanline 表示已经扫描的行数 0~ height-1
                uint32_t x;    //横坐标
                uint32_t y;    //纵坐标

                uint8_t *p, *buf = buffer.get();
                uint8_t a, r, g, b;

                while (jpeg_decompress.output_scanline < jpeg_decompress.output_height) {
                    y = jpeg_decompress.output_scanline;

                    jpeg_read_scanlines(&jpeg_decompress,    //解压对象
                                        &buf,    //保存解压数据的二级指针
                                        1);        //每次读取一行来解压

                    p = buf;

                    for (x = 0; x < jpeg_decompress.output_width; x++) {
                        if (jpeg_decompress.output_components == 3) {
                            a = 0;
                        } else {
                            a = *p++;
                        }
                        r = *p++;
                        g = *p++;
                        b = *p++;

                        uint32_t color = a << 24 | r << 16 | g << 8 | b;

                        this->texture.get()[x + y * this->width] = color;
                    }
                }
                //7）调用 jpeg_finish_decompress() 完成解压过程
                jpeg_finish_decompress(&jpeg_decompress);

                //8）调用 jpeg_destroy_decompress() 释放jpeg解压对象
                jpeg_destroy_decompress(&jpeg_decompress);
                fclose(infile);
            } catch (std::exception &e) {
                fclose(infile);
            }
        } else {
            throw std::runtime_error(
                    Utils::str_format("%s:%d:%s: %s is unrecognized format.\n",
                                      __FILE__, __LINE__, __FUNCTION__, path.c_str()));
        }
        // 关闭文件
        file.close();
    } catch (const std::exception &e) {
        // 关闭文件
        file.close();
    }
}

Texture::Texture(const Texture &other) : width(other.width), height(other.height), depth(other.depth), line_width(0) {
    this->texture.reset(new(std::nothrow) uint32_t[this->width * this->height]);
    std::copy(this->texture.get(), this->texture.get() + this->width * this->height, other.texture.get());

}

const std::unique_ptr <uint32_t> &Texture::get_texture() const {
    return this->texture;
}

const int32_t &Texture::get_width() const {
    return this->width;
}

const int32_t &Texture::get_height() const {
    return this->height;
}

const int16_t &Texture::get_depth() const {
    return this->depth;
}

const int32_t &Texture::get_line_width() const {
    return this->line_width;
}
