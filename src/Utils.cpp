/*
 * Created by Administrator on 2023/8/27.
 */

#include "Utils.hpp"

#include <cmath>


/* convert string to wstring */
std::wstring Utils::to_wide_string(const std::string &input) {
    std::wstring_convert <std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(input);
}

/* convert wstring to string */
std::string Utils::to_byte_string(const std::wstring &input) {
    // std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring_convert <std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(input);
}

float Utils::to_simple_angle(const float &angle) {
    float _angle = angle - std::floor(angle / 360.0f) * 360.0f;
    if (_angle < 0) {
        _angle += 360.0f;
    }

    return _angle;
}

float Utils::get_angle(const int32_t &ax, const int32_t &ay, const int32_t &bx, const int32_t &by) {
    auto angle = static_cast<float>(std::atan2(by - ay, bx - ax) * 180.0f / M_PI);

    return to_simple_angle(angle);
}

float Utils::get_angle(const Vector2D <int32_t> &A, const Vector2D <int32_t> &B) {
    auto angle = static_cast<float>(std::atan2(B.y - A.y, B.x - A.x) * 180.0f / M_PI);

    return to_simple_angle(angle);
}

bool Utils::is_bmp(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);

    uint8_t b, m;
    file.read((char *) &b, 1);
    file.read((char *) &m, 1);
    file.close();

    return (b == 0x42) && (m == 0x4D);
}

bool Utils::is_jpg(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);

    uint8_t ff, d8, ff2, e0_ef;
    file.read((char *) &ff, 1);
    file.read((char *) &d8, 1);
    file.read((char *) &ff2, 1);
    file.read((char *) &e0_ef, 1);
    file.close();

    return (ff == 0xFF) && (d8 == 0xD8) &&
           (ff2 == 0xFF) && (e0_ef >= 0xE0 && e0_ef <= 0xEF);
}

bool Utils::is_bmp(std::basic_ifstream<char> &file) {
    uint8_t b, m;
    file.read((char *) &b, 1);
    file.read((char *) &m, 1);

    return (b == 0x42) && (m == 0x4D);
}

bool Utils::is_jpg(std::basic_ifstream<char> &file) {
    uint8_t ff, d8, ff2, e0_ef;
    file.read((char *) &ff, 1);
    file.read((char *) &d8, 1);
    file.read((char *) &ff2, 1);
    file.read((char *) &e0_ef, 1);

    return (ff == 0xFF) && (d8 == 0xD8) &&
           (ff2 == 0xFF) && (e0_ef >= 0xE0 && e0_ef <= 0xEF);
}
