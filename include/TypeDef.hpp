/*
 * Created by Administrator on 2023/8/26.
 */

#ifndef GREEDYSNAKE_TYPEDEF_HPP
#define GREEDYSNAKE_TYPEDEF_HPP

#include <string>

template<typename T>
struct Vector2D {
    T x;
    T y;

    Vector2D() : x(0), y(0) {}

    Vector2D(T _x, T _y) : x(_x), y(_y) {}

    Vector2D(const Vector2D &v2d) : x(v2d.x), y(v2d.y) {}

    Vector2D &operator=(const Vector2D &v2d) {
        this->x = v2d.x;
        this->y = v2d.y;

        return *this;
    }

    Vector2D &operator+=(const Vector2D &v2d) {
        this->x += v2d.x;
        this->y += v2d.y;

        return *this;
    }

    Vector2D &operator-=(const Vector2D &v2d) {
        this->x -= v2d.x;
        this->y -= v2d.y;

        return *this;
    }

    Vector2D &operator*=(const T &num) {
        this->x *= num;
        this->y *= num;

        return *this;
    }

    Vector2D &operator/=(const T &num) {
        this->x /= num;
        this->y /= num;

        return *this;
    }

    friend Vector2D operator+(const Vector2D &v2d1, const Vector2D &v2d2) {
        Vector2D result{v2d1.x + v2d2.x, v2d1.y + v2d2.y};

        return result;
    }

    friend Vector2D operator-(const Vector2D &v2d1, const Vector2D &v2d2) {
        Vector2D result{v2d1.x - v2d2.x, v2d1.y - v2d2.y};

        return result;
    }

    friend Vector2D operator*(const Vector2D &v2d1, const T &num) {
        Vector2D result{v2d1.x * num, v2d1.y * num};

        return result;
    }

    friend Vector2D operator*(const T &num, const Vector2D &v2d1) {
        Vector2D result{v2d1.x * num, v2d1.y * num};

        return result;
    }

    friend Vector2D operator/(const Vector2D &v2d1, const T &num) {
        Vector2D result{v2d1.x / num, v2d1.y / num};

        return result;
    }

    friend Vector2D operator/(const T &num, const Vector2D &v2d1) {
        Vector2D result{v2d1.x / num, v2d1.y / num};

        return result;
    }
};

struct TextView {
    int32_t x, y;
    int32_t size;
    uint32_t color;
    std::string text;
    std::string ttf;

    TextView() : x(0), y(0), size(0), color(0), text(), ttf() {}

    TextView(const TextView &textView) {
        this->x = textView.x;
        this->y = textView.y;
        this->size = textView.size;
        this->color = textView.color;
        this->text = textView.text;
        this->ttf = textView.ttf;
    }

    TextView(const std::string &_text, int32_t _x, int32_t _y, int32_t _size, uint32_t _color = 0) {
        this->x = _x;
        this->y = _y;
        this->size = _size;
        this->color = _color;
        this->text = _text;
        this->ttf = std::string{""};
    }

    TextView(const std::string &_ttf, const std::string &_text, int32_t _x, int32_t _y, int32_t _size,
             uint32_t _color = 0) {
        this->x = _x;
        this->y = _y;
        this->size = _size;
        this->color = _color;
        this->text = _text;
        this->ttf = _ttf;
    }

    TextView &operator=(const TextView &textView) {
        this->x = textView.x;
        this->y = textView.y;
        this->size = textView.size;
        this->color = textView.color;
        this->text = textView.text;
        this->ttf = textView.ttf;
    }
};

#endif //GREEDYSNAKE_TYPEDEF_HPP
