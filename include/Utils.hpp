/*
 * Created by Administrator on 2023/8/27.
 */

#ifndef GREEDYSNAKE_UTILS_HPP
#define GREEDYSNAKE_UTILS_HPP

#include <string>
#include <locale>
#include <codecvt>
#include <cmath>
#include <memory>
#include <fstream>
#include "TypeDef.hpp"


namespace Utils {
    /* string=>wstring */
    std::wstring to_wide_string(const std::string &input);

    /* wstring => string */
    std::string to_byte_string(const std::wstring &input);

    /* Any angle => 0 ~ 360 */
    float to_simple_angle(const float &angle);

    /* Get the Angle of point B with respect to point A */
    float get_angle(const int32_t &ax, const int32_t &ay, const int32_t &bx, const int32_t &by);

    /* Get the Angle of point B with respect to point A */
    float get_angle(const Vector2D<int32_t> &A, const Vector2D<int32_t> &B);

    /* Get the distance between two points A and B */
    template<typename T>
    T get_distance(const T &ax, const T &ay, const T &bx, const T &by) {
        return std::sqrt((ax - bx) * (ax - bx) + (ay - by) * (ay - by));
    }

    /* Get the distance between two points A and B */
    template<typename T>
    T get_distance(const Vector2D<T> &A, const Vector2D<T> &B) {
        return std::sqrt((A.x - B.x) * (A.x - B.x) + (A.y - B.y) * (A.y - B.y));
    }

    /* format string */
    template<typename ... Args>
    std::string str_format(const std::string &format, Args ... args) {
        auto buf_size = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1;
        std::unique_ptr<char[]> buf(new(std::nothrow) char[buf_size]);

        if (!buf) {
            return std::string{""};
        }

        std::snprintf(buf.get(), buf_size, format.c_str(), args ...);
        return std::string(buf.get(), buf.get() + buf_size - 1);;
    }

    bool is_bmp(const std::string &filename);

    bool is_jpg(const std::string &filename);

    bool is_bmp(std::basic_ifstream<char> &file);

    bool is_jpg(std::basic_ifstream<char> &file);
}

#endif //GREEDYSNAKE_UTILS_HPP
