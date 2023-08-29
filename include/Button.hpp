/*
 * Created by Administrator on 2023/8/26.
 */

#ifndef GREEDYSNAKE_BUTTON_HPP
#define GREEDYSNAKE_BUTTON_HPP

#include <functional>
#include <utility>
#include <tslib.h>
#include "TypeDef.hpp"
#include "BaseEvent.hpp"
#include "Utils.hpp"


class Button {
private:
    const int32_t x1_, y1_, x2_, y2_;
    const uint32_t color_;
    const int32_t rad_;
    const TextView textView_;
    std::function<void()> click_callback;
    std::function<void()> press_callback;
    std::function<void()> release_callback;

public:
    int32_t x1, y1, x2, y2;
    uint32_t color;
    int32_t rad;
    TextView textView;

    Button() = delete;

    Button(int32_t _x1, int32_t _y1, int32_t _x2, int32_t _y2, uint32_t _color, int32_t _rad = 0,
           const TextView &_textView = {});

    void setOnClickListener(std::function<void()> _callback);

    void setOnPressListener(std::function<void()> _callback);

    void setOnReleaseListener(std::function<void()> _callback);

    void onClick();

    void onPress();

    void onRelease();

    const int32_t &get_x1() const;

    const int32_t &get_y1() const;

    const int32_t &get_x2() const;

    const int32_t &get_y2() const;

    const int32_t &get_rad() const;

    const uint32_t &get_color() const;

    const TextView &get_textView() const;
};


#endif //GREEDYSNAKE_BUTTON_HPP
